LDFLAGS:=-lsqlite3 -lmosquitto -luci
CFLAGS:=-Wall
BIN:=mqttsub

SRC:=$(wildcard *.c)
OBJS:=$(SRC:.c=.o)

.PHONY=all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(OBJS)
