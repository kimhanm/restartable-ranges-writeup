#include <mach/mach.h>
#include <stdlib.h>
#include <stdio.h>

struct task_restartable_range_t {
    mach_vm_address_t location;
    unsigned short length;
    unsigned short recovery_offs;
    unsigned int flags;
};
extern kern_return_t
task_restartable_ranges_register(task_t task,
                                 struct task_restartable_range_t *ranges,
                                 mach_msg_type_number_t count);

static kern_return_t hello_interposer(task_t task,
                                  struct task_restartable_range_t *ranges,
                                  mach_msg_type_number_t count) {
	fprintf(stderr, "Hello interposer!\n");
	return task_restartable_ranges_register(task, ranges, count);
}

/* see chapter 02: libevilfoo.c */
__attribute__((used, section("__DATA,__interpose")))
static struct {
	const void* replacement, *replacee;
} interposition = {
	(const void *)hello_interposer,
	(const void *)task_restartable_ranges_register,
};

