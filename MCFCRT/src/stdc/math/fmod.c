// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_fpu.h"

#undef fmodf
#undef fmod
#undef fmodl

float fmodf(float x, float y){
	unsigned fsw;
	return (float)__MCFCRT_fmod(&fsw, x, y);
}
double fmod(double x, double y){
	unsigned fsw;
	return (double)__MCFCRT_fmod(&fsw, x, y);
}
long double fmodl(long double x, long double y){
	unsigned fsw;
	return __MCFCRT_fmod(&fsw, x, y);
}
