// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "_fpu.h"

#define RND_NEAREST     (0ul)            // 四舍六入五凑双。
#define RND_DOWN        (1ul)            // 向负无穷舍入。
#define RND_UP          (2ul)            // 向正无穷舍入。
#define RND_ZERO        (3ul)            // 向零舍入。

#define PRCS_SINGLE     (0ul)            // 24 位。
#define PRCS_DOUBLE     (2ul)            // 53 位。
#define PRCS_EXTENDED   (3ul)            // 64 位。

#define EXCEPT_PM       (1ul << 5)       // 精度损失异常。
#define EXCEPT_UM       (1ul << 4)       // 下溢异常。
#define EXCEPT_OM       (1ul << 3)       // 上溢异常。
#define EXCEPT_ZM       (1ul << 2)       // 除以零异常。
#define EXCEPT_DM       (1ul << 1)       // 非规格化数异常。
#define EXCEPT_IM       (1ul << 0)       // 无效操作异常。

#define ROUNDING        (RND_NEAREST)
#define PRECISION       (PRCS_EXTENDED)
#define EXCEPT_MASK     (EXCEPT_PM | EXCEPT_UM | EXCEPT_DM)

static const uint16_t kFpCsr = (ROUNDING << 10) | (PRECISION << 8) | (EXCEPT_MASK << 0);
static const uint32_t kMxCsr = (ROUNDING << 13) |                    (EXCEPT_MASK << 7);

void __MCFCRT_FpuInitialize(void){
	__asm__ volatile (
		"fnclex \n"
		"fldcw %0 \n"
		"ldmxcsr %1 \n"
		:
		: "m"(kFpCsr), "m"(kMxCsr)
	);
}
