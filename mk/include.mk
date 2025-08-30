CC = $(CROSS)gcc
RC = $(CROSS)windres

CFLAGS = $(P_CFLAGS) -I include
LDFLAGS = $(P_LDFLAGS)
LIBS = $(P_LIBS)

OBJS = $(P_OBJS)
OBJS += src/main.o

.PHONY: all clean
.SUFFIXES: .c .o .rc .res

all: indigo$(EXEC)

indigo$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.rc.res:
	$(RC) -O coff $< $@

clean:
	rm -f *.exe src/*.o src/*.res src/*/*.o indigo
