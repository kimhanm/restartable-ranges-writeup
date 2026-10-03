CC := clang
CFLAGS :=  -Wall -Wextra -Werror

.PHONY: all run
all: build/synchronize

run: build/synchronize
	./build/synchronize

build/synchronize: main.c build/libmyrange.dylib
	$(CC) $(CFLAGS) main.c -o $@ \
		-Lbuild/ -lmyrange -Wl,-needed-lmyrange -Wl,-rpath,@executable_path

# deps
build/libmyrange.dylib: myrange.c range.S
	mkdir -p build/
	$(CC) -dynamiclib $(CFLAGS) $^ -o $@ \
		-Wl,-install_name,@rpath/libmyrange.dylib 

.PHONY: clean
clean: 
	rm -rf build/
