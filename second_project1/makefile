FUNCTION = $(wildcard fun/*.c)
MAIN_PATH = $(wildcard src/*.c)
FUNCTION_OBJ = $(patsubst %.c,%.o,$(FUNCTION))
INCLUDE = -I inc
LIB_PATH = libfile/libfun.a
LIBRARY = -L libfile/ -lfun
EXE_PATH = bin/main
%.o:%.c
	gcc $^ -c -o $@ $(INCLUDE) -lpthread

$(LIB_PATH):$(FUNCTION_OBJ)
	ar rcs $@ $^
	gcc ${MAIN_PATH} -o ${EXE_PATH} ${INCLUDE} ${LIBRARY} -lpthread

clean:
	rm ${EXE_PATH}
	rm ${LIB_PATH}
	rm ${FUNCTION_OBJ}
