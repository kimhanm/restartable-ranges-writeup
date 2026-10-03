#include <mach/mach.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern void myrange_start(void);
extern kern_return_t task_restartable_ranges_synchronize(task_t task);

static void *reader(void *arg) {
	(void)arg;
    printf("about to start looping...\n");
    myrange_start();
    return NULL;
}

int main(void) {
    pthread_t th;

    if (pthread_create(&th, NULL, reader, NULL) != 0) {
		fprintf(stderr, "pthread_create");
		exit(EXIT_FAILURE);
	}

    sleep(2);

    task_restartable_ranges_synchronize(mach_task_self());
    pthread_join(th, NULL);
    printf("success\n");

    return 0;
}
