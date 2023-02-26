#include "ulcd.h"
#include "charset.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define LCD_MASK_ALL 0x3f
#define LCD_MASK_E 0x20
#define LCD_MASK_RS 0x10
#define LCD_MASK_DATA 0xf
#define LCD_MODE_CMD 0x0
#define LCD_MODE_DATA 0x10
#define LCD_MARK 1000
#define LCD_SPACE 100000
#define LCD_LINE2_ADDR 0x40

static int _write_raw(int fd, unsigned int value, unsigned int mask);
static void _sleep(long nsec);
static int _enable(int fd);
static int _write4(int fd, unsigned int val);
static int _write8(int fd, unsigned int mode, unsigned int val);
static int _hw_init(int fd);
static unsigned int _get_pin_cfg(unsigned int lines, char* var_name);
static int _backspace(int fd);
static int _handle_c0(int fd, FILE* stream, int cp);
static int _handle_escape(int fd, FILE* stream);

static int _write_raw(int fd, unsigned int value, unsigned int mask) {
    struct gpio_v2_line_values data = {
        .bits = value,
        .mask = mask,
    };

    int ret = ioctl(fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &data);

    if (ret == -1) {
        fprintf(stderr, "Failed to set GPIO values: %s\n", strerror(errno));
        return LCD_ERR;
    }

    return 0;
}

static void _sleep(long nsec) {
    struct timespec tsp = {
        .tv_sec = 0,
        .tv_nsec = nsec,
    };
    int ret = 1;

    while (ret) {
        ret = nanosleep(&tsp, &tsp);
    }
}

static int _enable(int fd) {
    int ret = _write_raw(fd, 0, LCD_MASK_E);
    _sleep(LCD_MARK);
    ret |= _write_raw(fd, LCD_MASK_E, LCD_MASK_E);
    _sleep(LCD_MARK);
    ret |= _write_raw(fd, 0, LCD_MASK_E);
    _sleep(LCD_SPACE);
    return ret;
}

static int _write4(int fd, unsigned int val) {
    int ret = _write_raw(fd, val, LCD_MASK_DATA);
    ret |= _enable(fd);
    return ret;
}

static int _write8(int fd, unsigned int mode, unsigned int val) {
    int ret = _write_raw(fd, mode, LCD_MASK_RS);
    ret |= _write4(fd, val >> 4);
    ret |= _write4(fd, val);
    return ret;
}

static int _hw_init(int fd) {
    int ret = _write_raw(fd, 0x0, LCD_MASK_ALL);
    _sleep(LCD_SPACE);
    ret |= _write4(fd, 0x3);
    _sleep(LCD_SPACE * 45);
    ret |= _write4(fd, 0x3);
    _sleep(LCD_SPACE * 45);
    ret |= _write4(fd, 0x3);
    _sleep(LCD_SPACE);
    ret |= _write4(fd, 0x2);
    _sleep(LCD_SPACE * 20);
    ret |= lcd_command(fd, LCD_CMD_CLEAR);
    return ret;
}

static unsigned int _get_pin_cfg(unsigned int lines, char* var) {
    char* str = getenv(var);

    if (str == NULL) {
        fprintf(stderr, "Config variable %s not set\n", var);
        return 0;
    }

    int sval = atoi(str);

    if (sval < 1) {
        fprintf(stderr, "Invalid GPIO number %d\n", sval);
        return 0;
    }

    unsigned int val = (unsigned int)sval;

    if (val >= lines) {
        fprintf(stderr, "GPIO number %d out of range\n", val);
        return 0;
    }

    return val;
}

static int _backspace(int fd) {
    int ret = lcd_command(fd, LCD_CMD_CURSOR_SHIFT);
    ret |= lcd_write_raw_char(fd, ' ');
    ret |= lcd_command(fd, LCD_CMD_CURSOR_SHIFT);
    return ret;
}

static int _handle_c0(int fd, FILE* stream, int cp) {
    switch (cp) {
        case '\x03':
        case '\x04':
            return LCD_EXIT;
        case '\b':
            return _backspace(fd);
        case '\t':
            return lcd_write_raw_char(fd, ' ');
        case '\n':
            return lcd_command(fd, LCD_CMD_DDRAM_ADDR | LCD_LINE2_ADDR);
        case '\r':
            return lcd_command(fd, LCD_CMD_HOME);
        case '\x1b':
            return _handle_escape(fd, stream);
        default:
            return 0;
    }
}

static int _handle_escape(int fd, FILE* stream) {
    int chr = fgetc(stream);

    if (chr != '[') {
        return 0;
    }

    switch (chr) {
        case '1':
            chr = fgetc(stream);

            if (chr == 'C') {
                return lcd_command(
                    fd, LCD_CMD_CURSOR_SHIFT | LCD_FLAG_MOVE_RIGHT);
            }

            if (chr == 'D') {
                return lcd_command(fd, LCD_CMD_CURSOR_SHIFT);
            }

            return 0;
        case '2':
            chr = fgetc(stream);

            if (chr == 'J') {
                return lcd_command(fd, LCD_CMD_CLEAR);
            }

            return 0;
        case '?':
        default:
            return 0;
    }
}

int lcd_init(void) {
    char* dev_path = getenv("LCD_CFG_GPIO_DEV");

    if (dev_path == NULL) {
        dev_path = LCD_DEFAULT_GPIO_DEV;
    }

    int fd = open(dev_path, O_RDONLY);

    if (fd < 0) {
        fprintf(stderr, "Could not open GPIO chip: %s\n", strerror(errno));
        return LCD_ERR;
    }

    struct gpiochip_info chip_info;
    int ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &chip_info);

    if (ret == -1) {
        fprintf(stderr, "Could not get GPIO chip info: %s\n", strerror(errno));
        return LCD_ERR;
    }

    printf("Opened %s\n", chip_info.name);

    unsigned int lines = chip_info.lines;
    unsigned int d4 = _get_pin_cfg(lines, "LCD_CFG_D4");
    unsigned int d5 = _get_pin_cfg(lines, "LCD_CFG_D5");
    unsigned int d6 = _get_pin_cfg(lines, "LCD_CFG_D6");
    unsigned int d7 = _get_pin_cfg(lines, "LCD_CFG_D7");
    unsigned int rs = _get_pin_cfg(lines, "LCD_CFG_RS");
    unsigned int e = _get_pin_cfg(lines, "LCD_CFG_E");

    if (!(d4 && d5 && d6 && d7 && rs && e)) {
        return LCD_ERR;
    }

    printf(
        "PIN: GPIO\n D4:   %d\n D5:   %d\n "
        "D6:   %d\n D7:   %d\n RS:   %d\n  E:   %d\n",
        d4, d5, d6, d7, rs, e);

    struct gpio_v2_line_request req = {
        .offsets = {d4, d5, d6, d7, rs, e},
        .consumer = "uLCD",
        .config =
            (struct gpio_v2_line_config){
                .flags = GPIO_V2_LINE_FLAG_OUTPUT,
                .num_attrs = 0,
            },
        .num_lines = 6,
    };

    ret = ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &req);

    if (ret == -1 || req.fd <= 0) {
        fprintf(stderr, "GPIO line request failed: %s\n", strerror(errno));
        return LCD_ERR;
    }

    ret = _hw_init(req.fd);

    if (ret) {
        return LCD_ERR;
    }

    close(fd);
    return req.fd;
}

int lcd_write_raw_char(int fd, int chr) {
    if (chr < 0 || chr > 255) {
        fprintf(stderr, "Value out of range\n");
        return LCD_ERR;
    }

    return _write8(fd, LCD_MODE_DATA, (unsigned int)chr);
}

int lcd_write_raw_stream(int fd, FILE* stream) {
    if (stream == NULL) {
        return LCD_ERR;
    }

    int chr = fgetc(stream);
    int err = 0;

    while (chr != EOF && !err) {
        err = lcd_write_raw_char(fd, chr);
        chr = fgetc(stream);
    }

    return err;
}

int lcd_write_utf8_stream(int fd, FILE* stream) {
    if (stream == NULL) {
        return LCD_ERR;
    }

    int cp = _read_codepoint(stream);
    int chr = 0;
    int err = 0;

    while (cp != EOF && !err) {
        if (cp < 32) {
            err = _handle_c0(fd, stream, cp);
        } else if (cp < 128) {
            err = lcd_write_raw_char(fd, cp);
        } else {
            chr = _lookup_codepoint(cp);
            err = lcd_write_raw_char(fd, chr);
        }
        cp = _read_codepoint(stream);
    }

    return err;
}

int lcd_command(int fd, unsigned int cmd) {
    int ret = _write8(fd, LCD_MODE_CMD, cmd);
    _sleep(LCD_SPACE * 20);
    return ret;
}

int lcd_quit(int fd) {
    int ret = _write_raw(fd, 0x0, LCD_MASK_ALL);
    close(fd);
    return ret;
}
