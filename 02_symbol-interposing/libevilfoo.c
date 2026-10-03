extern int foo(void);

static int evil_foo(void) {
	int cube = 3 * 3 * 3;
	return foo() + cube;
}


/* see https://github.com/apple-oss-distributions/dyld/blob/dyld-1378/include/mach-o/dyld-interposing.h */
__attribute__((used, section("__DATA,__interpose")))
static struct {
	const void* replacement, *replacee;
} interposition = {
	(const void *)evil_foo,
	(const void *)foo,
};
