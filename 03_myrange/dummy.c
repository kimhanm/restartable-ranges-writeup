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

/* dummy range (see ranges.S) */
extern char myrange_start[], myrange_end[], myrange_recovery[];

static kern_return_t
interposed_registration(task_t task, struct task_restartable_range_t *ranges,
                        mach_msg_type_number_t count) {

    /* let us inspect what these ranges are doing */
    // for (mach_msg_type_number_t i = 0; i < count; ++i) {
    //     fprintf(stderr, "range %d start %lu\n", i,
    //             (unsigned long)ranges[i].location);
    //     fprintf(stderr, "range %d recovery %lu\n", i,
    //             (unsigned long)ranges[i].location +
    //                 (unsigned long)ranges[i].recovery_offs);
    // }

    struct task_restartable_range_t myrange = {
        .location = (mach_vm_address_t)myrange_start,
        .length = (unsigned short)(myrange_end - myrange_start),
        .recovery_offs = (unsigned short)(myrange_recovery - myrange_start),
        .flags = 0,
    };

    struct task_restartable_range_t rs[64];
    memcpy(rs, ranges, count * sizeof(struct task_restartable_range_t));
    rs[count] = myrange;

    fprintf(stderr, "about to register %d + 1 ranges\n", count);

    return task_restartable_ranges_register(task, rs, count + 1);
}

/* see chapter 02: libevilfoo.c */
__attribute__((used, section("__DATA,__interpose"))) static struct {
    const void *replacement, *replacee;
} interposition = {
    (const void *)interposed_registration,
    (const void *)task_restartable_ranges_register,
};
