
CC ?= gcc

PROG = clapl
OBJ += logger.o configparser.o hashtable.o
OBJ += db.o cache.o ui.o playback.o input.o
OBJ += commandtable.o commandparser.o cmd.o

LDFLAGS += -lncurses -lavcodec -lavformat -lavutil -lm -lsqlite3 -lpthread -lao


all: $(PROG)


$(PROG): $(PROG).c $(PROG).h $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)


.o: .c .h
	$(CC) -c $@ $^


install: all
	@sudo install -Dm755 $(PROG) $(DESTDIR)$(PREFIX)/bin/$(PROG)


clean:
	rm $(OBJ)


.PHONY:
	all
	install
	clean
