#include <stdint.h>

extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);

/* Link to the function we just wrote in kernel.c */
extern void kernel_handle_input(char c);

unsigned char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
};

void keyboard_handler() {
    uint8_t scancode = inb(0x60);

    if (scancode & 0x80) {
        // Key released - ignore
    } else {
        char key = kbdus[scancode];
        if (key != 0) {
            /* Send the key to the kernel brain! */
            kernel_handle_input(key);
        }
    }
}
