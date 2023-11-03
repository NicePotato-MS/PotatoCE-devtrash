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

#include "main.h"
#include "gfx/gfx.h"


/* 
// Serial host magic "PCEHOST"
// Serial calc magic "PCECALC"
// Magics must be sent back and forth on connection or no serial
//
*/


// Woah serial driver thingie stuff wow (not stolen (trust legit))

srl_device_t srl;

bool hasSrlDevice = false;
srl_error_t srlErr = 0;

uint8_t srlBuf[512];

uint8_t srlState = 0;
uint8_t oldSrlState = 255; // For drawing

#define SRL_STR_BUF_SIZE 1024

char srlStrBuf[SRL_STR_BUF_SIZE];
uint16_t strBufPtr = 0;

static usb_error_t handle_usb_event(usb_event_t event, void *eventData,
                                    usb_callback_data_t *callbackData __attribute__((unused))) {
    usb_error_t err;
    /* Delegate to srl USB callback */

    if(event == USB_DEVICE_ENABLED_EVENT) srlState = PSRL_STATE_POWER;
    if(event == USB_DEVICE_SUSPENDED_EVENT) srlState = PSRL_STATE_DISCONNECTED;
    // srlState PSRL_STATE_SERIAL is handled by reader (Magic listener)

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
        fontlib_DrawString("connect event\n");
        usb_ResetDevice(device);
    }

    // Call srl_Open on newly enabled device, if there is not currently a serial device in use
    if(event == USB_HOST_CONFIGURE_EVENT || (event == USB_DEVICE_ENABLED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE))) {
        // If we already have a serial device, ignore the new one
        if(hasSrlDevice) return USB_SUCCESS;
        
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

    gfx_SetPalette(dark_palette, sizeof_dark_palette, 0);
    gfx_SetTextTransparentColor(COLOR_TRANS);
    gfx_SetTextFGColor(COLOR_FG);
    gfx_SetTextBGColor(COLOR_BG);

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
    fontlib_SetColors(COLOR_FG, COLOR_BG);
    fontlib_SetLineSpacing(1, 1);
    fontlib_SetWindow(0,STATUS_BAR_SIZE,GFX_LCD_WIDTH,GFX_LCD_HEIGHT-STATUS_BAR_SIZE);
    fontlib_HomeUp();
    fontlib_SetNewlineOptions(FONTLIB_ENABLE_AUTO_WRAP | FONTLIB_AUTO_SCROLL | FONTLIB_PRECLEAR_NEWLINE);

    fontlib_DrawString("0123456789ABCDEFGHIJKLMNOPQRSTUV");

    // Setup Status Bar
    gfx_SetColor(COLOR_TOPBAR);
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
        char inBuf[64];

        // Read up to 64 bytes from the serial buffer
        uint8_t bytes_read = srl_Read(&srl, inBuf, sizeof inBuf);

        // Check for an error (e.g. device disconneced)
        if(bytes_read < 0) {
            char errorStr[32];
            sprintf(errorStr,"%u",bytes_read);
            fontlib_DrawString(strcat("Error ",strcat(errorStr," on srl_Read\n")));
            hasSrlDevice = false;
            srlState = PSRL_STATE_POWER;
        } else if(bytes_read > 0) {
            gfx_vram[GFX_LCD_WIDTH*10-3] = COLOR_YELLOW;
            /* Write the data back to serial */
            for(unsigned int i=0;i<bytes_read;i++) {
                if(inBuf[i] == "\0") {

                } else {
                    
                }
            }
            srl_Write(&srl, inBuf, bytes_read);
        } //else {gfx_vram[0] = COLOR_RED;}
    }

    //No write so just put red
    gfx_vram[1] = COLOR_RED;
}

void draw() {
    if(oldSrlState!=srlState) {
        oldSrlState = srlState;
        switch (srlState) {
            case PSRL_STATE_DISCONNECTED: gfx_Sprite_NoClip(plug_disconnected,GFX_LCD_WIDTH-plug_disconnected_width,0); break;
            case PSRL_STATE_POWER: gfx_Sprite_NoClip(plug_power,GFX_LCD_WIDTH-plug_power_width,0); break;
            case PSRL_STATE_SERIAL: gfx_Sprite_NoClip(plug_serial,GFX_LCD_WIDTH-plug_serial_width,0); break;
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

    prgmEnd();
    return 0;
}