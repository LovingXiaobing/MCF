// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_HEX_INPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_HEX_INPUT_STREAM_FILTER_HPP_

#include "AbstractInputStreamFilter.hpp"

namespace MCF {

class HexInputStreamFilter : public AbstractInputStreamFilter {
private:
	StreamBuffer x_sbufPlain;

public:
	explicit HexInputStreamFilter(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
		: AbstractInputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~HexInputStreamFilter() override;

	HexInputStreamFilter(HexInputStreamFilter &&) noexcept = default;
	HexInputStreamFilter &operator=(HexInputStreamFilter &&) noexcept = default;

private:
	void X_PopulatePlainBuffer(std::size_t uExpected);

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;
	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	void Swap(HexInputStreamFilter &rhs) noexcept {
		AbstractInputStreamFilter::Y_Swap(rhs);
		using std::swap;
		swap(x_sbufPlain, rhs.x_sbufPlain);
	}

public:
	friend void swap(HexInputStreamFilter &lhs, HexInputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
