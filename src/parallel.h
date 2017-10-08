#pragma once

// Users must include:
// #include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

typedef enum parallel_error {
  ESUCCESS = 0,
  ECLOSEDCHAN = -1,
} parallel_error;

typedef void *(*parallel_locked_val_func) (void *);
typedef void (*parallel_locked_rdval_func) (void *);

// locked value

typedef struct parallel_locked_val_s parallel_locked_val_s;

parallel_locked_val_s *
parallel_locked_val_new(void *initial_val);

void *
parallel_locked_val_free(parallel_locked_val_s *lv);

void *
parallel_locked_val_set(parallel_locked_val_s *lv, void *new_val);

void *
parallel_locked_val_get(parallel_locked_val_s *lv);

void *
parallel_locked_val_with(parallel_locked_val_s *lv, parallel_locked_val_func fn);

// rwlocked value

typedef struct parallel_rwlocked_val_s parallel_rwlocked_val_s;

parallel_rwlocked_val_s *
parallel_rwlocked_val_new(void *initial_val);

void *
parallel_rwlocked_val_free(parallel_rwlocked_val_s *rwlv);

void *
parallel_rwlocked_val_set(parallel_rwlocked_val_s *rwlv, void *new_val);

void *
parallel_rwlocked_val_get(parallel_rwlocked_val_s *rwlv);

void *
parallel_rwlocked_val_rdwith(parallel_rwlocked_val_s *rwlv, parallel_locked_rdval_func fn);

void *
parallel_rwlocked_val_wrwith(parallel_rwlocked_val_s *rwlv, parallel_locked_val_func fn);

// wait group

typedef struct parallel_wait_group_s parallel_wait_group_s;

parallel_wait_group_s *
parallel_wait_group_new(int initial_count);

void
parallel_wait_group_free(parallel_wait_group_s *wg);

void
parallel_wait_group_add(parallel_wait_group_s *wg, int delta);

void
parallel_wait_group_done(parallel_wait_group_s *wg);

void
parallel_wait_group_wait(parallel_wait_group_s *wg);

// channel

typedef struct parallel_channel_s parallel_channel_s;

parallel_channel_s *
parallel_channel_new();

void
parallel_channel_free(parallel_channel_s *ch);

int
parallel_channel_send(parallel_channel_s *ch, void *value);

int
parallel_channel_recv(parallel_channel_s *ch, void **value);

int
parallel_channel_close(parallel_channel_s *ch);

// spawn

/*
 * parallel_spawn launches a new thread in "fire and forget" mode
 * (i.e. detached state) to run the provided function. The arg
 * is passed to the function as argument, it can be NULL.
 * It returns 0 on success, the pthread error number on error.
*/
int
parallel_spawn(void (*fn) (void *), void *arg);

/*
 * parallel_spawn_ssz is identical to parallel_spawn except that the thread's
 * stack size can be specified.
*/
int
parallel_spawn_ssz(void (*fn) (void *), void *arg, size_t stack_sz);

int
parallel_spawn_wg(parallel_wait_group_s *wg, void (*fn) (void *), void *arg);

int
parallel_spawn_wg_ssz(parallel_wait_group_s *wg, void (*fn) (void *), void *arg, size_t stack_sz);

