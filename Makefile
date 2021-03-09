CC=gcc

SRC = $(wildcard src_chipmunk/*.c)

DEBUG0 = -g
DEBUG1 = -fsanitize=address -g

ODIR = obj
_OBJ = $(SRC:.c=.o)
OBJ = $(patsubst %,$(ODIR)/%,$(notdir $(_OBJ)))

chipmunk_lib.a: $(OBJ) 
	ar rcs $@ $^ 

$(ODIR)/%.o : src_chipmunk/%.c 
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -o $@ -c $< -Iinclude_chipmunk

.PHONY: clean
clean:
	rm -rf $(ODIR) chipmunk_lib.a

