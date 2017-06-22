#pragma once
#include "sci_enum.h"

#include <array>
#include <cstdint>
#include <common/be_val.h>
#include <common/structsize.h>

namespace sci
{

struct SCISpotPassSettings
{
   be_val<uint8_t> version;
   be_val<uint8_t> enable;
   be_val<uint8_t> auto_dl_app;
   std::array<uint8_t, 12> upload_console_info;
};
CHECK_OFFSET(SCISpotPassSettings, 0x00, version);
CHECK_OFFSET(SCISpotPassSettings, 0x01, enable);
CHECK_OFFSET(SCISpotPassSettings, 0x02, auto_dl_app);
CHECK_OFFSET(SCISpotPassSettings, 0x03, upload_console_info);
CHECK_SIZE(SCISpotPassSettings, 0x0F);

SCIError
SCIInitSpotPassSettings(SCISpotPassSettings *data);

} // namespace sci
