SRC:=$(wildcard *.c)
BIN:=sockets

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC)
	$(CC) -o $@ $^ -lblobmsg_json -lubox -lubus

clean:
	rm -f $(BIN)
