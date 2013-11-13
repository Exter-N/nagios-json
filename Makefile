CC=g++
CFLAGS=-Wall
LDFLAGS=
EXEC=nagios-json
SRC=$(wildcard *.cxx)
OBJ=$(SRC:.cxx=.o)
OBJDB=$(SRC:.cxx=-db.o)
LIB=
INCLUDE=

all: $(EXEC) $(EXEC)-db

$(EXEC): $(OBJ)
	$(CC) $(LDFLAGS) -O3 -s $(LIB) -o $(EXEC) $^
	sudo chgrp www-data $(EXEC)
	sudo chmod g+s $(EXEC)

$(EXEC)-db: $(OBJDB)
	$(CC) $(LDFLAGS) $(LIB) -o $(EXEC)-db $^
	sudo chgrp www-data $(EXEC)-db
	sudo chmod g+s $(EXEC)-db

main.o: nagios_host.h nagios_service.h json.h

%.o: %.cxx globals.h
	$(CC) $(CFLAGS) -O3 $(INCLUDE) -o $@ -c $<

%-db.o: %.cxx %.o
	$(CC) $(CFLAGS) -g $(INCLUDE) -o $@ -c $<

.PHONY: clean mrproper

clean:
	rm *.o

mrproper: clean
	rm $(EXEC)
	rm $(EXEC)-db
