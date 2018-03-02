
#ifndef BYZANCETYPES_H
#define BYZANCETYPES_H
 
#include "mbed.h"

typedef bool        ByzanceBool;
typedef int32_t     ByzanceInt;
typedef float       ByzanceFloat;
typedef char*       ByzanceString;

typedef const bool        ByzanceBoolConst;
typedef const int32_t     ByzanceIntConst;
typedef const float       ByzanceFloatConst;
typedef const char*       ByzanceStringConst;

#define BOOLEAN ByzanceBool
#define INTEGER ByzanceInt
#define FLOAT ByzanceFloat
#define STRING ByzanceString

#define ByzanceBoolFree(ARG) {}
#define ByzanceIntFree(ARG) {}
#define ByzanceFloatFree(ARG) {}
#define ByzanceStringFree(ARG) free(ARG)

#endif /* BYZANCETYPES_H */
