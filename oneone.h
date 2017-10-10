#pragma once

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

typedef enum one_error_e {
  ESUCCESS = 0,
  ECLOSEDCHAN = -1,
} one_error_e;

typedef void * (*one_locked_val_fn) (void * const);
typedef void (*one_locked_val_read_fn) (void * const);

// locked value

typedef struct one_locked_val_s one_locked_val_s;

one_locked_val_s *
one_locked_val_new(void * const initial_val);

void *
one_locked_val_free(one_locked_val_s * const lv);

void *
one_locked_val_set(one_locked_val_s * const lv, void * const new_val);

void *
one_locked_val_get(one_locked_val_s * const lv);

void *
one_locked_val_with(one_locked_val_s * const lv, one_locked_val_fn fn);

// rwlocked value

typedef struct one_rwlocked_val_s one_rwlocked_val_s;

one_rwlocked_val_s *
one_rwlocked_val_new(void * const initial_val);

void *
one_rwlocked_val_free(one_rwlocked_val_s * const rwlv);

void *
one_rwlocked_val_set(one_rwlocked_val_s * const rwlv, void * const new_val);

void *
one_rwlocked_val_get(one_rwlocked_val_s * const rwlv);

void *
one_rwlocked_val_read_with(one_rwlocked_val_s * const rwlv, one_locked_val_read_fn fn);

void *
one_rwlocked_val_with(one_rwlocked_val_s * const rwlv, one_locked_val_fn fn);

// wait group

typedef struct one_wait_group_s one_wait_group_s;

one_wait_group_s *
one_wait_group_new(int initial_count);

void
one_wait_group_free(one_wait_group_s * const wg);

void
one_wait_group_add(one_wait_group_s * const wg, int delta);

void
one_wait_group_done(one_wait_group_s * const wg);

void
one_wait_group_wait(one_wait_group_s * const wg);

// channel

typedef struct one_chan_s one_chan_s;

one_chan_s *
one_chan_new(void);

void
one_chan_free(one_chan_s * const ch);

int
one_chan_send(one_chan_s * const ch, void * value);

int
one_chan_recv(one_chan_s * const ch, void ** value);

int
one_chan_close(one_chan_s * const ch);

// spawn

/*
 * one_spawn launches a new thread in "fire and forget" mode
 * (i.e. detached state) to run the provided function. The arg
 * is passed to the function as argument, it can be NULL.
 * It returns 0 on success, the pthread error number on error.
*/
int
one_spawn(void (*fn) (void *), void * arg);

/*
 * one_spawn_ssz is identical to one_spawn except that the thread's
 * stack size can be specified.
*/
int
one_spawn_ssz(void (*fn) (void *), void * arg, size_t stack_sz);

int
one_spawn_wg(one_wait_group_s * const wg, void (*fn) (void *), void * arg);

int
one_spawn_wg_ssz(one_wait_group_s * const wg, void (*fn) (void *), void * arg, size_t stack_sz);

// version

typedef struct one_version_s {
  int major;
  int minor;
  int patch;
} one_version_s;

one_version_s
one_version(void);

