// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_NULL_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_NULL_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"

namespace MCF {

class NullOutputStream : public AbstractOutputStream {
public:
	NullOutputStream() noexcept = default;
	~NullOutputStream() override;

public:
	void Put(unsigned char byData) noexcept override;
	void Put(const void *pData, std::size_t uSize) noexcept override;
	void Flush(bool bHard) noexcept override;
};

}

#endif
