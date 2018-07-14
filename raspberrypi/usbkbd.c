#include <stdio.h>
#include <stdint.h>
#ifdef MICROPY_HW_USBHOST
#include "usbd/usbd.h"
#include "device/hid/keyboard.h"

#include "rpi.h"
#include "usbkbd.h"

static unsigned int kbd_addr;
static unsigned int keys[6];

#define USBKBD_KEYMAPSIZE (104)

unsigned char keymap_us[2][USBKBD_KEYMAPSIZE] = {
    {
        0x0, 0x1, 0x2, 0x3, 'a', 'b', 'c', 'd',
        'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
        'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
        '3', '4', '5', '6', '7', '8', '9', '0',
        0xd, 0x1b, 0x8, 0x9, ' ', '-', '=', '[',
        ']', '\\', 0x0, ';', '\'', '`', ',', '.',
        '/', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x12, 0x0, 0x0, 0x7f, 0x0, 0x0, 0x1c,
        0x1d, 0x1f, 0x1e, 0x0, '/', '*', '-', '+',
        0xd, '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', '.', '\\', 0x0, 0x0, '=',
    },
    {
        0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D',
        'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
        '#', '$', '%', '^', '&', '*', '(', ')',
        0xa, 0x1b, '\b', '\t', ' ', '_', '+', '{',
        '}', '|', '~', ':', '"', '~', '<', '>',
        '?', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+',
        0xa, '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', '.', '|', 0x0, 0x0, '=',
    }
};

unsigned char keymap_jp[2][USBKBD_KEYMAPSIZE] = {
    {
        0x0, 0x1, 0x2, 0x3, 'a', 'b', 'c', 'd',
        'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
        'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
        '3', '4', '5', '6', '7', '8', '9', '0',
        0xd, 0x1b, 0x8, 0x9, ' ', '-', '^', '[',
        ']', '\\', 0x0, ';', '\'', '`', ',', '.',
        '/', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x12, 0x0, 0x0, 0x7f, 0x0, 0x0, 0x1c,
        0x1d, 0x1f, 0x1e, 0x0, '/', '*', '-', '+',
        0xd, '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', '.', '\\', 0x0, 0x0, '=',
    },
    {
        0x0, 0x0, 0x0, 0x0, 'A', 'B', 'C', 'D',
        'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
        '#', '$', '%', '^', '&', '*', '(', ')',
        0xa, 0x1b, '\b', '\t', ' ', '_', '+', '{',
        '}', '|', '~', ':', '"', '~', '<', '>',
        '?', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, '/', '*', '-', '+',
        0xa, '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', '.', '|', 0x0, 0x0, '=',
    }
};

unsigned char keycode2char_us(int k, unsigned char shift) {
    if (k > 103) {
        return 0;
    } else {
        return keymap_us[(shift == 0) ? 0 : 1][k];
    }
}

static f_keycode2char_t _keycode2char = keycode2char_us;

// usbkbd_getc() returns -1 if no input
int usbkbd_getc() {
    unsigned int key;
    struct KeyboardModifiers mod;
    int result = -1;
    
    if (kbd_addr == 0) {
        // Is there a keyboard ?
        UsbCheckForChange();
        if (KeyboardCount() > 0) {
            kbd_addr = KeyboardGetAddress(0);
        }
    }

    if (kbd_addr != 0) {
        for(int i = 0; i < 6; i++) {
            // Read and print each keycode of pressed keys
            key = KeyboardGetKeyDown(kbd_addr, i);
            if (key != keys[0] && key != keys[1] && key != keys[2] && \
                key != keys[3] && key != keys[4] && key != keys[5] && key) {
                mod = KeyboardGetModifiers(kbd_addr);
                if (mod.RightControl | mod.LeftControl) {
                    unsigned char c = (*_keycode2char)(key, 1);
                    switch(c) {
                    case 'A': result = 1; break;
                    case 'B': result = 2; break;
                    case 'C': result = 3; break;
                    case 'D': result = 4; break;
                    case 'E': result = 5; break;
                    case 'F': result = 6; break;
                    case 'K': result = 11; break;
                    case 'N': result = 14; break;
                    case 'P': result = 16; break;
                    case 'U': result = 21; break;
                    default: result = 0;
                    }
                } else {
                    result = (*_keycode2char)(key, mod.RightShift | mod.LeftShift);
                    // convert arrow keys to EMACS binding controls
                    switch(result) {
                    case 0x1d: result = 2; break;
                    case 0x1c: result = 6; break;
                    case 0x1f: result = 14; break;
                    case 0x1e: result = 16; break;
                    default: break;
                    }
                }
            }
            keys[i] = key;
        }

        if (KeyboardPoll(kbd_addr) != 0) {
            kbd_addr = 0;
        }
    }
    return result;
}
#endif

