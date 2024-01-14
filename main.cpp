#include <iostream>
#include <ogcsys.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include "settings.h"

static void* xfb = nullptr;
static GXRModeObj* rmode = nullptr;

int main() {
  VIDEO_Init();

  rmode = VIDEO_GetPreferredMode(nullptr);
  xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
  console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight,
               rmode->fbWidth * VI_DISPLAY_PIX_SZ);
  VIDEO_Configure(rmode);
  VIDEO_SetNextFramebuffer(xfb);
  VIDEO_SetBlack(FALSE);
  VIDEO_Flush();
  VIDEO_WaitVSync();
  if (rmode->viTVMode & VI_NON_INTERLACE)
    VIDEO_WaitVSync();

  std::cout << "Get Serial Number (c) Open Shop Channel 2024" << std::endl << std::endl;


  ISFS_Initialize();
  CONF_Init();
  WPAD_Init();

  u8* buffer = Settings::GetEncryptedSettings();
  if (buffer != nullptr)
  {
    Settings settings(buffer);
    std::string code = settings.GetValue("CODE");
    code.append(settings.GetValue("SERNO"));

    // WSC returns a serial number with the internal console code and serial number.
    std::cout << "Your Serial Number is: " << std::endl;
    std::cout << code << std::endl << std::endl;
    sleep(5);
    
    std::cout << "Press HOME to return to the Wii Menu." << std::endl;
  }


  while (true) {
    WPAD_ScanPads();
    u32 pressed = WPAD_ButtonsDown(0);

    if (pressed & WPAD_BUTTON_HOME) WII_ReturnToMenu();

    VIDEO_WaitVSync();
  }

  return 0;
}
