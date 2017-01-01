// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef remainderf
#undef remainder
#undef remainderl

float remainderf(float x, float y){
	unsigned fsw;
	return (float)__MCFCRT_fremainder(&fsw, x, y);
}
double remainder(double x, double y){
	unsigned fsw;
	return (double)__MCFCRT_fremainder(&fsw, x, y);
}
long double remainderl(long double x, long double y){
	unsigned fsw;
	return __MCFCRT_fremainder(&fsw, x, y);
}
