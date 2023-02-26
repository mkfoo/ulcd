PROGRAM = ulcd
VPATH = src
CFLAGS = -Werror -Wall -Wextra -Wpedantic -std=c17 -D_POSIX_C_SOURCE=199309L
OFLAGS = -O1
OBJECTS = main.o ulcd.o 
HEADERS = ulcd.h charset.h escape.h

$(PROGRAM) : $(OBJECTS) 
	$(CC) $(CFLAGS) $(OFLAGS) $(OBJECTS) -o $(PROGRAM)

$(OBJECTS) : %.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(OFLAGS) $< -o $@

rpi1 : CC = zig cc --target=arm-linux-musleabihf -mcpu=arm1176jzf_s 
rpi1 : $(PROGRAM)

rpi3 : CC = zig cc --target=aarch64-linux-musl -mcpu=cortex_a53 
rpi3 : $(PROGRAM)

format :
	clang-format -i src/*

clean :
	rm -f $(PROGRAM) *.o

.PHONY : rpi1 rpi3 format clean
