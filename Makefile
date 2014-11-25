SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

.PHONY: install clean distclean

libmaddterm.a: $(OBJ)
	ar rvs $@ $(OBJ)
	ranlib $@

%.o: %.c src/libmaddterm.h
	gcc -c $< -o $@

install: libmaddterm.a
	cp libmaddterm.a /usr/lib/
	cp src/libmaddterm.h /usr/include/

clean:
	rm src/*.o

distclean:
	rm src/*.o libmaddterm.a
