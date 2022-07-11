#include "joypad.h"

uint8_t buttonStatus = 0;
uint8_t buttonIndex = 0;
bool strobe = false;

bool joypad_read() {
    if (buttonIndex > 7) {
        buttonIndex = 0;
    }
    bool result = (buttonStatus >> buttonIndex) & 1;
    if (!strobe) buttonIndex += 1;
    return result;
}

void joypad_write(uint8_t val) {
    strobe = val & 1;
    if (strobe) buttonIndex = 0;
}

void joypad_setButton(enum JoypadButton button) {
    buttonStatus = buttonStatus | ((uint8_t) button);
    
}

void joypad_unsetButton(enum JoypadButton button) {
    buttonStatus = buttonStatus & ~((uint8_t) button);
}

