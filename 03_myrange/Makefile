CC := clang
#CFLAGS :=  -Wall -Wextra -Werror

.PHONY: all hello myrange
all: build/hello_range build/myrange

hello: build/hello_range
	./build/hello_range

myrange: build/myrange
	./build/myrange

build/hello_range: main.c build/libhello.dylib
	$(CC) $(CFLAGS) main.c -o $@ \
		-Lbuild/ -Wl,-needed-lhello -Wl,-rpath,@executable_path

build/myrange: main.c build/libdummy.dylib
	$(CC) $(CFLAGS) main.c -o $@ \
		-Lbuild/ -Wl,-needed-ldummy -Wl,-rpath,@executable_path

# deps
build/libhello.dylib: hello_range.c
	mkdir -p build/
	$(CC) -dynamiclib $(CFLAGS) $^ -o $@ \
		-Wl,-install_name,@rpath/libhello.dylib 

build/libdummy.dylib: dummy.c range.S
	mkdir -p build/
	$(CC) -dynamiclib $(CFLAGS) $^ -o $@ \
		-Wl,-install_name,@rpath/libdummy.dylib 

.PHONY: clean
clean: 
	rm -rf build/
