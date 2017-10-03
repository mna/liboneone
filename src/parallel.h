#pragma once

#define _POSIX_C_SOURCE 200809L

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

