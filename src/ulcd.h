#include <stdio.h>

/** Clear display **/
#define LCD_CMD_CLEAR 0x01

/** Return home **/
#define LCD_CMD_HOME 0x02

/** Entry mode set **/
#define LCD_CMD_ENTRY_MODE 0x04
#define LCD_FLAG_DECREMENT 0x00
#define LCD_FLAG_INCREMENT 0x02
#define LCD_FLAG_SHIFT_DISPLAY 0x01

/** Display on/off control **/
#define LCD_CMD_DISPLAY_CTL 0x08
#define LCD_FLAG_DISPLAY_ON 0x04
#define LCD_FLAG_DISPLAY_OFF 0x00
#define LCD_FLAG_CURSOR_ON 0x02
#define LCD_FLAG_CURSOR_OFF 0x00
#define LCD_FLAG_BLINK_ON 0x01
#define LCD_FLAG_BLINK_OFF 0x00

/** Cursor or display shift **/
#define LCD_CMD_CURSOR_SHIFT 0x10
#define LCD_FLAG_DISPLAY_MOVE 0x08
#define LCD_FLAG_CURSOR_MOVE 0x00
#define LCD_FLAG_MOVE_RIGHT 0x04
#define LCD_FLAG_MOVE_LEFT 0x00

/** Function set **/
#define LCD_CMD_FUNCTION_SET 0x20
/** NB: This library only supports 4-bit mode **/
#define LCD_FLAG_4_BIT 0x00
#define LCD_FLAG_2_LINES 0x08
#define LCD_FLAG_1_LINE 0x00
#define LCD_FLAG_5X10_FONT 0x04
#define LCD_FLAG_5X8_FONT 0x00

/** Set CGRAM address **/
#define LCD_CMD_CGRAM_ADDR 0x40

/** Set DDRAM address **/
#define LCD_CMD_DDRAM_ADDR 0x80

#define LCD_DEFAULT_GPIO_DEV "/dev/gpiochip0"
#define LCD_DEFAULT_FIFO_PATH "/var/run/lcd0"

#define LCD_ERR -1
#define LCD_EXIT 11

/**
 * Configure GPIO character device.
 *
 * Open and configure a GPIO character device for use with
 * the LCD driver. If LCD_CFG_GPIO_DEV is not set, fall back
 * on LCD_DEFAULT_GPIO_DEV.
 * @return A valid file descriptor on success, LCD_ERR on failure.
 */
int lcd_init(void);

/**
 * Write a raw character from the 8-bit LCD character set.
 *
 * @param fd An open file descriptor returned by lcd_init.
 * @param chr Numeric value of the character to write. Must be within 1-byte
 * range.
 * @return 0 on success, LCD_ERR on failure.
 */
int lcd_write_raw_char(int fd, int chr);

/**
 * Write a raw stream without handling any special characters.
 *
 * @param fd An open file descriptor returned by lcd_init.
 * @param stream A stream of character data in the LCD's native encoding.
 * @return 0 on success, LCD_ERR on failure.
 */
int lcd_write_raw_stream(int fd, FILE* stream);

/**
 * Write a stream of utf-8 data, handling some control codes and escapes.
 *
 * Certain codepoints supported by the LCD character set are translated into
 * the native encoding. Unsupported codepoints and encoding errors are replaced
 * with '?'. Additionally, the following c0 codes and ANSI escapes are handled:
 * '\x03', '\x04', '\\b', '\t', '\\n', '\r', "\e[C", "\e[D", "\e[2J", "\e[?25h",
 * "\e[?25l", "\e[25m".
 *
 * @param fd An open file descriptor returned by lcd_init.
 * @param stream A stream of utf-8 encoded character data.
 * @return 0 on success, LCD_ERR on failure.
 */
int lcd_write_utf8_stream(int fd, FILE* stream);

/**
 * Execute an instruction on the LCD device.
 *
 * @param fd An open file descriptor returned by lcd_init.
 * @param cmd An 8-bit pattern to be written into the the LCD's data bus.
 * See the HD44780 data sheet for more info about the different instructions.
 * @return 0 on success, LCD_ERR on failure.
 */
int lcd_command(int fd, unsigned int cmd);

/**
 * Do some cleanup and close the GPIO fd.
 *
 * @param fd An open file descriptor to be closed.
 * @return 0 on success, non-zero on failure.
 */
int lcd_quit(int fd);
