CFLAGS = -Wall -Werror
LDFLAGS = -ltrix -lm -L/usr/local/lib -Wl,-R/usr/local/lib

.PHONY: install

rog: rog.c stb_image.o
	gcc $(CFLAGS) rog.c stb_image.o -o rog $(LDFLAGS)

stb_image.o: stb_image.c stb_image.h
	gcc -c stb_image.c -o stb_image.o

install: rog
	cp rog /usr/local/bin/rog
