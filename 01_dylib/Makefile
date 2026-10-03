CC := clang
#CFLAGS :=  -Wall -Wextra -Werror

.PHONY: all bad direct object static dylib rpath inspect
all: direct object static dynamic rpath

bad: main.c
	$(CC) $(CFLAGS) main.c

direct: main.c libfoo.c
	$(CC) $(CFLAGS) $^ -o main.direct

object: main.c libfoo.o
	$(CC) $(CFLAGS) $^ -o main.object

static: main.c libfoo.a
	$(CC) $(CFLAGS) main.c -L. -lfoo -o main.static

dynamic: main.c libfoo.dylib
	$(CC) $(CFLAGS) main.c -L. -lfoo -o main.dynamic

rpath: main.c build/libfoo.dylib
	$(CC) $(CFLAGS) main.c -Lbuild/ -lfoo -Wl,-rpath,@executable_path -o main.rpath
	

inspect: object static dynamic rpath 
	otool -L main.object
	otool -L main.static
	otool -L main.dynamic
	otool -L main.rpath
	otool -D libfoo.dylib
	otool -D build/libfoo.dylib

# deps

libfoo.o: libfoo.c
	$(CC) $(CFLAGS) -c $^ -o $@

libfoo.a: libfoo.o
	ar rs libfoo.a $^

libfoo.dylib: libfoo.c
	$(CC) -dynamiclib $(CFLAGS) $^ -o $@

build/libfoo.dylib: libfoo.c
	mkdir build/
	$(CC) -dynamiclib $(CFLAGS) $^ -o $@ \
		-Wl,-install_name,@rpath/libfoo.dylib

.PHONY: clean
clean: 
	rm -f main.direct main.object main.static main.dynamic main.rpath libfoo.o libfoo.dylib libfoo.a build/
