#include <stdio.h>

#define LCD_CMD_CLEAR 0x01
#define LCD_CMD_HOME 0x02
#define LCD_CMD_ENTRY_MODE 0x04
#define LCD_CMD_DISPLAY_CTL 0x08
#define LCD_CMD_CURSOR_SHIFT 0x10
#define LCD_CMD_FUNCTION_SET 0x20
#define LCD_CMD_CGRAM_ADDR 0x40
#define LCD_CMD_DDRAM_ADDR 0x80

#define LCD_FLAG_SHIFT_DISPLAY 0x01
#define LCD_FLAG_DECREMENT 0x00
#define LCD_FLAG_INCREMENT 0x02
#define LCD_FLAG_BLINK_ON 0x01
#define LCD_FLAG_BLINK_OFF 0x00
#define LCD_FLAG_CURSOR_ON 0x02
#define LCD_FLAG_CURSOR_OFF 0x00
#define LCD_FLAG_DISPLAY_ON 0x04
#define LCD_FLAG_DISPLAY_OFF 0x00
#define LCD_FLAG_MOVE_RIGHT 0x04
#define LCD_FLAG_MOVE_LEFT 0x00
#define LCD_FLAG_DISPLAY_MOVE 0x08
#define LCD_FLAG_CURSOR_MOVE 0x00

#define LCD_DEFAULT_GPIO_DEV "/dev/gpiochip0"
#define LCD_DEFAULT_FIFO_PATH "/var/run/lcd0"

#define LCD_ERR -1
#define LCD_EXIT 11

int lcd_init(void);
int lcd_write_raw_char(int fd, int chr);
int lcd_write_raw_stream(int fd, FILE* stream);
int lcd_write_utf8_stream(int fd, FILE* stream);
int lcd_command(int fd, unsigned int cmd);
int lcd_quit(int fd);
