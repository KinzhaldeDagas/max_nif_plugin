#ifndef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include <stdio.h>
#include <tchar.h>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

#if __cplusplus >= 201703L
namespace std {
template <class Arg, class Result>
struct unary_function {
	typedef Arg argument_type;
	typedef Result result_type;
};
template <class Arg1, class Arg2, class Result>
struct binary_function {
	typedef Arg1 first_argument_type;
	typedef Arg2 second_argument_type;
	typedef Result result_type;
};
}
#endif
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>
#include <list>
#include <map>

// _WIN32 will detect windows on most compilers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>

#include "MaxNifImport.h"
#include "MAX_MemDirect.h"

#if VERSION_3DSMAX >= (14000<<16) // Version 14 (2012)
#define SDK_RESERVED_METHOD(a)
#endif
