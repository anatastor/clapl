
CC ?= gcc

PROG = clapl
OBJ += logger.o configparser.o hashtable.o
OBJ += db.o cache.o ui.o playback.o input.o
OBJ += cmdmanager.o cmd.o

LDFLAGS += -lncurses -lavcodec -lavformat -lavutil -lm -lsqlite3 -lpthread -lao


all: $(PROG)


$(PROG): $(PROG).c $(PROG).h $(OBJ)
	$(CC) -g -o $@ $^ $(LDFLAGS)


.o: .c .h
	$(CC) -g -c $@ $^


install: all
	@mkdir -p ~/.config/clapl
	@cp -n config ~/.config/clapl
	@sudo install -Dm755 $(PROG) $(DESTDIR)$(PREFIX)/bin/$(PROG)


clean:
	rm $(OBJ)


.PHONY:
	all
	install
	clean
