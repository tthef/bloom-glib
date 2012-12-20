all: bloom

bloom: bloom-filter.c bloom-filter.h main.c
	$(CC) -o $@ -Wall -Werror bloom-filter.c main.c $(shell pkg-config --cflags --libs gobject-2.0)

clean:
	rm -f bloom
