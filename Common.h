#pragma once
#include <cstdio>
#define DBG_ASSERT(condition, msg, ...) do {if (!(condition)) {fprintf(stderr, "(%s:%d) %s : " msg  "\n", __FILE__, __LINE__, #condition, ##__VA_ARGS__ ); fflush(stderr);  __debugbreak();}} while (0)
