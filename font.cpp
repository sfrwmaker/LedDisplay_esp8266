#include "font.h"

ledFont::ledFont() {
    font = 0;
    for (uint8_t i = 0; i < 13; ++i)
        glyph_pos[i] = 0;
}

void ledFont::setFont(const u8g_fntpgm_uint8_t *fnt) {
	if (fnt && (fnt != font)) {
		font = fnt;
		initFontHeader();
        initGlyphPos();
	}
}

uint8_t ledFont::loadGlyph(uint16_t* area, uint8_t area_height, bool& finished, uint8_t encode, const uint8_t column) {
    // First, clean up the area
    for (uint8_t i = 0; i < area_height; area[i++] = 0);
	if (encode < FH.encoding_start || encode > FH.encoding_end) {
		return FH.width;								// Not found
	}
	uint8_t es = FH.encoding_start;
	
	uint16_t glyph_offset = min_glyph_offset;
    if (encode > 32) {                                  // Minimum code that is in the glyph_pos array
        uint8_t index = encode - 32;
        index >>= 4;                                    // Divide by 16
        if (glyph_pos[index] > min_glyph_offset) {
            glyph_offset = glyph_pos[index];
            es = (index << 4) + 32;
        }
    }
	
	for (uint8_t enc = es; enc <= FH.encoding_end; ++enc) {
		if (fontGetByte(glyph_offset) == 255) {			// Skip empty glyph
			++glyph_offset;
            if (enc == encode) {
                break;
            } else {
			    continue;
            }
		}
		if (enc == encode) {							// The requested symbol has been found
            loadGlyphHeader(glyph_offset);
			if (FH.format != 1) {
                glyph_offset += 6;
            } else {
                glyph_offset += 3;
            }
			loadGlyphData(area, area_height, finished, glyph_offset, column);
            uint8_t loaded_data_width = 8;
            if (finished)
                loaded_data_width = GH.DWIDTH - column*8;
            return loaded_data_width;
		} else {										// Skip current glyph
            uint8_t glyph_data_size = fontGetByte(glyph_offset+2);
            if (FH.format != 1) {
			    glyph_offset += glyph_data_size + 6;
            } else {
				glyph_offset += (glyph_data_size & 0xF) + 3;
            }
		}
	}

    // Empty symbol found or no symbol found. Draw a space
    for (uint8_t i = 0; i < area_height; ++i)
        area[i] = 0;
    finished = true;
    return FH.width;
}

uint8_t ledFont::fontGetByte(uint16_t offset) {
  const uint8_t *addr = font + offset;
  return pgm_read_byte_near(addr);  
}


uint16_t ledFont::fontGetWord(uint16_t offset) {
    uint16_t data;
	const uint8_t *addr = font + offset;
    data  = pgm_read_byte_near(addr);
    addr++;
    data <<= 8;
    data |= pgm_read_byte_near(addr);
    return data;
}

void ledFont::initFontHeader(void) {
	uint8_t offset = 0;
	if (font) {
		FH.format			= fontGetByte(offset);
		FH.width			= fontGetByte(++offset);
		FH.height			= fontGetByte(++offset);
		FH.x_offset			= fontGetByte(++offset);
		FH.y_offset			= fontGetByte(++offset);
		FH.A_height			= fontGetByte(++offset);
        offset              = 10;
		FH.encoding_start	= fontGetByte(offset);
		FH.encoding_end		= fontGetByte(++offset);
		FH.descent_g		= fontGetByte(++offset);
		FH.max_ascent		= fontGetByte(++offset);
		FH.min_descent		= fontGetByte(++offset);
		FH.xascent			= fontGetByte(++offset);
		FH.xdescent			= fontGetByte(++offset);
	}
}

void ledFont::initGlyphPos(void) {
    uint16_t glyph_offset = min_glyph_offset;
    uint8_t  enc = FH.encoding_start;

    for (uint8_t i = 0; i < 13; ++i) {
        uint8_t glyph_encode = 32 + i * 16;
        if (glyph_encode < FH.encoding_start) {
            glyph_pos[i] = min_glyph_offset;
            continue;
        }

        for (; enc <= FH.encoding_end; ++enc) {
            if (fontGetByte(glyph_offset) == 255) {     // Skip empty glyph
                if (enc == glyph_encode) {
                    break;
                } else {
                    ++glyph_offset;
                    continue;
                }
            }
            if (enc == glyph_encode) {                  // The requested symbol has been found
                break;
            } else {                                    // Skip current glyph
                uint8_t glyph_data_size = fontGetByte(glyph_offset+2);
                if (FH.format != 1) {
                    glyph_offset += glyph_data_size + 6;
                } else {
                    glyph_offset += (glyph_data_size & 0xF) + 3;
                }
            }
        }
        glyph_pos[i] = glyph_offset;
    }
}

void ledFont::loadGlyphHeader(uint16_t glyph_offset) {
	uint16_t offset = glyph_offset;
	switch (FH.format) {
		case 0:
		case 2:
			GH.BBX_width	= fontGetByte(offset);
			GH.BBX_height	= fontGetByte(++offset);
			GH.data_size	= fontGetByte(++offset);// (BBX width + 7)/8 * BBX height
			GH.DWIDTH		= fontGetByte(++offset);
			GH.BBX_xoffset	= fontGetByte(++offset);
			GH.BBX_yoffset	= fontGetByte(++offset);
			break;
		case 1:
		default:
			/*
			format 1
			0             BBX xoffset				signed   --> upper 4 Bit
			0             BBX yoffset				signed --> lower 4 Bit
			1             BBX width					unsigned --> upper 4 Bit
			1             BBX height				unsigned --> lower 4 Bit
			2             data size					unsigned -(BBX width + 7)/8 * BBX height  --> lower 4 Bit
			2             DWIDTH					signed --> upper  4 Bit
			byte 0 == 255 indicates empty glyph
			*/
			int8_t tmpi		= fontGetByte(offset);
			GH.BBX_yoffset = tmpi & 0xF;
			GH.BBX_yoffset	-=2;
			GH.BBX_xoffset = tmpi >> 4;
			uint8_t tmpu	= fontGetByte(++offset);
			GH.BBX_height	= tmpu & 0xF;
			GH.BBX_width	= tmpu >> 4;
			tmpu			= fontGetByte(++offset);
			GH.data_size	= tmpu & 0xF;
			GH.DWIDTH		= tmpu >> 4;
			break;
	}
}

void ledFont::loadGlyphData(uint16_t* area, uint8_t area_height, bool &finished, uint16_t glyph_offset, const uint8_t column) {
    if (GH.data_size == 0 || GH.BBX_height == 0) {
        finished = true;                            // The glyph is loaded completely
        return;
    }
	uint8_t gl_columns = GH.data_size / GH.BBX_height;

    if (column > gl_columns) {
        finished = true;                            // The glyph is loaded completely
        return;
    }
    uint8_t glyph_height = GH.BBX_height;
    if (area_height < GH.BBX_height)                // Prevent the glyph overdraw
        glyph_height = area_height;

    uint8_t skip = 0;
	if (area_height >= FH.A_height) {               // If display is higher than 'A' letter
		skip = (area_height - FH.A_height) / 2;
	}
    uint8_t base = skip + FH.A_height;              // The base line position

    int glyph_top = base - GH.BBX_yoffset - GH.BBX_height;
    if (glyph_top + glyph_height > area_height)     // Prevent the glyph overdraw
        glyph_height = area_height - glyph_top;
        
    // Draw the glyph
	uint16_t offset = glyph_offset;
	for (uint8_t i = 0; i < glyph_height; ++i) {
        uint16_t data = 0;
        if (gl_columns > 1) {
            if (GH.BBX_xoffset >= 0) {
                if (column == 0) {
                    data = fontGetByte(offset+column);
                    data <<= 8 - GH.BBX_xoffset;
                } else {
                    data = fontGetByte(offset+column-1);
                    data <<= 8;
                    data |= fontGetByte(offset+column);
                    data <<= 8 - GH.BBX_xoffset;
                }
            }
        } else {
            data = fontGetByte(offset);
            data <<= 8 - GH.BBX_xoffset;
        }
        area[i+glyph_top] = data;
		offset += gl_columns;
	}
    finished = (column >= gl_columns-1);
}
