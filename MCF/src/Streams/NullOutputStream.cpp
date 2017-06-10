// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "NullOutputStream.hpp"

namespace MCF {

NullOutputStream::~NullOutputStream(){ }

void NullOutputStream::Put(unsigned char byData) noexcept {
	(void)byData;
}
void NullOutputStream::Put(const void *pData, std::size_t uSize) noexcept {
	(void)pData;
	(void)uSize;
}
void NullOutputStream::Flush(bool bHard) noexcept {
	(void)bHard;
}

}
