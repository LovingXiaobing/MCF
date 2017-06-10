// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BUFFERING_OUTPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_BUFFERING_OUTPUT_STREAM_FILTER_HPP_

#include "AbstractOutputStreamFilter.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

class BufferingOutputStreamFilter : public AbstractOutputStreamFilter {
private:
	Vector<unsigned char> x_vecBuffer;

public:
	explicit BufferingOutputStreamFilter(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept
		: AbstractOutputStreamFilter(std::move(pUnderlyingStream))
	{ }
	~BufferingOutputStreamFilter() override;

public:
	void Put(unsigned char byData) override;
	void Put(const void *pData, std::size_t uSize) override;
	void Flush(bool bHard) override;
};

}

#endif
