CC := clang
#CFLAGS :=  -Wall -Wextra -Werror

.PHONY: all run
all: build/ordinary build/interposed

build/ordinary: main.c build/libfoo.dylib
	$(CC) $(CFLAGS) main.c -Lbuild/ -lfoo -Wl,-rpath,@executable_path -o $@


build/interposed: main.c build/libfoo.dylib build/libevilfoo.dylib
	$(CC) $(CFLAGS) main.c -Lbuild/ -lfoo -Wl,-needed-levilfoo -Wl,-rpath,@executable_path -o $@

run: all
	./build/ordinary
	./build/interposed
	
# deps
build/libfoo.dylib: libfoo.c
	mkdir -p build/
	$(CC) -dynamiclib $(CFLAGS) $^ -o $@ \
		-Wl,-install_name,@rpath/libfoo.dylib


build/libevilfoo.dylib: libevilfoo.c build/libfoo.dylib
	$(CC) -dynamiclib $(CFLAGS) $^ -Lbuild/ -lfoo -o $@ \
		-Wl,-rpath,@loader_path \
		-Wl,-install_name,@rpath/libevilfoo.dylib 

.PHONY: clean
clean: 
	rm -rf build/
