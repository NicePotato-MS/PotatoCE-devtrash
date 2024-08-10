#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <keypadc.h>
#include <graphx.h>
#include <fontlibc.h>
#include <srldrvce.h>
#include "gfx/gfx.h"

#include "potatofont.h"
#include "potatoterm.h"

// Woah serial driver thingie stuff wow (not stolen (trust legit))
srl_device_t srl;

#define SERIAL_BAUD 115200

bool hasSrlDevice = false;
srl_error_t srlErr = 0;

uint8_t srlBuf[4096];

uint16_t strBufPtr = 0;

static usb_error_t handle_usb_event(usb_event_t event, void *eventData,
                                    usb_callback_data_t *callbackData __attribute__((unused))) {
    usb_error_t err;
    /* Delegate to srl USB callback */
    // srlState PSRL_STATE_SERIAL is handled by reader (Magic listener)

    if((err = srl_UsbEventCallback(event, eventData, callbackData)) != USB_SUCCESS) {
        //fontlib_DrawString("callback error ");
        //char outStr[16];
        //sprintf(outStr,"%d",err);
        //fontlib_DrawString(strcat(outStr,"\n"));
        return err;
    }
    /* Enable newly connected devices */
    if(event == USB_DEVICE_CONNECTED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE)) {
        usb_device_t device = eventData;
        //fontlib_DrawString("connect event\n");
        usb_ResetDevice(device);
    }

    // Call srl_Open on newly enabled device, if there is not currently a serial device in use
    if(event == USB_HOST_CONFIGURE_EVENT || (event == USB_DEVICE_ENABLED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE))) {
        // If we already have a serial device, ignore the new one
        if(hasSrlDevice) return USB_SUCCESS;
        
        usb_device_t device;
        if(event == USB_HOST_CONFIGURE_EVENT) {
            /* Use the device representing the USB host. */
            //fontlib_DrawString("usb host config event\n");
            device = usb_FindDevice(NULL, NULL, USB_SKIP_HUBS);

            if(device == NULL) return USB_SUCCESS;
        } else {
            // Use the newly enabled device
            //fontlib_DrawString("no host config event\n");
            device = eventData;
        }

        // Initialize the serial library with the newly attached device
        srl_error_t error = srl_Open(&srl, device, srlBuf, sizeof srlBuf, SRL_INTERFACE_ANY, SERIAL_BAUD);
        if(error) {
            srlErr = error;
            //fontlib_DrawString("error ");
            //char outStr[16];
            //sprintf(outStr,"%d",err);
            //fontlib_DrawString(strcat(outStr,"\n"));
            return USB_SUCCESS;
        }

        srlErr = 0;

        hasSrlDevice = true;
    }

    if(event == USB_DEVICE_DISCONNECTED_EVENT) {
        //fontlib_DrawString("disconnect event\n");
        usb_device_t device = eventData;
        if(device == srl.dev) {
            srl_Close(&srl);
            srlErr = -7;
            hasSrlDevice = false;
        }
    }

    return USB_SUCCESS;
}

void prgmEnd(int status) {
    gfx_End();
    usb_Cleanup();
    exit(status);
}

fontlib_font_t *font;
fontlib_font_t *font_bold;

void prgmStart() {
    gfx_Begin();
    gfx_ZeroScreen();

    gfx_SetPalette(xubunterm, sizeof_xubunterm, 0);
    gfx_SetTextFGColor(15);
    gfx_SetTextBGColor(0);

    // Setup font
    font = fontlib_GetFontByIndex("DRMONO", 0);
    if (!font) {
        gfx_PrintStringXY("Please install the DRMONO font.",10,10);
        do {kb_Scan();} while (!kb_IsDown(kb_KeyClear));
        prgmEnd(1);
    }
    font_bold = fontlib_GetFontByIndex("DRMONO", 1);

    fontlib_SetFont(font,0);
    fontlib_SetNewlineOptions(0);
    fontlib_SetWindow(1,0,GFX_LCD_WIDTH,GFX_LCD_HEIGHT);
    fontlib_SetColors(0xFF,0x00);
    fontlib_SetCursorPosition(1,0);
    

    const usb_standard_descriptors_t *desc = srl_GetCDCStandardDescriptors();
    usb_error_t usb_error = usb_Init(handle_usb_event, NULL, desc, USB_DEFAULT_INIT_FLAGS);
    if(usb_error) {
        usb_Cleanup();
        char errorOut[64] = "USB init error";
        char errorStr[16];
        sprintf(errorStr,"%u",usb_error);
        gfx_PrintStringXY(strcat(errorOut,strcat(errorStr,"\n")),10,10);
        do kb_Scan(); while(!kb_IsDown(kb_KeyClear));
        prgmEnd(2);
    }
}

/*

0 - Normal print
1 - Loading control code

*/

#define MAX_ROWS 53
#define MAX_COLS 24

#define X_SIZE 6
#define Y_SIZE 10

#define MAX_X MAX_ROWS*X_SIZE
#define MAX_Y MAX_COLS*Y_SIZE

uint8_t termOldFG;
uint8_t termOldBG;
uint8_t termCursorCharPointer;
char termCursorChar[X_SIZE*Y_SIZE];

void termCursorOff() {
    if(fontlib_GetCursorX() <= MAX_X) {
        termCursorCharPointer = 0;
        uint8_t *character = &gfx_vram[fontlib_GetCursorX()+1+fontlib_GetCursorY()*GFX_LCD_WIDTH];
        for (size_t x = 0; x < X_SIZE; x++) {
            for (size_t y = 0; y < Y_SIZE; y++) {
                character[x+y*GFX_LCD_WIDTH] = termCursorChar[termCursorCharPointer];
                termCursorCharPointer++;
            }
        }
    }
}

void termCursorOn() {
    if(fontlib_GetCursorX() <= MAX_X) {
        termCursorCharPointer = 0;
        uint8_t *character = &gfx_vram[fontlib_GetCursorX()+1+fontlib_GetCursorY()*GFX_LCD_WIDTH];
        unsigned int oldX = fontlib_GetCursorX();
        for (size_t x = 0; x < X_SIZE; x++) {
            for (size_t y = 0; y < Y_SIZE; y++) {
                termCursorChar[termCursorCharPointer] = character[x+y*GFX_LCD_WIDTH];
                termCursorCharPointer++;
            }
        }
        //termOldFG = fontlib_GetForegroundColor;
        //termOldFG = fontlib_GetBackgroundColor;
        //fontlib_SetColors
        fontlib_DrawGlyph(8);
        fontlib_SetCursorPosition(oldX,fontlib_GetCursorY());
    }
    
    
}

void termNewline() {
    if(fontlib_Newline()) {
        fontlib_ScrollWindowDown();
        fontlib_ClearEOL();
    }
}

void termPutChar(char character) {
    if(character == '\n') {
        termNewline();
    } else {
        fontlib_DrawGlyph(character);
        if(fontlib_GetCursorX() <= MAX_X) {
            termNewline();
        }
    }
    

}

void termPrint(char *str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        termPutChar(str[i]);
    }
}

char debugString[256];

int controlMode = 0;
int controlPointer = 0;
char controlCommand; // e.g. '['
unsigned int argBufferPointer;
char argBuffer[64]; // For storing the current arg while collecting
unsigned int controlArgsPointer;
unsigned int controlArgs[5]; // Max 5 args

char cursorCharacter; // Character under cursor

int cursorChange; // for character move functions
unsigned int newX;
uint8_t newY;

void registerArg() {
    if(controlArgsPointer > sizeof(controlArgs)) { // No more room for args, ignore
        return;
    } else { // Room for another arg, collect it
        argBuffer[argBufferPointer] = 0; // Set last character to NUL for atoi
        controlArgs[controlArgsPointer] = atoi(argBuffer); // Collect arg
        controlArgsPointer++;
        argBufferPointer = 0; // Reset arg buffer for next arg
    }
}

void loop() {
    kb_Scan();
    usb_HandleEvents();

    if(hasSrlDevice) {
        char inBuf[512];

        // Read from the serial buffer
        uint8_t bytes_read = srl_Read(&srl, inBuf, sizeof inBuf);

        // Check for an error (e.g. device disconneced)
        if(bytes_read < 0) {
            char errorStr[32];
            sprintf(errorStr,"%u",bytes_read);
            fontlib_DrawString(strcat("Error ",strcat(errorStr," on srl_Read\n")));
            hasSrlDevice = false;
        } else if(bytes_read > 0) {
            termCursorOff();
            for(unsigned int i=0;i<bytes_read;i++) {
                switch(controlMode) {
                    case 0: // Text print mode
                        switch(inBuf[i]) {
                            case 27:
                                // Escape and search for control code
                                controlMode = 1;
                                break;
                            default:
                                termPutChar(inBuf[i]);
                                break;
                        }
                        break;
                    case 1: // Search for control code
                        if(inBuf[i] == ' '){break;} // Skip whitespace
                        if(64 <= inBuf[i] && inBuf[i]<=95) { // Command character
                            controlCommand = inBuf[i];
                            controlMode = 2; // Start gathering args
                            argBufferPointer = 0;
                            controlArgsPointer = 0;
                            memset(controlArgs,0,sizeof(controlArgs)); // Reset args to 0
                        }
                        break;
                    case 2: // Gather args
                        if(48 <= inBuf[i] && inBuf[i]<=57) { // Is number, collect in buffer
                            if(argBufferPointer == sizeof(argBuffer)) { // Arg is overflowing, ignore
                                break;
                            } else { // Room in arg buffer, collect
                                argBuffer[argBufferPointer] = inBuf[i];
                                argBufferPointer++;
                                break;
                            }
                        }
                        if(inBuf[i] == ';') { // Arg delimiter, get ready for next arg
                            registerArg();
                            break;
                        }
                        if(64 <= inBuf[i] && inBuf[i]<=126) { // Function name character, execute
                            registerArg();
                            switch (controlCommand) { // Pretty much always '['
                                case '[':
                                    switch(inBuf[i]) {
                                        case 'H': // Cursor to position
                                        case 'f': // same
                                            if(controlArgs[0] > MAX_ROWS) {
                                                newX = MAX_X;
                                            } else {
                                                newX = controlArgs[1]*X_SIZE;
                                            }
                                            if(controlArgs[1] > MAX_COLS) {
                                                newY = MAX_Y;
                                            } else {
                                                newY = controlArgs[1]*Y_SIZE;
                                            }
                                            fontlib_SetCursorPosition(newX,newY);
                                            break;
                                        case 'A': // Move cursor up
                                            cursorChange = controlArgs[0]*Y_SIZE;
                                            if(fontlib_GetCursorY()>=cursorChange) { // Only move if not overflowing top
                                                fontlib_SetCursorPosition(fontlib_GetCursorX(),fontlib_GetCursorY()-cursorChange);
                                            } else {
                                                fontlib_SetCursorPosition(fontlib_GetCursorX(),0);
                                            }
                                            break;
                                        case 'B': // Move cursor down
                                            cursorChange = fontlib_GetCursorY()+controlArgs[0]*Y_SIZE;
                                            if(cursorChange > MAX_Y) {
                                                cursorChange = MAX_Y;
                                            }
                                            fontlib_SetCursorPosition(fontlib_GetCursorX(),cursorChange);
                                            break;
                                        case 'C': // Move cursor right
                                            cursorChange = fontlib_GetCursorX()+controlArgs[0]*X_SIZE;
                                            if(cursorChange > MAX_X) {
                                                cursorChange = MAX_X;
                                            }
                                            fontlib_SetCursorPosition(cursorChange,fontlib_GetCursorY());
                                            break;
                                        case 'D': // Move cursor left
                                            cursorChange = controlArgs[0]*X_SIZE;
                                            if((int)fontlib_GetCursorX()>=cursorChange) { // Only move if not overflowing top
                                                fontlib_SetCursorPosition(fontlib_GetCursorX()-cursorChange,fontlib_GetCursorY());
                                            } else {
                                                fontlib_SetCursorPosition(0,fontlib_GetCursorY());
                                            }
                                            break;
                                        case 'm': // Text style
                                            switch(controlArgs[0]) {
                                                case 0: // Reset to default
                                                    fontlib_SetColors(15,0);
                                                    fontlib_SetFont(font,0);
                                                    break;
                                                case 1: // Bold
                                                    fontlib_SetFont(font_bold,0);
                                                    break;
                                                case 38: // Custom FG
                                                    switch(controlArgs[1]) {
                                                        case 5:
                                                            fontlib_SetForegroundColor(controlArgs[2]);
                                                            break;
                                                        case 2:
                                                            // TODO: rgb?
                                                            break;
                                                    }
                                                    break;
                                                case 48: // Custom BG
                                                    switch(controlArgs[1]) {
                                                        case 5:
                                                            fontlib_SetBackgroundColor(controlArgs[2]);
                                                            break;
                                                        case 2:
                                                            // TODO: rgb?
                                                            break;
                                                    }
                                                    break;
                                                default:
                                                    if(30<=controlArgs[0] && controlArgs[0]<=37) { // Reg FG
                                                        fontlib_SetForegroundColor(controlArgs[0]-30);
                                                        break;
                                                    }
                                                    if(40<=controlArgs[0] && controlArgs[0]<=47) { // Reg BG
                                                        fontlib_SetBackgroundColor(controlArgs[0]-40);
                                                        break;
                                                    }
                                                    if(90<=controlArgs[0] && controlArgs[0]<=97) { // Bright FG
                                                        fontlib_SetForegroundColor(controlArgs[0]-81);
                                                        break;
                                                    }
                                                    if(100<=controlArgs[0] && controlArgs[0]<=107) { // Bright BG
                                                        fontlib_SetBackgroundColor(controlArgs[0]-91);
                                                        break;
                                                    }
                                            }
                                            break;
                                        }
                                    break;
                                default:
                                    break;
                            }
                            controlMode = 0;
                        }
                        break;
                }
            }
            termCursorOn();
        }
    }
}

int main(void)
{
    prgmStart();

    potatofont_ReadyFonts();
    //potatofont_DrawGlyph5x10('A',1,1);

    do {
        //loop();
    } while (!kb_IsDown(kb_KeyClear));

    prgmEnd(0);
}