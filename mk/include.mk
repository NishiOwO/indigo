P_CC ?= $(CROSS)gcc
P_RC ?= $(CROSS)windres

CFLAGS = $(P_CFLAGS) -I include
LDFLAGS = $(P_LDFLAGS)
LIBS = $(P_LIBS)

OBJS = $(P_OBJS)
OBJS += src/main.o

.PHONY: all clean format
.SUFFIXES: .c .o .rc .res

all: indigo$(EXEC)

indigo$(EXEC): $(OBJS)
	$(P_CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(AFTERCMD)

.c.o:
	$(P_CC) $(CFLAGS) -c -o $@ $<

.rc.res:
	$(P_RC) -O coff $< $@

src/indigo.res: src/indigo.rc src/images/indigo.ico src/images/splash.bmp

format:
	clang-format --verbose -i `find include src "(" -name "*.c" -or -name "*.h" ")" -and -not -name "stb_ds.h"`

clean:
	rm -f *.exe src/*.o src/*.res src/*/*.o indigo
