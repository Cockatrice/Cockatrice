#ifndef MSVC_ROUND_FIX
#define MSVC_ROUND_FIX
/**
 * This helper function exists only because MS VC++ 2010 does not support round() in 
 * <cmath>. round() works with g++ and clang++ but is formally a C++11 extension.
 * So this file exists for MS VC++ only.
 */
inline double round(double val) {
	return floor(val + 0.5);
}

#endif /* MSVC_ROUND_FIX */
