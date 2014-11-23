SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

libmaddterm.a: $(OBJ)
	ar rvs $@ $(OBJ)
	ranlib $@

%.o: %.c src/libmaddterm.h
	gcc -c $< -o $@

