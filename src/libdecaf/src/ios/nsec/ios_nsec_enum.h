#ifndef IOS_NSEC_ENUM_H
#define IOS_NSEC_ENUM_H

#include <common/enum_start.inl>

ENUM_NAMESPACE_ENTER(ios)

ENUM_NAMESPACE_ENTER(nsec)

ENUM_BEG(NSSLCommand, uint32_t)
   ENUM_VALUE(CreateContext,     0x1)
ENUM_END(NSSLCommand)

ENUM_BEG(NSSLError, int32_t)
   ENUM_VALUE(OK,                         0)
   ENUM_VALUE(Generic,                    -1)
   ENUM_VALUE(InvalidNsslContext,         -2621441)
   ENUM_VALUE(InvalidCertID,              -2621442)
   ENUM_VALUE(CertLimit,                  -2621443)
   ENUM_VALUE(InvalidNsslConnection,      -2621444)
   ENUM_VALUE(InvalidCert,                -2621445)
   ENUM_VALUE(ZeroReturn,                 -2621446)
   ENUM_VALUE(WantRead,                   -2621447)
   ENUM_VALUE(WantWrite,                  -2621448)
   ENUM_VALUE(IoError,                    -2621449)
   ENUM_VALUE(NsslLibError,               -2621450)
   ENUM_VALUE(Unknown,                    -2621451)
   ENUM_VALUE(OutOfMemory,                -2621452)
   ENUM_VALUE(InvalidState,               -2621453)
   ENUM_VALUE(HandshakeError,             -2621454)
   ENUM_VALUE(NoCert,                     -2621455)
   ENUM_VALUE(InvalidFd,                  -2621456)
   ENUM_VALUE(LibNotReady,                -2621457)
   ENUM_VALUE(IpcError,                   -2621458)
   ENUM_VALUE(ResourceLimit,              -2621459)
   ENUM_VALUE(InvalidHandle,              -2621460)
   ENUM_VALUE(InvalidCertType,            -2621461)
   ENUM_VALUE(InvalidKeyType,             -2621462)
   ENUM_VALUE(InvalidSize,                -2621463)
   ENUM_VALUE(NoPeerCert,                 -2621464)
   ENUM_VALUE(InsufficientSize,           -2621465)
   ENUM_VALUE(NoCipher,                   -2621466)
   ENUM_VALUE(InvalidArg,                 -2621467)
   ENUM_VALUE(InvalidNsslSession,         -2621468)
   ENUM_VALUE(NoSession,                  -2621469)
   ENUM_VALUE(SslShutdownError,           -2621470)
   ENUM_VALUE(CertSizeLimit,              -2621471)
   ENUM_VALUE(CertNoAccess,               -2621472)
   ENUM_VALUE(InvalidCertId2,             -2621473)
   ENUM_VALUE(CertReadError,              -2621474)
   ENUM_VALUE(CertStoreInitFailure,       -2621475)
   ENUM_VALUE(InvalidCertEncoding,        -2621476)
   ENUM_VALUE(CertStoreError,             -2621477)
   ENUM_VALUE(PrivateKeyReadError,        -2621478)
   ENUM_VALUE(InvalidPrivateKey,          -2621479)
   ENUM_VALUE(NotReady,                   -2621480)
   ENUM_VALUE(EncryptionError,            -2621481)
   ENUM_VALUE(NoCertStore,                -2621482)
   ENUM_VALUE(PrivateKeySizeLimit,        -2621483)
   ENUM_VALUE(ProcessMaxExtCerts,         -2621484)
   ENUM_VALUE(ProcessMaxContexts,         -2621485)
   ENUM_VALUE(ProcessMaxConnections,      -2621486)
   ENUM_VALUE(CertNotExportable,          -2621487)
   ENUM_VALUE(InvalidCertSize,            -2621488)
   ENUM_VALUE(InvalidKeySize,             -2621489)
ENUM_END(NSSLError)

ENUM_BEG(NSSLVersion, uint32_t)
   ENUM_VALUE(Auto,              0x0)
ENUM_END(NSSLVersion)

ENUM_NAMESPACE_EXIT(nsec)

ENUM_NAMESPACE_EXIT(ios)

#include <common/enum_end.inl>

#endif // ifdef IOS_NSEC_ENUM_H
