#ifndef MATRIX_7219_H
#define MATRIX_7219_H

#include "fonts.h"
#include "font.h"

class ledDisplay : public ledFont {
    public:
        ledDisplay(uint8_t cs_pin, uint8_t width, uint8_t height);
        bool        init(void);
        void        clean(void);
        void        show(void);
        uint8_t     intencity(uint8_t intencity);
        void        activate(bool switch_on);
        uint16_t    delayMS(uint16_t delay_ms);
        void        eolSize(uint16_t pixels);
        void        utf8text(const String& string);
        void        scrollLeft(void);
        void        drawPixel(uint16_t x, uint16_t y);

    private:
        void        writeRegister(uint8_t reg, uint8_t value);
        uint8_t     loadNextBuffer(void);
        String      utf8toWin1251(const String& str);
        uint8_t     cs;                             // The number of pin for CS signal. Use hardwarre SPI protocol
        uint8_t     h_m, v_m;                       // The horizontal number and vertical number of the matricies 8*8
        uint8_t     bright;                         // Brightness of the display
        String      message;                        // The text message to be displayed
        uint8_t     message_index;                  // The index of the symbol in the message to be displayed
        uint16_t    eol_space;                      // Space after end of line (pixels)
        uint16_t    show_delay;                     // The delay in ms afrer show()
        uint8_t     *screen;                        // Data buffer for the display (dynamicaly allocated)
        uint16_t    *next;                          // The next dual-octet array to be scrolled (dynamicaly allocated)
        uint8_t     next_pixels;                    // The number of pixel in the next buffer
        enum        max7219_register { MAX7219_REG_NOOP = 0, MAX7219_REG_DIGIT0 = 1, MAX7219_REG_DECODEMODE = 9,
                            MAX7219_REG_INTENSITY, MAX7219_REG_SCANLIMIT, MAX7219_REG_SHUTDOWN, MAX7219_REG_DISPLAYTEST = 0xF};
        const       uint16_t min_delay = 10;        // Minimum possible delay after the show()
};

#endif
