// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.h"
#include "module.h"

int atexit(void (*func)(void)){
	if(!_MCFCRT_AtModuleExit((void (*__attribute__((__cdecl__)))(intptr_t))func, 0)){
		return -1;
	}
	return 0;
}
