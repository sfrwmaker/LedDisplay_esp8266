#ifndef FONT_H
#define FONT_H

#include <Arduino.h>

#define U8G_FONT_SECTION(name) PROGMEM
typedef uint8_t         u8g_fntpgm_uint8_t;

/*
  We use the fonts from u8glib library
  the fields of the font data structure are accessed directly by offset font information 
  offset
  0             font format
  1             FONTBOUNDINGBOX width           	unsigned
  2             FONTBOUNDINGBOX height          	unsigned
  3             FONTBOUNDINGBOX x-offset        	signed
  4             FONTBOUNDINGBOX y-offset        	signed
  5             capital A height                	unsigned
  6             start 'A'							Is not included in the FontHeader structure
  8             start 'a'							Is not included in the FontHeader structure
  10            encoding start						Is included in the EncodingCache class
  11            encoding end						Is included in the EncodingCache class
  12            descent 'g'                     	negative: below baseline
  13            font max ascent
  14            font min decent             		negative: below baseline 
  15            font xascent
  16            font xdecent             			negative: below baseline 
  
*/

typedef struct font_header_s {
    uint8_t		format;								// glyph format can be 0 or 1
	uint8_t		width;
	uint8_t     height;
	int8_t		x_offset;
	int8_t		y_offset;
	uint8_t		A_height;
	uint8_t		encoding_start;
	uint8_t		encoding_end;
	int8_t		descent_g;
	int8_t		max_ascent;
    int8_t      min_descent;
    int8_t      xascent;
    int8_t      xdescent;
} FontHeader;

typedef struct glyph_header_s {
	uint8_t		BBX_width;
    uint8_t		BBX_height;
    uint8_t     data_size;
    int8_t		DWIDTH;
    int8_t      BBX_xoffset;
    int8_t 		BBX_yoffset;
} GlyphHeader;

class ledFont {
	public:
		ledFont();
        bool        isFontLoaded(void)              { return font != 0; }
		void		setFont(const u8g_fntpgm_uint8_t *fnt);		
		uint8_t		loadGlyph(uint16_t* area, uint8_t area_height, bool& finished, const uint8_t encode, const uint8_t column);

    private:
		uint8_t 	fontGetByte(uint16_t offset);
		uint16_t 	fontGetWord(uint16_t offset);
		void		initFontHeader(void);
        void        initGlyphPos(void);
		void		loadGlyphHeader(uint16_t glyph_offset);
		void		loadGlyphData(uint16_t* area, uint8_t area_height, bool& finished, uint16_t glyph_offset, const uint8_t column);
		void		insertEncoding(uint8_t encoding, uint16_t offset);
		const u8g_fntpgm_uint8_t    *font;
        uint16_t    glyph_pos[14];                  // glyph offset for 32, 48, 32+16*i element, i = 0..13
		FontHeader				    FH;
		GlyphHeader				    GH;
		const 		uint16_t    	min_glyph_offset = 17;
};

#endif
