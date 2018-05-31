#ifndef FONTS_H
#define FONTS_H

#include "font.h"

extern const u8g_fntpgm_uint8_t     u8g_font_cronyx2t[];
extern const u8g_fntpgm_uint8_t     u8g_font_cronyx3c[];
extern const u8g_fntpgm_uint8_t     u8g_font_cronyx3h[];
extern const u8g_fntpgm_uint8_t     u8g_font_cronyx3hb[];
extern const u8g_fntpgm_uint8_t     u8g_font_cronyx3t[];
extern const u8g_fntpgm_uint8_t     u8g_font_cronyx3tb[];
extern const u8g_fntpgm_uint8_t     u8g_font_koi8x13[];
extern const u8g_fntpgm_uint8_t     u8g_font_koi9x15[];

typedef struct sFontItem {
    String name;
    const  u8g_fntpgm_uint8_t *id;
} FontItem;

#endif
