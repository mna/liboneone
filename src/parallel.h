#pragma once
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

// Fire and forget

/*
 * parallel_fnf launches a new thread in "fire and forget" mode
 * (i.e. detached state) to run the provided function. The arg
 * is passed to the function as argument, it can be NULL.
 * It returns 0 on success, the pthread error number on error.
*/
int parallel_fnf(void (*fn) (void *), void *arg);

/*
 * parallel_fnf_ssz is identical to parallel_fnf except that the thread's
 * stack size can be specified.
*/
int parallel_fnf_ssz(void (*fn) (void *), void *arg, size_t stack_sz);

// locked value

typedef struct parallel_locked_val_s parallel_locked_val_s;

parallel_locked_val_s * parallel_locked_val_new(void *initial_val);
void * parallel_locked_val_free(parallel_locked_val_s *lv);
void * parallel_locked_val_set(parallel_locked_val_s *lv, void *new_val);
void * parallel_locked_val_get(parallel_locked_val_s *lv);
void * parallel_locked_val_with(parallel_locked_val_s *lv, void *(*fn) (void *));

// wait group

typedef struct parallel_wait_group_s parallel_wait_group_s;

parallel_wait_group_s * parallel_wait_group_new(int initial_count);
void parallel_wait_group_free(parallel_wait_group_s *wg);
void parallel_wait_group_add(parallel_wait_group_s *wg, int delta);
void parallel_wait_group_done(parallel_wait_group_s *wg);
void parallel_wait_group_wait(parallel_wait_group_s *wg);

