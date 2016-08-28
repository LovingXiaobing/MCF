// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "wcpcpy.h"
#include "expect.h"
#include "assert.h"
#include <emmintrin.h>

wchar_t *_MCFCRT_wcpcpy(wchar_t *restrict dst, const wchar_t *restrict src){
	register wchar_t *wp = dst;
	register const wchar_t *rp = src;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 31) != 0){
		const wchar_t rc = *rp;
		*wp = rc;
		if(rc == 0){
			return wp;
		}
		++rp;
		++wp;
	}
	const __m128i xz = _mm_setzero_si128();
	if(((uintptr_t)wp & 15) != 0){
		for(;;){
			const __m128i xw0 = _mm_load_si128((const __m128i *)rp);
			const __m128i xw1 = _mm_load_si128((const __m128i *)rp + 1);
			__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw0, xz), _mm_cmpeq_epi16(xw1, xz));
			unsigned mask = (unsigned)_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT(mask != 0)){
				while((mask & 1) == 0){
					const wchar_t rc = *rp;
					*wp = rc;
					++rp;
					++wp;
					mask >>= 1;
				}
				*wp = 0;
				return wp;
			}
			_mm_storeu_si128((__m128i *)wp, xw0);
			_mm_storeu_si128((__m128i *)wp + 1, xw1);
			rp += 16;
			wp += 16;
		}
	} else {
		for(;;){
			const __m128i xw0 = _mm_load_si128((const __m128i *)rp);
			const __m128i xw1 = _mm_load_si128((const __m128i *)rp + 1);
			__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw0, xz), _mm_cmpeq_epi16(xw1, xz));
			unsigned mask = (unsigned)_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT(mask != 0)){
				while((mask & 1) == 0){
					const wchar_t rc = *rp;
					*wp = rc;
					++rp;
					++wp;
					mask >>= 1;
				}
				*wp = 0;
				return wp;
			}
			_mm_storeu_si128((__m128i *)wp, xw0);
			_mm_storeu_si128((__m128i *)wp + 1, xw1);
			rp += 16;
			wp += 16;
		}
	}
}
wchar_t *_MCFCRT_wcppcpy(wchar_t *dst, wchar_t *end, const wchar_t *restrict src){
	_MCFCRT_ASSERT(dst + 1 <= end);
	register wchar_t *wp = dst;
	wchar_t *const wend = end - 1;
	register const wchar_t *rp = src;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 31) != 0){
		if(wp == wend){
			*wp = 0;
			return wp;
		}
		const wchar_t rc = *rp;
		*wp = rc;
		if(rc == 0){
			return wp;
		}
		++rp;
		++wp;
	}
	const __m128i xz = _mm_setzero_si128();
	if(((uintptr_t)wp & 15) != 0){
		for(;;){
			const __m128i xw0 = _mm_load_si128((const __m128i *)rp);
			const __m128i xw1 = _mm_load_si128((const __m128i *)rp + 1);
			__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw0, xz), _mm_cmpeq_epi16(xw1, xz));
			unsigned mask = (unsigned)_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT((mask != 0) || ((size_t)(wend - wp) < 16))){
				while(((mask & 1) == 0) && (wp != wend)){
					const wchar_t rc = *rp;
					*wp = rc;
					++rp;
					++wp;
					mask >>= 1;
				}
				*wp = 0;
				return wp;
			}
			_mm_storeu_si128((__m128i *)wp, xw0);
			_mm_storeu_si128((__m128i *)wp + 1, xw1);
			rp += 16;
			wp += 16;
		}
	} else {
		for(;;){
			const __m128i xw0 = _mm_load_si128((const __m128i *)rp);
			const __m128i xw1 = _mm_load_si128((const __m128i *)rp + 1);
			__m128i xt = _mm_packs_epi16(_mm_cmpeq_epi16(xw0, xz), _mm_cmpeq_epi16(xw1, xz));
			unsigned mask = (unsigned)_mm_movemask_epi8(xt);
			if(_MCFCRT_EXPECT_NOT((mask != 0) || ((size_t)(wend - wp) < 16))){
				while(((mask & 1) == 0) && (wp != wend)){
					const wchar_t rc = *rp;
					*wp = rc;
					++rp;
					++wp;
					mask >>= 1;
				}
				*wp = 0;
				return wp;
			}
			_mm_storeu_si128((__m128i *)wp, xw0);
			_mm_storeu_si128((__m128i *)wp + 1, xw1);
			rp += 16;
			wp += 16;
		}
	}
}
