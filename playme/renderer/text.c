#include <stddef.h>

#include "internal.h"

static unsigned Rows(unsigned a, unsigned b, unsigned c, unsigned d, unsigned e) {
    return (a << 12) | (b << 9) | (c << 6) | (d << 3) | e;
}

static unsigned Glyph(char c) {
    if ('a' <= c && c <= 'z') {
        c = (char)(c - 'a' + 'A');
    }
    switch (c) {
        case 'A':
            return Rows(2, 5, 7, 5, 5);
        case 'B':
            return Rows(6, 5, 6, 5, 6);
        case 'C':
            return Rows(3, 4, 4, 4, 3);
        case 'D':
            return Rows(6, 5, 5, 5, 6);
        case 'E':
            return Rows(7, 4, 6, 4, 7);
        case 'F':
            return Rows(7, 4, 6, 4, 4);
        case 'G':
            return Rows(3, 4, 5, 5, 3);
        case 'H':
            return Rows(5, 5, 7, 5, 5);
        case 'I':
            return Rows(7, 2, 2, 2, 7);
        case 'J':
            return Rows(1, 1, 1, 5, 2);
        case 'K':
            return Rows(5, 5, 6, 5, 5);
        case 'L':
            return Rows(4, 4, 4, 4, 7);
        case 'M':
            return Rows(5, 7, 7, 5, 5);
        case 'N':
            return Rows(5, 7, 7, 7, 5);
        case 'O':
            return Rows(7, 5, 5, 5, 7);
        case 'P':
            return Rows(6, 5, 6, 4, 4);
        case 'Q':
            return Rows(7, 5, 5, 7, 1);
        case 'R':
            return Rows(6, 5, 6, 5, 5);
        case 'S':
            return Rows(3, 4, 7, 1, 6);
        case 'T':
            return Rows(7, 2, 2, 2, 2);
        case 'U':
            return Rows(5, 5, 5, 5, 7);
        case 'V':
            return Rows(5, 5, 5, 5, 2);
        case 'W':
            return Rows(5, 5, 7, 7, 5);
        case 'X':
            return Rows(5, 5, 2, 5, 5);
        case 'Y':
            return Rows(5, 5, 2, 2, 2);
        case 'Z':
            return Rows(7, 1, 2, 4, 7);
        case '0':
            return Rows(7, 5, 5, 5, 7);
        case '1':
            return Rows(2, 6, 2, 2, 7);
        case '2':
            return Rows(6, 1, 2, 4, 7);
        case '3':
            return Rows(6, 1, 2, 1, 6);
        case '4':
            return Rows(5, 5, 7, 1, 1);
        case '5':
            return Rows(7, 4, 6, 1, 6);
        case '6':
            return Rows(3, 4, 6, 5, 2);
        case '7':
            return Rows(7, 1, 2, 2, 2);
        case '8':
            return Rows(2, 5, 2, 5, 2);
        case '9':
            return Rows(2, 5, 3, 1, 6);
        case '.':
            return Rows(0, 0, 0, 0, 2);
        case ',':
            return Rows(0, 0, 0, 2, 4);
        case '!':
            return Rows(2, 2, 2, 0, 2);
        case '?':
            return Rows(6, 1, 2, 0, 2);
        case ':':
            return Rows(0, 2, 0, 2, 0);
        case ';':
            return Rows(0, 2, 0, 2, 4);
        case '-':
            return Rows(0, 0, 7, 0, 0);
        case '_':
            return Rows(0, 0, 0, 0, 7);
        case '+':
            return Rows(0, 2, 7, 2, 0);
        case '=':
            return Rows(0, 7, 0, 7, 0);
        case '/':
            return Rows(1, 1, 2, 4, 4);
        case '\\':
            return Rows(4, 4, 2, 1, 1);
        case '(':
            return Rows(1, 2, 2, 2, 1);
        case ')':
            return Rows(4, 2, 2, 2, 4);
        case '[':
            return Rows(3, 2, 2, 2, 3);
        case ']':
            return Rows(6, 2, 2, 2, 6);
        case '<':
            return Rows(1, 2, 4, 2, 1);
        case '>':
            return Rows(4, 2, 1, 2, 4);
        case ' ':
            return 0;
        default:
            return Rows(7, 5, 2, 0, 2);
    }
}

static void draw_character(unsigned char *pixels, const numeric_vector_t *size, const numeric_vector_t *pos, char c, int scale, int red, int green, int blue) {
    unsigned bits = Glyph(c);
    int x;
    int y;
    int scalex;
    int scaley;
    int pixelx;
    int pixely;
    int pixelpos;

    for (y = 0; y < 5; y++) {
        for (x = 0; x < 3; x++) {
            if (((bits >> ((4 - y) * 3 + 2 - x)) & 1u) == 0) {
                continue;
            }
            for (scaley = 0; scaley < scale; scaley++) {
                for (scalex = 0; scalex < scale; scalex++) {
                    pixelx = pos->x + x * scale + scalex;
                    pixely = pos->y + y * scale + scaley;
                    if (pixelx < 0 || pixely < 0 || size->x <= pixelx || size->y <= pixely) {
                        continue;
                    }
                    pixelpos = (pixely * size->x + pixelx) * 3;
                    pixels[pixelpos] = (unsigned char)red;
                    pixels[pixelpos + 1] = (unsigned char)green;
                    pixels[pixelpos + 2] = (unsigned char)blue;
                }
            }
        }
    }
}

void renderer_text_draw(unsigned char *pixels, const numeric_vector_t *size, const numeric_vector_t *pos, const char *content, int scale, int red, int green, int blue) {
    numeric_vector_t cursor;

    if (pixels == NULL || size == NULL || pos == NULL || content == NULL || scale <= 0) {
        return;
    }
    cursor = *pos;
    while (*content != '\0') {
        if (*content == '\n') {
            cursor.x = pos->x;
            cursor.y += 6 * scale;
        } else {
            draw_character(pixels, size, &cursor, *content, scale, red, green, blue);
            cursor.x += 4 * scale;
        }
        content++;
    }
}
