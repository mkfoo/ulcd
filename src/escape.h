#include <stdio.h>

enum ESCAPE {
    ESC_CURSOR_FORWARD,
    ESC_CURSOR_BACKWARD,
    ESC_ERASE_DISPLAY,
    ESC_SHOW_CURSOR,
    ESC_HIDE_CURSOR,
    ESC_BLINK_OFF,
    ESC_UNKNOWN,
};

static int _read_escape(FILE* stream);
static int _r0(FILE* stream);
static int _r1(FILE* stream);
static int _r2(FILE* stream);
static int _r3(FILE* stream);
static int _skip(FILE* stream);

static int _read_escape(FILE* stream) {
    if (fgetc(stream) == '[') {
        return _r0(stream);
    }

    return ESC_UNKNOWN;
}

static int _r0(FILE* stream) {
    switch (fgetc(stream)) {
        case 'C':
            return ESC_CURSOR_FORWARD;
        case 'D':
            return ESC_CURSOR_BACKWARD;
        case '0':
        case '1':
            return _r1(stream);
        case '2':
            return _r2(stream);
        case '?':
            return _r3(stream);
        default:
            return _skip(stream);
    }
}

static int _r1(FILE* stream) {
    switch (fgetc(stream)) {
        case 'C':
            return ESC_CURSOR_FORWARD;
        case 'D':
            return ESC_CURSOR_BACKWARD;
        default:
            return _skip(stream);
    }
}

static int _r2(FILE* stream) {
    switch (fgetc(stream)) {
        case 'J':
            return ESC_ERASE_DISPLAY;
        case '5':
            if (fgetc(stream) == 'm') {
                return ESC_BLINK_OFF;
            }
        default:
            return _skip(stream);
    }
}

static int _r3(FILE* stream) {
    if (fgetc(stream) != '2') {
        return _skip(stream);
    }

    if (fgetc(stream) != '5') {
        return _skip(stream);
    }

    switch (fgetc(stream)) {
        case 'h':
            return ESC_SHOW_CURSOR;
        case 'l':
            return ESC_HIDE_CURSOR;
        default:
            return _skip(stream);
    }
}

static int _skip(FILE* stream) {
    int chr = fgetc(stream);

    while (chr != EOF && chr > '\x1f' && chr < '@') {
        chr = fgetc(stream);
    }

    return ESC_UNKNOWN;
}
