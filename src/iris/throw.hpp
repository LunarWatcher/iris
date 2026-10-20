#pragma once

#if (defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)) && !defined(IRIS_NOEXCEPTION)
#ifndef IRIS_THROW
#define IRIS_THROW(exception) throw exception
#endif
#else
#include <cstdlib>
#ifndef IRIS_THROW
#define IRIS_THROW(exception)                   \
    std::abort();                               \
    std::ignore = exception
#endif
#ifndef IRIS_NOEXCEPTION
#define IRIS_NOEXCEPTION
#endif
#endif
