// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "BufferingOutputStreamFilter.hpp"

namespace MCF {

BufferingOutputStreamFilter::~BufferingOutputStreamFilter(){
}

void BufferingOutputStreamFilter::Put(unsigned char byData){
	y_vStream.BufferedPut(byData);
	y_vStream.Flush(y_vStream.kFlushBufferAuto);
}
void BufferingOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	y_vStream.BufferedPut(pData, uSize);
	y_vStream.Flush(y_vStream.kFlushBufferAuto);
}
void BufferingOutputStreamFilter::Flush(bool bHard){
	y_vStream.Flush(bHard);
}

}
