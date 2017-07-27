// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARRAY_HPP_
#define MCF_CORE_ARRAY_HPP_

#include "ArrayView.hpp"
#include "Assert.hpp"
#include "Exception.hpp"
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

namespace Impl_Array {
	template<typename ElementT, std::size_t kSizeT>
	struct NoexceptSwappableChecker {
		constexpr bool operator()() const noexcept {
			using std::swap;
			return noexcept(swap(std::declval<ElementT (&)[kSizeT]>(), std::declval<ElementT (&)[kSizeT]>()));
		}
	};
}

template<typename ElementT, std::size_t kSizeT, std::size_t ...kRemainingT>
class Array : public Array<Array<ElementT, kRemainingT...>, kSizeT> {
	static_assert(kSizeT > 0, "An array shall have m_a non-zero size.");
};

template<typename ElementT, std::size_t kSizeT>
class Array<ElementT, kSizeT> {
	static_assert(kSizeT > 0, "An array shall have m_a non-zero size.");

public:
	ElementT m_a[kSizeT];

public:
	// 整体仿造容器，唯独没有 Clear()。
	using Element         = ElementT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <Array>;
	using Enumerator      = Impl_Enumerator::Enumerator      <Array>;

	using ConstView       = ArrayView<const Element>;
	using View            = ArrayView<      Element>;

	constexpr bool IsEmpty() const noexcept {
		return false;
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		for(std::size_t uIndex = 0; uIndex < kSizeT; ++uIndex){
			*itOutput = std::move(m_a[uIndex]);
			++itOutput;
		}
		return itOutput;
	}

	constexpr const Element *GetFirst() const noexcept {
		return GetBegin();
	}
	Element *GetFirst() noexcept {
		return GetBegin();
	}
	constexpr const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	constexpr const Element *GetLast() const noexcept {
		return GetEnd() - 1;
	}
	Element *GetLast() noexcept {
		return GetEnd() - 1;
	}
	constexpr const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == kSizeT){
			return nullptr;
		}
		return pBegin + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		MCF_DEBUG_CHECK(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == kSizeT){
			return nullptr;
		}
		return pBegin + uOffset;
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		return ConstEnumerator(*this, GetFirst());
	}
	Enumerator EnumerateFirst() noexcept {
		return Enumerator(*this, GetFirst());
	}
	ConstEnumerator EnumerateConstFirst() const noexcept {
		return EnumerateFirst();
	}
	ConstEnumerator EnumerateLast() const noexcept {
		return ConstEnumerator(*this, GetLast());
	}
	Enumerator EnumerateLast() noexcept {
		return Enumerator(*this, GetLast());
	}
	ConstEnumerator EnumerateConstLast() const noexcept {
		return EnumerateLast();
	}
	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}
	constexpr ConstEnumerator EnumerateConstSingular() const noexcept {
		return EnumerateSingular();
	}

	void Swap(Array &aOther) noexcept(Impl_Array::NoexceptSwappableChecker<Element, kSizeT>()()) {
		using std::swap;
		swap(m_a, aOther.m_a);
	}

	// Array 需求。
	const Element *GetData() const noexcept {
		return m_a;
	}
	Element *GetData() noexcept {
		return m_a;
	}
	static constexpr std::size_t GetSize() noexcept {
		return kSizeT;
	}

	const Element *GetBegin() const noexcept {
		return m_a;
	}
	Element *GetBegin() noexcept {
		return m_a;
	}
	const Element *GetConstBegin() const noexcept {
		return GetBegin();
	}
	const Element *GetEnd() const noexcept {
		return m_a + kSizeT;
	}
	Element *GetEnd() noexcept {
		return m_a + kSizeT;
	}
	const Element *GetConstEnd() const noexcept {
		return GetEnd();
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= kSizeT){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Array: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= kSizeT){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Array: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex < kSizeT);

		return m_a[uIndex];
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		MCF_DEBUG_CHECK(uIndex < kSizeT);

		return m_a[uIndex];
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

public:
	ConstView GetView() const noexcept {
		return ConstView(GetData(), GetSize());
	}
	View GetView() noexcept {
		return View(GetData(), GetSize());
	}

public:
	operator ConstView() const noexcept {
		return GetView();
	}
	operator View() noexcept {
		return GetView();
	}

	explicit operator const Element *() const noexcept {
		return GetData();
	}
	explicit operator Element *() noexcept {
		return GetData();
	}

	friend void swap(Array &aSelf, Array &aOther) noexcept(noexcept(aSelf.Swap(aOther))) {
		aSelf.Swap(aOther);
	}

	friend decltype(auto) begin(const Array &aOther) noexcept {
		return aOther.EnumerateFirst();
	}
	friend decltype(auto) begin(Array &aOther) noexcept {
		return aOther.EnumerateFirst();
	}
	friend decltype(auto) cbegin(const Array &aOther) noexcept {
		return begin(aOther);
	}
	friend decltype(auto) end(const Array &aOther) noexcept {
		return aOther.EnumerateSingular();
	}
	friend decltype(auto) end(Array &aOther) noexcept {
		return aOther.EnumerateSingular();
	}
	friend decltype(auto) cend(const Array &aOther) noexcept {
		return end(aOther);
	}
};

}

#endif
