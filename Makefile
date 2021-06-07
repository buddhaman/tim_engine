CC=gcc

SRC_CHIPMUNK = $(wildcard src_chipmunk/*.c)
SRC = $(wildcard src/*)

DEBUG0 = -g
CFLAGS = -DNDEBUG
ODIR = obj
_OBJ = $(SRC_CHIPMUNK:.c=.o)
OBJ = $(patsubst %,$(ODIR)/%,$(notdir $(_OBJ)))

exe: $(SRC)
	$(CC) src/main.c -L./. -l:chipmunk_lib.a -lm -ldl -o exe -g -Iinclude_chipmunk -Iinclude -I/usr/local/include/SDL2 -Bstatic -lSDL2 -Wall -pthread external.o -Lchipmunk_lib 

external:
	$(CC) src/external_libs.c -c -o external.o -O3 -Iinclude \
    -Iinclude_chipmunk -I/usr/local/include/SDL2 -lSDL2 -Wall 

chipmunk: $(OBJ) 
	ar rcs chipmunk_lib.a $^ 

$(ODIR)/%.o : src_chipmunk/%.c 
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -o $@ -c -O3 $< -Iinclude_chipmunk

.PHONY: clean
clean:
	rm -rf $(ODIR) chipmunk_lib.a exe

