#pragma once

// NOTE: clang defines __GNUC__ too.
#ifdef __GNUC__
#define unused __attribute__((unused))
#else
#define unused
#endif
