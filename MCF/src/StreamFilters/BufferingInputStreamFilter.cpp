// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "BufferingInputStreamFilter.hpp"

namespace MCF {

BufferingInputStreamFilter::~BufferingInputStreamFilter(){
}

int BufferingInputStreamFilter::Peek(){
	y_vStream.PopulateBuffer(1);
	return y_vStream.BufferedPeek();
}
int BufferingInputStreamFilter::Get(){
	y_vStream.PopulateBuffer(1);
	return y_vStream.BufferedGet();
}
bool BufferingInputStreamFilter::Discard(){
	y_vStream.PopulateBuffer(1);
	return y_vStream.BufferedDiscard();
}
std::size_t BufferingInputStreamFilter::Peek(void *pData, std::size_t uSize){
	y_vStream.PopulateBuffer(uSize);
	return y_vStream.BufferedPeek(pData, uSize);
}
std::size_t BufferingInputStreamFilter::Get(void *pData, std::size_t uSize){
	y_vStream.PopulateBuffer(uSize);
	return y_vStream.BufferedGet(pData, uSize);
}
std::size_t BufferingInputStreamFilter::Discard(std::size_t uSize){
	y_vStream.PopulateBuffer(uSize);
	return y_vStream.BufferedDiscard(uSize);
}

}
