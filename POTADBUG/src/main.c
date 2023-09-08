#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <sys/timers.h>

#include <keypadc.h>
#include <graphx.h>
#include <fontlibc.h>
#include <srldrvce.h>

#include "gfx/gfx.h"


/* 
// Serial host magic "PCEHOST"
// Serial calc magic "PCECALC"
// Magics must be sent back and forth on connection or no serial
//
*/



// GUI config

#define STATUS_BAR_SIZE 16

// Colors

#define COLOR_BLACK 0x00
#define COLOR_TRANS 0x01
#define COLOR_WHITE 0x02
#define COLOR_DARK_GREY 0x03
#define COLOR_RED 0x04
#define COLOR_BLUE 0x05


#define SERIAL_BAUD 115200

// Woah serial driver thingie stuff wow (not stolen (trust legit))

srl_device_t srl;

bool hasSrlDevice = false;
srl_error_t srlErr = 0;

uint8_t srlBuf[512];

uint8_t srlState = 0;
uint8_t oldSrlState = 255; // For drawing

#define PSRL_STATE_DISCONNECTED 0
#define PSRL_STATE_POWER 1          // Connected but no serial
#define PSRL_STATE_SERIAL 2         // Connected with serial


static usb_error_t handle_usb_event(usb_event_t event, void *eventData,
                                    usb_callback_data_t *callbackData __attribute__((unused))) {
    usb_error_t err;
    /* Delegate to srl USB callback */

    if((err = srl_UsbEventCallback(event, eventData, callbackData)) != USB_SUCCESS) {
        fontlib_DrawString("callback error ");
        char outStr[16];
        sprintf(outStr,"%d",err);
        fontlib_DrawString(strcat(outStr,"\n"));
        return err;
    }
    /* Enable newly connected devices */
    if(event == USB_DEVICE_CONNECTED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE)) {
        usb_device_t device = eventData;
        srlConnected = true;
        fontlib_DrawString("connect event\n");
        usb_ResetDevice(device);
    }

    // Call srl_Open on newly enabled device, if there is not currently a serial device in use
    if(event == USB_HOST_CONFIGURE_EVENT || (event == USB_DEVICE_ENABLED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE))) {
        // If we already have a serial device, ignore the new one
        if(hasSrlDevice) return USB_SUCCESS;
        
        if(event == USB_DEVICE_ENABLED_EVENT) srlState = PSRL_STATE_DISCONNECTED;
        if(event == USB_DEVICE_SUSPENDED_EVENT) srlState = PSRL_STATE_POWER;
        // srlState PSRL_STATE_SERIAL is handled by reader (Magic listener)
        
        usb_device_t device;
        if(event == USB_HOST_CONFIGURE_EVENT) {
            /* Use the device representing the USB host. */
            fontlib_DrawString("usb host config event\n");
            device = usb_FindDevice(NULL, NULL, USB_SKIP_HUBS);

            if(device == NULL) return USB_SUCCESS;
        } else {
            // Use the newly enabled device
            fontlib_DrawString("no host config event\n");
            device = eventData;
        }

        // Initialize the serial library with the newly attached device
        srl_error_t error = srl_Open(&srl, device, srlBuf, sizeof srlBuf, SRL_INTERFACE_ANY, SERIAL_BAUD);
        if(error) {
            srlErr = error;
            fontlib_DrawString("error ");
            char outStr[16];
            sprintf(outStr,"%d",err);
            fontlib_DrawString(strcat(outStr,"\n"));
            return USB_SUCCESS;
        }

        srlErr = 0;

        hasSrlDevice = true;
    }

    if(event == USB_DEVICE_DISCONNECTED_EVENT) {
        fontlib_DrawString("disconnect event\n");
        usb_device_t device = eventData;
        if(device == srl.dev) {
            srl_Close(&srl);
            srlErr = -7;
            hasSrlDevice = false;
            srlConnected = false;
        }
    }

    return USB_SUCCESS;
}

void prgmEnd() {
    usb_Cleanup();
    gfx_End();
}

void prgmStart() {
    gfx_Begin();
    gfx_ZeroScreen();

    fontlib_font_t *font;

    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTextTransparentColor(COLOR_TRANS);
    gfx_SetTextFGColor(COLOR_WHITE);
    gfx_SetTextBGColor(COLOR_BLACK);

    // Setup font
    font = fontlib_GetFontByIndex("DRSANS", 3);
    if (!font) {
        gfx_PrintStringXY("DRSANS appvar not found or invalid",2,20);
        do {kb_Scan();} while (!kb_IsDown(kb_KeyClear));
        prgmEnd();
        exit(-1);
    }
    // Setup text window
    fontlib_SetFont(font, 0);
    fontlib_SetColors(COLOR_WHITE, COLOR_BLACK);
    fontlib_SetLineSpacing(1, 1);
    fontlib_SetWindow(0,STATUS_BAR_SIZE,GFX_LCD_WIDTH,GFX_LCD_HEIGHT-STATUS_BAR_SIZE);
    fontlib_HomeUp();
    fontlib_SetNewlineOptions(FONTLIB_ENABLE_AUTO_WRAP | FONTLIB_AUTO_SCROLL | FONTLIB_PRECLEAR_NEWLINE);

    // Setup Status Bar
    gfx_SetColor(COLOR_DARK_GREY);
    gfx_FillRectangle_NoClip(0,0,GFX_LCD_WIDTH,STATUS_BAR_SIZE);


    // Setup USB
    const usb_standard_descriptors_t *desc = srl_GetCDCStandardDescriptors();
    usb_error_t usb_error = usb_Init(handle_usb_event, NULL, desc, USB_DEFAULT_INIT_FLAGS);
    if(usb_error) {
        usb_Cleanup();
        char errorStr[32];
        sprintf(errorStr,"%u",usb_error);
        gfx_PrintStringXY(strcat("USB init error ",strcat(errorStr,"\n")),2,20);
        do kb_Scan(); while(!kb_IsDown(kb_KeyClear));
        prgmEnd();
        exit(-2);
    }
}

void loop() {
    kb_Scan();
    usb_HandleEvents();

    if(hasSrlDevice) {
        char in_buf[64];

        // Read up to 64 bytes from the serial buffer
        uint8_t bytes_read = srl_Read(&srl, in_buf, sizeof in_buf);

        // Check for an error (e.g. device disconneced)
        if(bytes_read < 0) {
            char errorStr[32];
            sprintf(errorStr,"%u",bytes_read);
            fontlib_DrawString(strcat("Error ",strcat(errorStr," on srl_Read\n")));
            hasSrlDevice = false;
            srlState = PSRL_STATE_POWER;
        } else if(bytes_read > 0) {
            /* Write the data back to serial */
            char outStr[16];
            for(unsigned int i=0;i<bytes_read;i++) {
                sprintf(outStr,"%u",in_buf[i]);
                fontlib_DrawString(strcat(outStr,","));
            }
            srl_Write(&srl, in_buf, bytes_read);
        }
    }
}

void draw() {
    if(oldSrlConnected!=srlConnected) {
        oldSrlConnected = srlConnected;
        if(hasSrlDevice) {
            gfx_Sprite_NoClip(plug_on,GFX_LCD_WIDTH-plug_on_width,0);
        } else {
            gfx_Sprite_NoClip(plug_off,GFX_LCD_WIDTH-plug_off_width,0);
        }
    }
}

/* Main function, called first */
int main(void)
{
    prgmStart();

    do {
        loop();
        draw();
    } while (!kb_IsDown(kb_KeyClear));

    /* Return 0 for success */
    prgmEnd();
    return 0;
}