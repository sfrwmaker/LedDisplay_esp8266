#include "matrix7219.h"
#include <SPI.h>

ledDisplay::ledDisplay(uint8_t cs_pin, uint8_t width, uint8_t height) {
    h_m             = width  >> 3;                      // Divided by 8
    v_m             = height >> 3;
    cs              = cs_pin;
    screen          = 0;
    next            = 0;
    next_pixels     = 0;
    bright          = 7;
    message         = String("");
    message_index   = 0;
    eol_space       = 8;
    show_delay      = 40;
}

bool ledDisplay::init(void) {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    delay(100);

    screen = (uint8_t *)malloc(uint16_t(h_m) * uint16_t(v_m) * 8);
    next   = (uint16_t *)malloc(uint16_t(v_m) * 8 * 2);
    if (screen && next) {
        writeRegister(MAX7219_REG_SCANLIMIT,    7);     // show all 8 digits
        writeRegister(MAX7219_REG_DECODEMODE,   0);     // using an led matrix (not digits)
        writeRegister(MAX7219_REG_DISPLAYTEST,  0);     // no display test
        writeRegister(MAX7219_REG_INTENSITY,    bright);// character intensity: range: 0 to 15
        writeRegister(MAX7219_REG_SHUTDOWN,     0);     // in shutdown mode
        clean();
        delay(100);
        activate(true);
        show();
        return true;
    }
    return false;
}

void ledDisplay::clean(void) {
    for (uint16_t i = 0; i <  uint16_t(h_m) * uint16_t(v_m) * 8; ++i)
        screen[i] = 0;
    for (uint8_t i = 0; i < v_m*8; ++i)
        next[i] = 0;
}

void ledDisplay::activate(bool switch_on) {
    uint8_t state = 0;
    if (switch_on) state = 1;
    writeRegister(MAX7219_REG_SHUTDOWN, state);
    delay(10);
}

uint16_t ledDisplay::delayMS(uint16_t delay_ms) {
    if (delay_ms > min_delay)
        show_delay = delay_ms;
    return show_delay;
}

void ledDisplay::eolSize(uint16_t pixels) {
    uint16_t max_space = h_m; max_space <<= 3;
    if (pixels > max_space) pixels = max_space;
    eol_space = pixels;
}

void ledDisplay::show(void) {
    for (uint8_t row = 0; row < 8; ++row) {
        digitalWrite(cs, LOW);
        for (uint8_t i = 0; i < v_m; ++i) {
            uint16_t m = i*h_m;
            m <<= 3;                                    // Multiply by 8
            for (uint8_t j = 0; j < h_m; ++j) {
                SPI.transfer(row+MAX7219_REG_DIGIT0);
                SPI.transfer(screen[row*h_m + m + j]);
            }
        }
        digitalWrite(cs, HIGH);
        delayMicroseconds(10);
    }
    delay(show_delay);
}

uint8_t ledDisplay::intencity(uint8_t intencity) {
    if (intencity < 16) {
        bright = intencity;
        writeRegister(MAX7219_REG_INTENSITY, bright);
    }
    return bright;
}

void ledDisplay::utf8text(const String& string) {
    clean();
    message = utf8toWin1251(string);
    message_index = 0;
}

void ledDisplay::scrollLeft(void) {
    if (next_pixels == 0) {
        next_pixels = loadNextBuffer();
    }
    for (uint16_t row = 0; row < v_m*8; ++row) {
        for (uint16_t col = 0; col < h_m-1; ++col) {
            uint16_t address = col+row*h_m;
            screen[address] <<= 1;
            uint8_t  dot = screen[address+1] & 0x80;    // The left bit of the next octet
            if (dot) screen[address] |= 1;
        }
        uint16_t last = (row+1)*h_m - 1;
        uint16_t dot = next[row] & 0x8000;              // next is a 16-bit width
        screen[last] <<= 1;
        if (dot) screen[last] |= 1;
        next[row] <<= 1;
    }
    --next_pixels;
}

void ledDisplay::drawPixel(uint16_t x, uint16_t y) {
    if ((x >= uint16_t(h_m)*8) || (y >= uint16_t(v_m)*8)) return;
    uint8_t h_matrix  = x >> 3;                           // Divided by 8
    uint8_t pixel     = x % 8;
    uint8_t mask      = 0b10000000 >> pixel;
    screen[h_matrix + y*h_m] = mask;
}

void ledDisplay::writeRegister(volatile uint8_t reg, volatile uint8_t value) {
    digitalWrite(cs, LOW);
    for (uint16_t i = 0; i < uint16_t(h_m) * uint16_t(v_m); ++i) {
        SPI.transfer(reg);
        SPI.transfer(value);
    }
    digitalWrite(cs, HIGH);
    delay(50);
}

uint8_t ledDisplay::loadNextBuffer(void) {
    static uint8_t column = 0;
    if (!isFontLoaded()) 
        return 8;

    uint8_t pixel_loaded = eol_space;
    if (message_index == message.length()) {            // End of the message
        for (uint8_t i = 0; i < v_m*8; ++i)             // Write a space after the message
            next[i] = 0;
        message_index = 0;
        column = 0;
    } else {
        bool load_complete = true;                      // Wether the glyph loaded completely. This variable will be modified by the loadGlyph()
        pixel_loaded = loadGlyph(next, v_m*8, load_complete, message.charAt(message_index), column);
        if (!load_complete) {
            ++column;
        } else {
            column = 0;
            ++message_index;
        }
    }
    return pixel_loaded;
}

String ledDisplay::utf8toWin1251(const String& str)  {
    String result = "";
    uint8_t k = str.length();
    uint8_t i = 0;
   
    while (i < k) {
        uint8_t n = str.charAt(i++);
   
        if (n >= 0xC0) {
            switch (n) {
                case 0xD0:
                    n = str.charAt(i++);
                    if (n == 0x81) { n = 0xA8; break; }
                    if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
                    break;
                 case 0xD1:
                    n = str.charAt(i++);
                    if (n == 0x91) { n = 0xB8; break; }
                    if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
                    break;
            }
        }
        result += String(char(n));
    }
    return result;
}

