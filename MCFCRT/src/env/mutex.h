// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_MUTEX_H_
#define __MCFCRT_ENV_MUTEX_H_

#include "_crtdef.h"

#ifndef __MCFCRT_MUTEX_INLINE_OR_EXTERN
#	define __MCFCRT_MUTEX_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

// 初始化为 { 0 } 即可。
typedef struct __MCFCRT_tagMutex {
	volatile _MCFCRT_STD uintptr_t __u;
} _MCFCRT_Mutex;

#define _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT   100u

__MCFCRT_MUTEX_INLINE_OR_EXTERN void _MCFCRT_InitializeMutex(_MCFCRT_Mutex *__pMutex) _MCFCRT_NOEXCEPT {
	__atomic_store_n(&(__pMutex->__u), 0, __ATOMIC_RELEASE);
}

extern bool _MCFCRT_WaitForMutex(_MCFCRT_Mutex *__pMutex, _MCFCRT_STD size_t __uMaxSpinCount, _MCFCRT_STD uint64_t __u64UntilFastMonoClock) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_WaitForMutexForever(_MCFCRT_Mutex *__pMutex, _MCFCRT_STD size_t __uMaxSpinCount) _MCFCRT_NOEXCEPT;
extern void _MCFCRT_SignalMutex(_MCFCRT_Mutex *__pMutex) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
