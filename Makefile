CC=gcc
CC_WIN=x86_64-w64-mingw32-gcc

SRC_CHIPMUNK = $(wildcard src_chipmunk/*.c)
SRC = $(wildcard src/*)

DEBUG0 = -g
CFLAGS = -DNDEBUG
ODIR = obj
ODIR_WIN = obj_win
_OBJ = $(SRC_CHIPMUNK:.c=.o)
OBJ = $(patsubst %,$(ODIR)/%,$(notdir $(_OBJ)))
OBJ_WIN = $(patsubst %,$(ODIR_WIN)/%,$(notdir $(_OBJ)))

evodraw: $(SRC)
	$(CC) src/main.c -L./. -l:chipmunk_lib.a -lm -ldl -o evodraw -g -Iinclude_chipmunk -Iinclude -I/usr/local/include/SDL2 -Bstatic -lSDL2 -Wall -pthread external.o -Lchipmunk_lib 

evodraw_win: $(OBJ_WIN)
	$(CC_WIN) src/main.c -lm -o evodraw_win externalWin.o -g -Iinclude_chipmunk -Iinclude -I../SDL2/include -L../SDL2/lib -Wall  \
		-static -lmingw32 -lSDL2main -lSDL2 -mwindows -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lsetupapi -lhid -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc -lopengl32 $^

external:
	$(CC) src/external_libs.c -c -o external.o -O3 -Iinclude \
    -Iinclude_chipmunk -I/usr/local/include/SDL2 -lSDL2 -Wall 

external_win: 
	$(CC_WIN) src/external_libs.c -c -lm -o externalWin.o -g -O3 -Iinclude_chipmunk -Iinclude -I../SDL2/include \
		-L../SDL2/lib -Wall 

chipmunk: $(OBJ) 
	ar rcs chipmunk_lib.a $^ 

$(ODIR)/%.o : src_chipmunk/%.c 
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -o $@ -c -O3 $< -Iinclude_chipmunk

$(ODIR_WIN)/%.o : src_chipmunk/%.c 
	@mkdir -p $(ODIR_WIN)
	$(CC_WIN) $(CFLAGS) -o $@ -c -O3 $< -Iinclude_chipmunk

.PHONY: clean
clean:
	rm -rf $(ODIR) $(ODIR_WIN) chipmunk_lib_win.a chipmunk_lib.a exe deploy/win/* deploy/linux/*

