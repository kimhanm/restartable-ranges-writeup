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

/* see ranges.S */
extern char myrange_start[], myrange_end[], myrange_recovery[];

int main(void) {

    struct task_restartable_range_t myrange = {
        .location = (mach_vm_address_t)myrange_start,
        .length = (unsigned short)(myrange_end - myrange_start),
        .recovery_offs = (unsigned short)(myrange_recovery - myrange_start),
        .flags = 0,
    };

    kern_return_t kr =
        task_restartable_ranges_register(mach_task_self(), &myrange, 1);

    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "task_restartable_ranges_register: %s (%d)\n",
                mach_error_string(kr), kr);
        exit(EXIT_FAILURE);
    }

    printf("success\n");

    return 0;
}
