#include <stdio.h>
#define NCHARS 95
#define RPCHAR 63

const int CHARSET[NCHARS] = {
    12290, 12300, 12301, 12289, 12539, 12530, 12449, 12451, 12453, 12455, 12457,
    12515, 12517, 12519, 12483, 12540, 12450, 12452, 12454, 12456, 12458, 12459,
    12461, 12463, 12465, 12467, 12469, 12471, 12473, 12475, 12477, 12479, 12481,
    12484, 12486, 12488, 12490, 12491, 12492, 12493, 12494, 12495, 12498, 12501,
    12504, 12507, 12510, 12511, 12512, 12513, 12514, 12516, 12518, 12520, 12521,
    12522, 12523, 12524, 12525, 12527, 12531, 12443, 12444, 945,   228,   946,
    949,   956,   963,   961,   0,     8730,  0,     0,     739,   162,   163,
    241,   246,   0,     0,     952,   8734,  937,   252,   931,   960,   0,
    0,     21315, 19975, 20870, 247,   0,     9608,
};

static int _lookup_codepoint(int cp);
static int _read_codepoint(FILE *stream);
static int _read_b2(FILE *stream, int b1);
static int _read_b3(FILE *stream, int b1, int b2);
static int _read_b4(FILE *stream, int b1, int b2, int b3);

static int _lookup_codepoint(int cp) {
    int offset = 256 - NCHARS;

    for (int i = NCHARS - 1; i >= 0; i--) {
        if (CHARSET[i] == cp) {
            return i + offset;
        }
    }

    return RPCHAR;
}

static int _read_codepoint(FILE *stream) {
    int b1 = fgetc(stream);

    if (b1 == EOF) {
        return EOF;
    }

    if (b1 < 128) {
        return b1;
    }

    if (b1 >= 192) {
        return _read_b2(stream, b1);
    }

    return RPCHAR;
}

static int _read_b2(FILE *stream, int b1) {
    int b2 = fgetc(stream);

    if (b2 == EOF) {
        return EOF;
    }

    if ((b2 & 192) != 128) {
        return RPCHAR;
    }

    if (!(b1 & 32)) {
        return (b1 & 31) << 6 | (b2 & 63);
    }

    return _read_b3(stream, b1, b2);
}

static int _read_b3(FILE *stream, int b1, int b2) {
    int b3 = fgetc(stream);

    if (b3 == EOF) {
        return EOF;
    }

    if ((b3 & 192) != 128) {
        return RPCHAR;
    }

    if (!(b1 & 16)) {
        return (b1 & 15) << 12 | (b2 & 63) << 6 | (b3 & 63);
    }

    return _read_b4(stream, b1, b2, b3);
}

static int _read_b4(FILE *stream, int b1, int b2, int b3) {
    int b4 = fgetc(stream);

    if (b4 == EOF) {
        return EOF;
    }

    if ((b4 & 192) != 128) {
        return RPCHAR;
    }

    if (!(b1 & 8)) {
        return (b1 & 7) << 18 | (b2 & 63) << 12 | (b3 & 63) << 6 | (b4 & 63);
    }

    return RPCHAR;
}
