
#pragma once

#ifndef NDEBUG
#include <cassert>
#define ASSERT(exp) assert(exp)
#define VERIFY(exp) assert(exp)
#else
#define ASSERT(exp)
#define VERIFY(exp) exp
#endif
