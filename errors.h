#pragma once

#include <stdio.h>
#include <string.h>
#include <errno.h>

// FATAL prints msg and aborts unconditionnally.
#define FATAL(msg)                   do {                                                   \
                                       printf("%s\n", (msg));                               \
                                       abort();                                             \
                                     } while(0);

// ERRCLEANUP jumps to the error##labelnum label if errVal is not 0.
#define ERRCLEANUP(errVal, labelnum) if((errVal)) goto error##labelnum

// ERRFATAL prints the error message and aborts if errVal is not 0.
#define ERRFATAL(errVal, context)     do {                                                  \
                                        int theErr = (errVal);                              \
                                        if(theErr) {                                        \
                                          printf("%s: %s\n", (context), strerror(theErr));  \
                                          abort();                                          \
                                        }                                                   \
                                      } while(0);

// NULLFATAL prints msg and aborts if nullVal is NULL.
#define NULLFATAL(nullVal, msg)       do {                                                  \
                                        if(!(nullVal)) {                                    \
                                          printf("%s\n", (msg));                            \
                                          abort();                                          \
                                        }                                                   \
                                      } while(0);


// NEGFATAL prints msg and aborts if intVal is negative.
#define NEGFATAL(intVal, msg)       do {                                                    \
                                        if((intVal) < 0) {                                  \
                                          printf("%s: %s\n", (msg), strerror(errno));       \
                                          abort();                                          \
                                        }                                                   \
                                      } while(0);


