
# Writeup
This writeup is a tutorial on how to get working with restartable ranges (see [0]) on release kernels. Use at your own risk!

Prerequisites
- You want to use them and have an idea what they might be good for.
- Basic multi-threaded programming in C (e.g. `pthread.h`)
- a tiny bit of assembly (arm64)

## What is a restartable range?

For the purposes of this writeup, we will introduce some terminology that may diverge from what the XNU devs have actually implemented/intended.

1. A **restartable range** consists of
    - a region of user-space code called its **critical section**, together with
    - an address in executable memory called its **recovery address**
2. Registering restartable ranges:
    - Registration may only occur if the task is single-threaded
    - On release kernels (not debug or dev), registration may only happen _once_ (this is will be important for later!)
3. the registered critical sections _must not overlap_!
4. A thread is said to
    - be **redirectable** if the user-space program counter (**UPC**) lies within one of the tasks's registered critical sections (_excluding boundary points_)
    - _have been_ **redirected**, if its UPC is set to the corresponding range's recovery address (which is unique by 3.)

Essentially, the synchronization mechanism (`task_restartable_ranges_synchronize`) redirects every other thread currently in a critical section, with a few caveats:

- If a thread is currently executing, it sets a flag `AST_RESET_PCS` that requests the thread redirect its UPC.
- If, while accessing memory during a critical section, the thread were to fault, then (despite redirecting the UPC) the kernel could still try to access memory that has possibly been invalidated by a writer (the thread issuing the synchronization request).

5. The synchronization primitive `task_restartable_ranges_synchronize` waits until all requested redirections have been acknowledged and all relevant fault handlers have returned.


For the remainder of the writeup, I suggest you clone the directory and follow along the chapters by running the relevant `make` commands in each subdir.

References


- [0]: <https://github.com/apple-oss-distributions/xnu/blob/main/osfmk/kern/restartable.c>


---


## `00_hello_ranges/`

In this program, we define a dummy restartable range (see `ranges.S`) and try to register it with `task_restartable_ranges_register`.

If we try running this program, it will fail as the kernel responds with `KERN_NOT_SUPPORTED (46)`.
```sh
cc main.c range.S -o main.out && ./main.out     # `make` for the lazy
```
To figure out why, let us start up a debugger, insert a breakpoint at `main` and at `task_restartable_ranges_register` and follow execution.
```sh
make inspect        # for the lazy
```
Note that even _before_ we enter `main`, the `task_restartable_ranges_register` breakpoint is triggered by `dyld>libSystem>libdispatch>libobjc`..
If we read the source code, we see that release kernels may only register restartable ranges once in their lifetime [1].

Our takeaway is that if we want to use restartable ranges in our own program, we must somehow take over their registration and append our ranges to that list.


References

- [1]: <https://github.com/apple-oss-distributions/xnu/blob/main/osfmk/kern/restartable.c#L562>


---


## `01_dyld/`

Skip this section if you know what the following command does
```sh
cc main.c -Lbuild -lfoo -Wl,-rpath,@executable_path -o main.out
```

Notation: We write (capital C) **Compiler** for the _program_ `/usr/bin/cc`, and (lowercase c) **compiler** for the abstract machine that does the _compilation step_ such as `cc -S` (as opposed to the many other steps that the Compiler does).

Suppose you wanted to call a function `foo` that is defined in another file `libfoo.c`. If you declare it with `extern int foo(void);`, you can use it freely and the compiler will be happy. However when building an executable, the linker will complain
```sh
cc main.c       # `make bad` for the lazy
```
The basic solution would be to pass `libfoo.c` to our Compiler
```sh
cc main.c libfoo.c -o main.direct   # `make direct` for the lazy
```
However, suppose we had multiple programs that used `libfoo`, so doing this would mean that `libfoo.c` has to be analyzed every single time.

### Object Files
To save compilation time, we could create an **object file**
```sh
cc libfoo.c -o libfoo.o
cc main.c libfoo.o -o main.object
# `make object` for the lazy
```

Suppose now libfoo were split into many small files `libfoo1.c, libfoo2.c, libfoo3.c, ...`. This would mean our compilation commands would get super long. We can therefore create a **static library**: an _archive_ of object files.
```sh
ar rs libfoo.a libfoo.o # libfoo1.o libfoo2.o ...
cc main.c -L./ -lfoo -o main.static
```
where `-lfoo` asks the compiler to search for a library named `libfoo` and `-L./` says where to find it (the current working directory).

Note that if we ever update the library, we must then relink the application.

### Dynamic Libraries
A **dynamic library** (**dylib**) can be updated without having to relink all programs that use it.
```sh
cc -dynamiclib libfoo.c -o libfoo.dylib
cc main.c -L./ -lfoo -o main.dynamic
# `make dynamic` for the lazy
```
Note: On Linux, the build artifacts for dynamic libraries are called "shared object files" and have the `.so` suffix (as opposed to `.dylib`)

We aren't fully done yet. For reaons outlined in Quinn's Post on the Apple Developer Forum on "Dynamic Library Identification" [1] (or the developer archive [2]), we should add the location of `libfoo.dylib` to the linker's **runtime path** (**rpath**).
TODO: elaborate

```sh
mkdir -p build/
cc -dynamiclib libfoo.c -o build/libfoo.dylib \
    -Wl,-install_name,@rpath/libfoo.dylib
cc main.c -Lbuild/ -lfoo \
    -Wl,-rpath,@executable_path -o main.rpath
# `make rpath` for the lazy
```

We note some relevant information with `make inspect`.
TODO: elaborate

References
[1] Dynamic Library Identification - Quinn's Post in Developer Forums: <https://developer.apple.com/forums/thread/736719>
[2] Apple Developer Archive - Dynamic Library Programming Topics - Run-Path Dependent Libraries: <https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/DynamicLibraries/100-Articles/RunpathDependentLibraries.html>



---


## `02_symbol-interposing`

Skip this section if you have already read through Derek Selander's excellent writeup on symbol interposing [3].

Check out the `Makefile`. We are merely building a dynamic library `libevilfoo.dylib` and linking `main.c` against it with the `-neededlevilfoo` which tells the linker to record `libevilfoo.dylib` as a dependency even though `main.c` does not import a function from there.

```sh
make run
```


[3] Derek Selander - Symbol Interposing: <https://github.com/DerekSelander/symbol-interposing>
[4] Apple OSS Distributions (GitHub) - `dyld-interposing.h`: <https://github.com/apple-oss-distributions/dyld/blob/dyld-1378/include/mach-o/dyld-interposing.h>


---


## `03_myrange`

In this section, we will finally register our own restartable range using symbol interposing.

Recall that range registration happens before `main` even runs, so this time our `main` function will be as boring as can be
```c
int main(void) { return 0; }
```
We can also essentially just copy over our scheme from chapter `02` and apply it to `task_restartable_ranges_register` by printing `"Hello interpose!"` before calling `task_restartable_ranges_register` (see `hello_range.c`).

```sh
make hello
```

In order to actually register a range, let us copy what we had in chapter `00`'s `main`, but this time put the logic into a dylib we are loading (see `dummy.c:interposed_registration`).
If what he have gathered so far is right, then we can intercept the function arguments, append our dummy range and call the real function with the extra range.

```sh
make myrange
```

Fun fact: It seems like macOS 26 SDK's `libobjc` registered only 5 ranges, whereas whatever macOS 27 uses registers 39 ranges! It seems as though the libobjc devs are putting restartable ranges to good use.


---


## `04_first_example`

This toy example is there to test if the registration really suceeded.
A "reader" thread will spin in a loop that can only exit if it is redirected.

```asm
.text
.globl _myrange_start, _myrange_end, _myrange_recovery
_myrange_start:
    nop
loop:
    b loop
_myrange_end:
_myrange_recovery:
    ret
```
notes: the since synchronization checks that `range.location < PC` (strictly larger), we put a `nop` after the start.

Our main thread will spawn the reader, sleep for two seconds and then call `task_restartable_ranges_synchronize`, which should put the looping thread out of its misery.

```sh
make run # for the lazy
```

---


TODO:



