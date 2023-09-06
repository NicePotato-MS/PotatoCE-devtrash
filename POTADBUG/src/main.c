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



// Woah serial driver thingie stuff wow (not stolen (trust legit))

srl_device_t srl;

bool hasSrlDevice = false;
srl_error_t srlErr = 0;

uint8_t srlBuf[512];

static usb_error_t handle_usb_event(usb_event_t event, void *eventData,
                                    usb_callback_data_t *callbackData __attribute__((unused))) {
    usb_error_t err;
    /* Delegate to srl USB callback */
    if ((err = srl_UsbEventCallback(event, eventData, callbackData)) != USB_SUCCESS)
        return err;
    /* Enable newly connected devices */
    if(event == USB_DEVICE_CONNECTED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE)) {
        usb_device_t device = eventData;
        usb_ResetDevice(device);
    }

    /* Call srl_Open on newly enabled device, if there is not currently a serial device in use */
    if(event == USB_HOST_CONFIGURE_EVENT || (event == USB_DEVICE_ENABLED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE))) {

        /* If we already have a serial device, ignore the new one */
        if(hasSrlDevice) return USB_SUCCESS;

        usb_device_t device;
        if(event == USB_HOST_CONFIGURE_EVENT) {
            /* Use the device representing the USB host. */
            device = usb_FindDevice(NULL, NULL, USB_SKIP_HUBS);
            if(device == NULL) return USB_SUCCESS;
        } else {
            /* Use the newly enabled device */
            device = eventData;
        }

        /* Initialize the serial library with the newly attached device */
        srl_error_t error = srl_Open(&srl, device, srlBuf, sizeof srlBuf, SRL_INTERFACE_ANY, 9600);
        if(error) {
            srlErr = error;
            return USB_SUCCESS;
        }
        srlErr = 0;
        hasSrlDevice = true;
    }

    if(event == USB_DEVICE_DISCONNECTED_EVENT) {
        usb_device_t device = eventData;
        if(device == srl.dev) {
            srl_Close(&srl);
            srlErr = -7;
            hasSrlDevice = false;
        }
    }

    return USB_SUCCESS;
}

void close() {
    usb_Cleanup();
    gfx_End();
}

/* Main function, called first */
int main(void)
{
    gfx_Begin();
    gfx_ZeroScreen();

    fontlib_font_t *font;

    gfx_SetTextTransparentColor(0);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextBGColor(0x00);

    /* Get the first font present in the font pack */
    font = fontlib_GetFontByIndex("DRSANS", 3);
    /* This check is important! If fetching the font fails, trying to use the font will go . . . poorly. */
    if (!font) {
        gfx_PrintStringXY("DRSANS appvar not found or invalid",2,2);
        do {kb_Scan();} while (!kb_IsDown(kb_KeyClear));
        close();
        return -1;
    }
    /* Use font for whatever */
    fontlib_SetFont(font, 0);
    fontlib_SetColors(0xFF, 0x0);
    fontlib_SetLineSpacing(1, 1);
    fontlib_HomeUp();
    fontlib_SetNewlineOptions(FONTLIB_ENABLE_AUTO_WRAP | FONTLIB_AUTO_SCROLL);

    // Setup USB
    const usb_standard_descriptors_t *desc = srl_GetCDCStandardDescriptors();
    /* Initialize the USB driver with our event handler and the serial device descriptors */
    usb_error_t usb_error = usb_Init(handle_usb_event, NULL, desc, USB_DEFAULT_INIT_FLAGS);
    if(usb_error) {
        usb_Cleanup();
        char errorStr[32];
        sprintf(errorStr,"%u",usb_error);
        fontlib_DrawString(strcat("USB init error ",strcat(errorStr,"\n")));
        do kb_Scan(); while(!kb_IsDown(kb_KeyClear));
        return 1;
    }

    do {
        kb_Scan();
        usb_HandleEvents();

        if(hasSrlDevice) {
            char in_buf[64];

            /* Read up to 64 bytes from the serial buffer */
            uint8_t bytes_read = srl_Read(&srl, in_buf, sizeof in_buf);

            /* Check for an error (e.g. device disconneced) */
            if(bytes_read < 0) {
                char errorStr[32];
                sprintf(errorStr,"%u",bytes_read);
                fontlib_DrawString(strcat("Error ",strcat(errorStr," on srl_Read\n")));
                hasSrlDevice = false;
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
    } while (!kb_IsDown(kb_KeyClear));

    /* Return 0 for success */
    close();
    return 0;
}