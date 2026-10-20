#include <stdint.h>
#include <stddef.h>

/* ============ VGA CONSTANTS ============ */
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

/* ============ COLORS ============ */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

/* ============ TERMINAL STATE ============ */
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

/* ============ HELPER FUNCTIONS ============ */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)color << 8 | uc;
}

/* ============ TERMINAL FUNCTIONS ============ */

/* Clear screen and reset cursor to top-left */
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
        }
    }
}

/* Change text color */
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

/* Put char at specific position */
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    terminal_buffer[y * VGA_WIDTH + x] = vga_entry(c, color);
}

/* Scroll screen up by one line */
void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    /* Clear last line */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    terminal_row = VGA_HEIGHT - 1;
}

/* Put char with newline and scrolling support */
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }
    
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

/* Write string */
void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

/* ============ NUMBER OUTPUT ============ */

/* Print hex number (for addresses) */
void terminal_write_hex(uint32_t num) {
    terminal_writestring("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uint32_t nibble = (num >> i) & 0xF;
        terminal_putchar((nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10));
    }
}

/* Print decimal number */
void terminal_write_dec(uint32_t num) {
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    char buf[12];
    int i = 0;
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (i > 0) {
        terminal_putchar(buf[--i]);
    }
}

/* ============ KERNEL MAIN ============ */
void kernel_main(void) {
    terminal_initialize();
    
    /* Welcome message */
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("========================================\n");
    terminal_writestring("     Welcome to LapiaOS [Prototype2/02.08.26]\n");
    terminal_writestring("========================================\n\n");
    
    /* Status messages with colors */
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("[OK]   Terminal driver loaded\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[INFO] VGA buffer address: ");
    terminal_write_hex((uint32_t)VGA_MEMORY);
    terminal_writestring("\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("\nThis is a normal line.\n");
    terminal_writestring("Line with newline test:\n");
    terminal_writestring("  -> Second line\n");
    terminal_writestring("  -> Third line\n\n");
    
    /* Color palette demo */
    terminal_writestring("Color palette:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK));
    terminal_writestring("  Blue  ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Green  ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
    terminal_writestring("Red  ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("Magenta  ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("Brown\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("\n--- Scrolling test (lines 1-30) ---\n");
    
    /* Fill screen to test scrolling */
    for (int i = 1; i <= 30; i++) {
        terminal_write_dec(i);
        terminal_writestring(": This line tests automatic scrolling.\n");
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\n[OK] All tests passed. System ready.\n");
    
    while (1) {
        __asm__ __volatile__ ("hlt");
    }
}
