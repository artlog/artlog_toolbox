
BINARY=mini

all:	$(BINARY)

$(BINARY):	mini.c
	gcc -I/home/plhardy/artisanlogiciel/code/artlog_toolbox/build/include mini.c -l ldap -L/home/plhardy/artisanlogiciel/code/artlog_toolbox/build/lib -l alcommon -o $@

clean:
	rm $(BINARY)

.PHONY:	all clean
