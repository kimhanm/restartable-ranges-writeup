#include <mach/mach.h>
#include <stdio.h>
#include <stdlib.h>

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

extern char myrange_start[], myrange_end[], myrange_recovery[];

static kern_return_t
interposed_registration(task_t task, struct task_restartable_range_t *ranges,
                        mach_msg_type_number_t count) {

    struct task_restartable_range_t myrange = {
        .location = (mach_vm_address_t)myrange_start,
        .length = (unsigned short)(myrange_end - myrange_start),
        .recovery_offs = (unsigned short)(myrange_recovery - myrange_start),
        .flags = 0,
    };

    struct task_restartable_range_t rs[64];
    memcpy(rs, ranges, count * sizeof(struct task_restartable_range_t));
    rs[count] = myrange;

    // fprintf(stderr, "about to register %d + 1 ranges\n", count);

    return task_restartable_ranges_register(task, rs, count + 1);
}

__attribute__((used, section("__DATA,__interpose"))) static struct {
    const void *replacement, *replacee;
} interposition = {
    (const void *)interposed_registration,
    (const void *)task_restartable_ranges_register,
};
