#ifndef HEADER_POTATOFONT
#define HEADER_POTATOFONT

extern unsigned char* currentFont;
extern unsigned char* fontRegular10;
extern unsigned char* fontBold10;

void potatofont_ReadyFonts();

extern unsigned char potatofont_FGColor;
extern unsigned char potatofont_BGColor;

void potatofont_DrawGlyph5x10(unsigned char glyph, char x, unsigned int y);

#endif