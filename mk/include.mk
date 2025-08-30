CC = $(CROSS)gcc
RC = $(CROSS)windres

CFLAGS = $(P_CFLAGS) -I include
LDFLAGS = $(P_LDFLAGS)
LIBS = $(P_LIBS)

OBJS = $(P_OBJS)
OBJS += src/main.o

.PHONY: all clean format
.SUFFIXES: .c .o .rc .res

all: indigo$(EXEC)

indigo$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.rc.res:
	$(RC) -O coff $< $@

format:
	clang-format --verbose -i `find include src "(" -name "*.c" -or -name "*.h" ")" -and -not -name "stb_ds.h"`

clean:
	rm -f *.exe src/*.o src/*.res src/*/*.o indigo
