CFLAGS=-DGL_GLEXT_PROTOTYPES -Wextra -std=c99
OBJS=
LDFLAGS=
LDLIBS=-lm -lGL -lGLU -lglut
CC=gcc
CPP=g++


all : check_gl check_texsize linear_mapping max_reduce

max_reduce: max_reduce.o glsl_utils.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

linear_mapping: linear_mapping.o glsl_utils.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

check_texsize: check_texsize.o glsl_utils.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

check_gl: check_gl.o glsl_utils.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

%: %.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean :
	rm check_gl
	rm check_texsize
	rm linear_mapping
	rm max_reduce
	rm check_gl.o
	rm check_texsize.o
	rm linear_mapping.o
	rm max_reduce.o
	rm glsl_utils.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cc
	$(CPP) -c -o $@ $< $(CFLAGS)

.PHONY: all clean
