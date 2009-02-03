/* KallistiOS ##version##

   rwsem_test.c
   Copyright (C) 2008 Lawrence Sebald

*/

/* This program is a test of the reader/writer semaphores added in KOS 1.3.0.
   Basically, what a reader/writer semaphore does is it allows unlimited readers
   to occupy their critical sections, while guaranteeing exclusive access to
   writers. This is a concept that I picked up from my undergraduate operating
   systems class that I figured would probably be useful for KOS. */

#include <stdio.h>

#include <kos/thread.h>
#include <kos/rwsem.h>

#include <arch/arch.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>

#define UNUSED __attribute__((unused))

rw_semaphore_t *s = NULL;
uint32 number = 0;

void writer0(void *param UNUSED) {
    int i;

    for(i = 0; i < 20; ++i) {
        if(rwsem_write_lock(s)) {
            printf("Writer 0 could not obtain write lock!\n");
            return;
        }

        printf("Writer 0 obtained write lock\n");
        number += 8;
        rwsem_write_unlock(s);

        thd_sleep(10);
    }

    printf("Writer 0 done\n");
}

void writer1(void *param UNUSED) {
    int i;

    for(i = 0; i < 17; ++i) {
        if(rwsem_write_lock(s)) {
            printf("Writer 1 could not obtain write lock!\n");
            return;
        }

        printf("Writer 1 obtained write lock\n");
        number *= 3;
        rwsem_write_unlock(s);

        thd_sleep(5);
    }

    printf("Writer 1 done\n");
}

void reader0(void *param UNUSED) {
    int i;

    for(i = 0; i < 12; ++i) {
        if(rwsem_read_lock(s)) {
            printf("Reader 0 could not obtain read lock!\n");
            return;
        }

        printf("Reader 0 obtained read lock\n");
        printf("Number: %lu\n", number);
        rwsem_read_unlock(s);

        thd_sleep(20);
    }

    printf("Reader 0 done\n");
}

void reader1(void *param UNUSED) {
    int i;

    for(i = 0; i < 23; ++i) {
        if(rwsem_read_lock(s)) {
            printf("Reader 1 could not obtain read lock!\n");
            return;
        }

        printf("Reader 1 obtained read lock\n");
        printf("Number * 2: %lu\n", number * 2);
        rwsem_read_unlock(s);

        thd_sleep(16);
    }

    printf("Reader 1 done\n");
}

KOS_INIT_FLAGS(INIT_DEFAULT);
int main(int argc, char *argv[]) {
    kthread_t *w0, *w1, *r0, *r1;

    /* Exit if the user presses all buttons at once. */
    cont_btn_callback(0, CONT_START | CONT_A | CONT_B | CONT_X | CONT_Y,
                      arch_exit);

    printf("KallistiOS Reader/Writer Semaphore test program\n");

    /* Create the reader/writer semaphore that will be used. */
    s = rwsem_create();

    if(!s) {
        printf("Could not create RW semaphore, bailing out!\n");
        arch_exit();
    }

    printf("About to create threads\n");
    w0 = thd_create(writer0, NULL);
    w1 = thd_create(writer1, NULL);
    r0 = thd_create(reader0, NULL);
    r1 = thd_create(reader1, NULL);

    printf("About to sleep\n");
    thd_wait(w0);
    thd_wait(w1);
    thd_wait(r0);
    thd_wait(r1);

    if(rwsem_read_lock(s)) {
        printf("Could not obtain final read lock!\n");
        arch_exit();
    }

    printf("Final number: %lu\n", number);

    rwsem_read_unlock(s);
    rwsem_destroy(s);

    printf("Reader/Writer semaphore tests completed successfully!\n");
    return 0;
}