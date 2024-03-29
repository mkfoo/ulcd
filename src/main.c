#include "ulcd.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
    int fd = lcd_init();

    if (fd == LCD_ERR) {
        return EXIT_FAILURE;
    }

    char* fifo_path = getenv("LCD_CFG_FIFO_PATH");

    if (fifo_path == NULL) {
        fifo_path = LCD_DEFAULT_FIFO_PATH;
    }

    if (mkfifo(fifo_path, 0775) == -1) {
        fprintf(stderr, "Failed to create FIFO: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    int err = 0;
    FILE* stream;

    while (!err) {
        stream = fopen(fifo_path, "r");

        if (stream == NULL) {
            fprintf(stderr, "Failed to open FIFO: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        err = lcd_write_utf8_stream(fd, stream);
        fclose(stream);
    }

    if (unlink(fifo_path)) {
        fprintf(stderr, "Failed to remove FIFO: %s\n", strerror(errno));
    }

    err |= lcd_quit(fd);

    if (err == LCD_EXIT) {
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
