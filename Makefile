CC=g++
CFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter -std=c++11
LDFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter
EXEC=nagios-json
SRC=$(wildcard *.cxx)
OBJ=$(SRC:.cxx=.o)
OBJDB=$(SRC:.cxx=-db.o)
LIB=
INCLUDE=

all: $(EXEC) $(EXEC)-db

$(EXEC): $(OBJ)
	$(CC) $(LDFLAGS) -O3 -march=native -flto -fwhole-program -s $(LIB) -o $(EXEC) $^

$(EXEC)-db: $(OBJDB)
	$(CC) $(LDFLAGS) $(LIB) -o $(EXEC)-db $^

main.o: string_map.h json.h nagios_host.h nagios_perfdata.h nagios_range.h nagios_service.h strutil.h
nagios_host.o: json.h nagios_host.h nagios_perfdata.h nagios_range.h nagios_service.h
nagios_perfdata.o: json.h lexer.h nagios_perfdata.h nagios_range.h strutil.h
nagios_range.o: json.h lexer.h nagios_range.h strutil.h
nagios_service.o: json.h nagios_perfdata.h nagios_range.h nagios_service.h
string_map.o: string_map.h strutil.h
strutil.o: strutil.h

%.o: %.cxx globals.h
	$(CC) $(CFLAGS) -O3 -march=native -flto -fwhole-program $(INCLUDE) -o $@ -c $<

%-db.o: %.cxx %.o
	$(CC) $(CFLAGS) -g $(INCLUDE) -o $@ -c $<

.PHONY: clean mrproper install

install:
	install -o root -g www-data -m 755 nagios-json /usr/bin/nagios-json
	install -o root -g www-data -m 644 nagios-json.conf /etc/nagios-json.conf

clean:
	rm *.o

mrproper: clean
	rm $(EXEC)
	rm $(EXEC)-db
