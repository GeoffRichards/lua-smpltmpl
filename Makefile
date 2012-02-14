CFLAGS = -g -O -W -Wall -std=c99

tmplcmpl: tmplcmpl.o
	$(CC) $(LDFLAGS) -o $@ $^
tmplcmpl.o: tmplcmpl.c
	$(CC) $(CFLAGS) -c $^

test: test/foo.qtmpl.lua
	lua test/run.lua
	diff -u test/out.expected test/out.got
test/foo.qtmpl.lua: test/foo.qtmpl
	./tmplcmpl $< $@

clean:
	rm -f tmplcmpl tmplcmpl.o

.PHONY: test clean
