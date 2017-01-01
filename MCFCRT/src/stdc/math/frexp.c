// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef frexpf
#undef frexp
#undef frexpl

static inline long double fpu_frexp(long double x, int *exp){
	if(x == 0){
		*exp = 0;
		return 0;
	} else {
		long double n;
		const long double m = __MCFCRT_fxtract(&n, x);
		__MCFCRT_fistp(exp, n + 1);
		return m / 2;
	}
}

float frexpf(float x, int *exp){
	return (float)fpu_frexp(x, exp);
}
double frexp(double x, int *exp){
	return (double)fpu_frexp(x, exp);
}
long double frexpl(long double x, int *exp){
	return fpu_frexp(x, exp);
}
