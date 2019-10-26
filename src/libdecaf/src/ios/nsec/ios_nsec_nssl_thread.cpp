#include "ios_nsec_nssl_device.h"
#include "ios_nsec_nssl_thread.h"
#include "ios_nsec_nssl_request.h"
#include "ios_nsec_nssl_response.h"

#include "ios/kernel/ios_kernel_messagequeue.h"
#include "ios/kernel/ios_kernel_process.h"
#include "ios/kernel/ios_kernel_resourcemanager.h"
#include "ios/kernel/ios_kernel_thread.h"
#include "ios/kernel/ios_kernel_ipc.h"
#include "ios/net/ios_net_soshim.h"

#include "ios/ios_stackobject.h"

#include <array>

namespace ios::nsec::internal
{

using SocketDeviceHandle = int32_t;
using namespace kernel;
using namespace net;

constexpr auto NumNsslMessages = 32u;
constexpr auto NsslThreadStackSize = 0x4000u;
constexpr auto NsslThreadPriority = 47u;

struct StaticNsslThreadData
{
   be2_val<BOOL> suspended;
   be2_val<ThreadId> threadId;
   be2_val<MessageQueueId> messageQueueId;
   be2_struct<IpcRequest> stopMessage;
   be2_array<Message, NumNsslMessages> messageBuffer;
   be2_array<uint8_t, NsslThreadStackSize> threadStack;

   be2_val<BOOL> openedSocketHandle;
   be2_val<Handle> socketHandle;
};

static phys_ptr<StaticNsslThreadData>
sNsslThreadData = nullptr;

static std::array<std::unique_ptr<NSSLDevice>, ProcessId::Max>
sDevices;

static NSSLDevice *
getDevice(SocketDeviceHandle handle)
{
   if (handle < 0 || handle >= sDevices.size()) {
      return nullptr;
   }

   return sDevices[handle].get();
}

static Error
nsslOpen(phys_ptr<ResourceRequest> request)
{
   // Send GetProcessSocketHandle to /dev/socket
   if (!sNsslThreadData->openedSocketHandle) {
      auto error = SOShim_Open();
      if (error < 0) {
         return static_cast<Error>(NSSLError::IoError);
      }
      sNsslThreadData->socketHandle = static_cast<Handle>(error);
      sNsslThreadData->openedSocketHandle = TRUE;
   }

   auto error = SOShim_GetProcessSocketHandle(sNsslThreadData->socketHandle,
                                              request->requestData.titleId,
                                              request->requestData.processId);
   if (error < 0) {
      return static_cast<Error>(NSSLError::IoError);
   }

   // There is one NSSL device per process
   auto pid = request->requestData.clientPid;
   auto idx = static_cast<size_t>(pid);

   if (!sDevices[idx]) {
      sDevices[idx] = std::make_unique<NSSLDevice>(static_cast<Handle>(error));
   }

   return static_cast<Error>(idx);
}

static Error
nsslClose(phys_ptr<ResourceRequest> request)
{
   auto pid = request->requestData.clientPid;
   auto idx = static_cast<size_t>(pid);

   if (idx != request->requestData.handle) {
      return Error::Exists;
   }

   sDevices[idx] = nullptr;
   return Error::OK;
}

static Error
nsslIoctl(phys_ptr<ResourceRequest> resourceRequest)
{
   auto error = Error::OK;
   auto device = getDevice(resourceRequest->requestData.handle);
   if (!device) {
      return static_cast<Error>(NSSLError::InvalidHandle);
   }

   switch (static_cast<NSSLCommand>(resourceRequest->requestData.args.ioctl.request)) {
   case NSSLCommand::CreateContext:
   {
      if (resourceRequest->requestData.args.ioctl.inputLength != sizeof(NSSLCreateContextRequest)) {
         return Error::InvalidArg;
      }

      if (!resourceRequest->requestData.args.ioctl.inputBuffer) {
         return Error::InvalidArg;
      }

      auto request = phys_cast<NSSLCreateContextRequest *>(resourceRequest->requestData.args.ioctl.inputBuffer);
      error = static_cast<Error>(device->createContext(request->version));
      break;
   }
   default:
      error = Error::InvalidArg;
   }

   return error;
}

static Error
nsslIoctlv(phys_ptr<ResourceRequest> request)
{
   auto device = getDevice(request->requestData.handle);
   if (!device) {
      return Error::InvalidHandle;
   }

   /* TODO: Handle commands
   switch (static_cast<NSSLCommand>(request->requestData.command)) {
   default:
   }
   */

   return Error::InvalidArg;
}

static Error
nsslThreadEntry(phys_ptr<void> /*context*/)
{
   StackObject<Message> message;

   while (true) {
      auto error = IOS_ReceiveMessage(sNsslThreadData->messageQueueId,
                                      message,
                                      MessageFlags::None);
      if (error < Error::OK) {
         break;
      }

      auto request = parseMessage<ResourceRequest>(message);
      switch (request->requestData.command) {
      case Command::Open:
         IOS_ResourceReply(request, nsslOpen(request));
         break;
      case Command::Close:
         IOS_ResourceReply(request, nsslClose(request));
         break;
      case Command::Ioctl:
         IOS_ResourceReply(request, nsslIoctl(request));
         break;
      case Command::Ioctlv:
         IOS_ResourceReply(request, nsslIoctlv(request));
         break;
      case Command::Suspend:
         // TODO: /dev/nsec/nssl Suspend
         return Error::OK;
      case Command::Resume:
         // TODO: /dev/nsec/nssl Resume
         return Error::OK;
      default:
         IOS_ResourceReply(request, Error::InvalidArg);
      }
   }

   if (sNsslThreadData->messageQueueId > 0) {
      IOS_DestroyMessageQueue(sNsslThreadData->messageQueueId);
      sNsslThreadData->messageQueueId = Error::Invalid;
   }

   IOS_SuspendThread(IOS_GetCurrentThreadId());
   return Error::OK;
}

Error
registerNsslResourceManager()
{
   auto error = IOS_CreateMessageQueue(phys_addrof(sNsslThreadData->messageBuffer),
                                       sNsslThreadData->messageBuffer.size());
   if (error < Error::OK) {
      return error;
   }
   sNsslThreadData->messageQueueId = static_cast<MessageQueueId>(error);

   error = IOS_RegisterResourceManager("/dev/nsec/nssl", sNsslThreadData->messageQueueId);
   if (error < Error::OK) {
      IOS_DestroyMessageQueue(sNsslThreadData->messageQueueId);
      sNsslThreadData->messageQueueId = Error::Invalid;
      return error;
   }

   error = IOS_AssociateResourceManager("/dev/nsec/nssl", ResourcePermissionGroup::NSEC);
   if (error < Error::OK) {
      IOS_DestroyMessageQueue(sNsslThreadData->messageQueueId);
      sNsslThreadData->messageQueueId = Error::Invalid;
      return error;
   }

   return Error::OK;
}

Error
startNsslThread()
{
   auto error = IOS_CreateThread(&nsslThreadEntry,
                                 nullptr,
                                 phys_addrof(sNsslThreadData->threadStack) + sNsslThreadData->threadStack.size(),
                                 sNsslThreadData->threadStack.size(),
                                 NsslThreadPriority,
                                 ThreadFlags::Detached);
   if (error < Error::OK) {
      return error;
   }

   sNsslThreadData->threadId = static_cast<ThreadId>(error);
   kernel::internal::setThreadName(sNsslThreadData->threadId, "NsslThread");

   return IOS_StartThread(sNsslThreadData->threadId);
}

Error
stopNsslThread()
{
   return IOS_JamMessage(sNsslThreadData->messageQueueId,
                         makeMessage(phys_addrof(sNsslThreadData->stopMessage)),
                         MessageFlags::NonBlocking);
}

void
initialiseStaticNsslData()
{
   sNsslThreadData = allocProcessStatic<StaticNsslThreadData>();
   sNsslThreadData->stopMessage.command = Command::Suspend;
}

} // namespace ios::nsec::internal
