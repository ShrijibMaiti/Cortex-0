#include <stdint.h>
#include <stddef.h>

/* --- HARDWARE I/O --- */
void outb(uint16_t port, uint8_t val) { asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) ); }
uint8_t inb(uint16_t port) { uint8_t ret; asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) ); return ret; }

/* --- VGA DRIVER --- */
enum vga_color { VGA_COLOR_BLACK = 0, VGA_COLOR_LIGHT_GREY = 7, VGA_COLOR_WHITE = 15 };
static const size_t VGA_WIDTH = 80; static const size_t VGA_HEIGHT = 25;
size_t terminal_row; size_t terminal_column; uint8_t terminal_color; uint16_t* terminal_buffer;

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) { return (uint16_t) uc | (uint16_t) color << 8; }

void terminal_initialize(void) {
    terminal_row = 0; terminal_column = 0;
    terminal_color = vga_entry(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_putchar(char c) {
    if (c == '\n') { terminal_column = 0; terminal_row++; return; }
    if (c == '\b') { // Handle Backspace
        if (terminal_column > 0) {
            terminal_column--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_column;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
        return;
    }
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = vga_entry(c, terminal_color);
    if (++terminal_column == VGA_WIDTH) { terminal_column = 0; terminal_row++; }
}

void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) terminal_putchar(data[i]);
}

/* --- HELPER: READ CPU CYCLES (AMD FLEX) --- */
uint64_t rdtsc() {
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/* --- HELPER: INTEGER TO STRING --- */
void print_dec(uint32_t n) {
    if (n == 0) { terminal_putchar('0'); return; }
    uint32_t acc = n; char c[32]; int i = 0;
    while (acc > 0) { c[i] = '0' + (acc % 10); acc /= 10; i++; }
    char c2[32]; c2[i] = 0; int j = 0; i--;
    while(i >= 0) { c2[j++] = c[i--]; }
    terminal_writestring(c2);
}

/* --- HELPER: STRING COMPARE --- */
int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* --- HELPER: READ CPU VENDOR STRING (CPUID) --- */
void print_cpu_vendor() {
    uint32_t eax = 0, ebx, ecx, edx;
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
    char vendor[13];
    vendor[0] = ebx; vendor[1] = ebx>>8; vendor[2] = ebx>>16; vendor[3] = ebx>>24;
    vendor[4] = edx; vendor[5] = edx>>8; vendor[6] = edx>>16; vendor[7] = edx>>24;
    vendor[8] = ecx; vendor[9] = ecx>>8; vendor[10] = ecx>>16; vendor[11] = ecx>>24;
    vendor[12] = '\0';
    terminal_writestring("Silicon Identity: ");
    terminal_writestring(vendor);
    terminal_writestring("\n");
}

/* --- THE BRAIN (INPUT BUFFER) --- */
char input_buffer[256];
int buffer_pos = 0;

void execute_command() {
    terminal_writestring("\n"); // New line after Enter
    
    if (strcmp(input_buffer, "PHYSICS") == 0 || strcmp(input_buffer, "physics") == 0) {
        terminal_writestring("[AI-CORE] Simulating Physics Engine...\n");
        terminal_writestring("Gravity: 9.81 m/s^2 | Velocity: 0 m/s\n");
    } 
    else if (strcmp(input_buffer, "CPU") == 0 || strcmp(input_buffer, "cpu") == 0) {
        terminal_writestring("[HARDWARE] Reading CPU Time-Stamp Counter...\n");
        uint64_t cycles = rdtsc();
        terminal_writestring("Cycles: "); print_dec((uint32_t)cycles); terminal_writestring("\n");
        terminal_writestring("Status: 32-bit Protected Mode | Ring 0 Active\n");
    }
    else if (strcmp(input_buffer, "VENDOR") == 0 || strcmp(input_buffer, "vendor") == 0) {
        terminal_writestring("[HARDWARE SCAN] Interrogating Processor...\n");
        print_cpu_vendor();
    }
    else if (strcmp(input_buffer, "HELP") == 0 || strcmp(input_buffer, "help") == 0) {
        terminal_writestring("Available Commands:\n");
        terminal_writestring(" - physics : Run physics simulation\n");
        terminal_writestring(" - cpu     : Show raw hardware stats (RDTSC)\n");
        terminal_writestring(" - vendor  : Interrogate silicon manufacturer\n");
        terminal_writestring(" - help    : Show this menu\n");
    }
    else if (buffer_pos > 0) { // Only print unknown if they actually typed something
        terminal_writestring("Unknown command: ");
        terminal_writestring(input_buffer);
        terminal_writestring("\n");
    }
    
    // Reset Prompt
    terminal_writestring("> ");
    buffer_pos = 0;
    for(int i=0; i<256; i++) input_buffer[i] = 0; // Clear buffer
}

/* --- KEYBOARD INPUT HANDLER --- */
void kernel_handle_input(char c) {
    if (c == '\n') {
        execute_command();
    } else if (c == '\b') {
        if (buffer_pos > 0) {
            buffer_pos--;
            input_buffer[buffer_pos] = 0;
            terminal_putchar('\b'); // Remove from screen
        }
    } else {
        if (buffer_pos < 255) {
            input_buffer[buffer_pos] = c;
            buffer_pos++;
            input_buffer[buffer_pos] = 0; // Null terminate
            terminal_putchar(c); // Echo to screen
        }
    }
}

void pic_remap() {
    outb(0x20, 0x11); outb(0xA0, 0x11); outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02); outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0xFD); outb(0xA1, 0xFF);
}

extern void gdt_install();
extern void idt_install();

void kmain(void) {
    gdt_install();
    terminal_initialize();
    pic_remap();
    idt_install();
    asm volatile("sti");

    terminal_writestring("Cortex-0 AI-Native Kernel [AMD EDITION]\n");
    terminal_writestring("Core Systems: ONLINE\n");
    terminal_writestring("Type 'help' for commands.\n");
    terminal_writestring("----------------------------\n> ");

    while(1) { asm volatile("hlt"); } // Wait for interrupt
}
