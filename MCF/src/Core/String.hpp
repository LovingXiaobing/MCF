// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_HPP_
#define MCF_CORE_STRING_HPP_

#include "StringView.hpp"
#include "_CheckedSizeArithmetic.hpp"
#include "_Enumerator.hpp"
#include "Assert.hpp"
#include "CountOf.hpp"
#include "CopyMoveFill.hpp"
#include <type_traits>
#include <cstring>
#include <cstddef>
#include <cstdint>

// 借用了 https://github.com/elliotgoodrich/SSO-23 的一些想法。然而我们可以优化更多……
// 我们支持的最大 SSO 长度是 31 个“字符”，而不是 23 个“字节”。

namespace MCF {

template<StringType kTypeT>
class String;

namespace Impl_String {
	template<StringType kSrcTypeT>
	struct Transcoder;
}

template<StringType kTypeT>
class String {
public:
	using View   = StringView<kTypeT>;
	using Traits = typename View::Traits;
	using Char   = typename Traits::Char;

	enum : std::size_t {
		kNpos = View::kNpos,
	};

	// 容器需求。
	using Element         = Char;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <String>;
	using Enumerator      = Impl_Enumerator::Enumerator      <String>;

public:
	static void UnifyAppend(String<StringType::kUtf16> &u16sDst, const View &svSrc);
	static void DeunifyAppend(String &strDst, const StringView<StringType::kUtf16> &u16svSrc);

	static void UnifyAppend(String<StringType::kUtf32> &u32sDst, const View &svSrc);
	static void DeunifyAppend(String &strDst, const StringView<StringType::kUtf32> &u32svSrc);

private:
	__extension__ union {
		struct {
			Char achData[31];
			std::make_signed_t<Char> schComplLength;
		};
		struct {
			Char *pchBegin;
			std::size_t uLength;
			std::size_t uSizeAllocated;
		};
	} x_vStorage;

private:
	std::size_t X_GetSmallLength() const noexcept {
		return CountOf(x_vStorage.achData) - static_cast<std::make_unsigned_t<Char>>(x_vStorage.schComplLength);
	}
	void X_SetSmallLength(std::size_t uLength) noexcept {
		x_vStorage.schComplLength = static_cast<std::make_signed_t<Char>>(CountOf(x_vStorage.achData) - uLength);
	}

public:
	String() noexcept {
		x_vStorage.achData[0]     = static_cast<Char>(0xDEADBEEF);
		x_vStorage.schComplLength = static_cast<std::make_signed_t<Char>>(CountOf(x_vStorage.achData));
	}
	explicit String(Char ch, std::size_t uCount = 1)
		: String()
	{
		Append(ch, uCount);
	}
	explicit String(const Char *pszBegin)
		: String()
	{
		Append(pszBegin);
	}
	String(const Char *pchBegin, const Char *pchEnd)
		: String()
	{
		Append(pchBegin, pchEnd);
	}
	String(const Char *pchBegin, std::size_t uLen)
		: String()
	{
		Append(pchBegin, uLen);
	}
	explicit String(const View &rhs)
		: String()
	{
		Append(rhs);
	}
	String(std::initializer_list<Char> rhs)
		: String()
	{
		Append(rhs);
	}
	template<StringType kOtherTypeT>
	explicit String(const StringView<kOtherTypeT> &rhs)
		: String()
	{
		Append(rhs);
	}
	template<StringType kOtherTypeT>
	explicit String(const String<kOtherTypeT> &rhs)
		: String()
	{
		Append(rhs);
	}
	String(const String &rhs)
		: String()
	{
		Append(rhs);
	}
	String(String &&rhs) noexcept
		: String()
	{
		Swap(rhs);
	}
	String &operator=(Char ch){
		Assign(ch, 1);
		return *this;
	}
	String &operator=(const Char *pszBegin){
		Assign(pszBegin);
		return *this;
	}
	String &operator=(const View &rhs){
		Assign(rhs);
		return *this;
	}
	template<StringType kOtherTypeT>
	String &operator=(const StringView<kOtherTypeT> &rhs){
		Assign(rhs);
		return *this;
	}
	template<StringType kOtherTypeT>
	String &operator=(const String<kOtherTypeT> &rhs){
		Assign(rhs);
		return *this;
	}
	String &operator=(const String &rhs){
		Assign(rhs);
		return *this;
	}
	String &operator=(String &&rhs) noexcept {
		Assign(std::move(rhs));
		return *this;
	}
	~String() noexcept {
		if(x_vStorage.schComplLength < 0){
			::operator delete[](x_vStorage.pchBegin);
		}
#ifndef NDEBUG
		std::memset(&x_vStorage, 0xDD, sizeof(x_vStorage));
#endif
	}

private:
	Char *X_ChopAndSplice(std::size_t uRemovedBegin, std::size_t uRemovedEnd, std::size_t uFirstOffset, std::size_t uThirdOffset){
		const auto pchOldBuffer = GetBegin();
		const auto uOldSize = GetSize();
		auto pchNewBuffer = pchOldBuffer;
		const auto uNewSize = uThirdOffset + (uOldSize - uRemovedEnd);
		auto uCharsToAlloc = uNewSize + 1;

		MCF_DEBUG_CHECK(uRemovedBegin <= uOldSize);
		MCF_DEBUG_CHECK(uRemovedEnd <= uOldSize);
		MCF_DEBUG_CHECK(uRemovedBegin <= uRemovedEnd);
		MCF_DEBUG_CHECK(uFirstOffset + uRemovedBegin <= uThirdOffset);

		const auto uOldCapacity = GetCapacity();
		if(uOldCapacity < uNewSize){
			uCharsToAlloc += (uCharsToAlloc >> 1);
			uCharsToAlloc = (uCharsToAlloc + 0x0F) & static_cast<std::size_t>(-0x10);
			if(uCharsToAlloc < uNewSize + 1){
				uCharsToAlloc = uNewSize + 1;
			}
			const auto uBytesToAlloc = Impl_CheckedSizeArithmetic::Mul(sizeof(Char), uCharsToAlloc);
			pchNewBuffer = (Char *)::operator new[](uBytesToAlloc);
		}

		if((pchNewBuffer + uFirstOffset != pchOldBuffer) && (uRemovedBegin != 0)){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(Char));
		}
		if((pchNewBuffer + uThirdOffset != pchOldBuffer + uRemovedEnd) && (uOldSize != uRemovedEnd)){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldSize - uRemovedEnd) * sizeof(Char));
		}

		if(pchNewBuffer != pchOldBuffer){
			if(x_vStorage.schComplLength >= 0){
				x_vStorage.schComplLength = -1;
			} else {
				::operator delete[](pchOldBuffer);
			}

			x_vStorage.pchBegin = pchNewBuffer;
			x_vStorage.uLength = uOldSize;
			x_vStorage.uSizeAllocated = uCharsToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	}
	void X_SetSize(std::size_t uNewSize) noexcept {
		MCF_DEBUG_CHECK(uNewSize <= GetCapacity());

		if(x_vStorage.schComplLength >= 0){
			X_SetSmallLength(uNewSize);
		} else {
			x_vStorage.uLength = uNewSize;
		}
	}

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		X_SetSize(0);
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			const auto pBegin = GetBegin();
			const auto pEnd = GetEnd();
			for(auto p = pBegin; p != pEnd; ++p){
				*itOutput = *p;
				++itOutput;
			}
		} catch(...){
			Clear();
			throw;
		}
		Clear();
		return itOutput;
	}

	const Element *GetFirst() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	Element *GetFirst() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	Element *GetLast() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		return pBegin + uOffset - 1;
	}
	Element *GetPrev(const Element *pPos) noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		return pBegin + uOffset - 1;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset + 1 == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset + 1;
	}
	Element *GetNext(const Element *pPos) noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset + 1 == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset + 1;
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

	void Swap(String &rhs) noexcept {
		using std::swap;
		swap(x_vStorage, rhs.x_vStorage);
	}

	// String 需求。
	const Char *GetBegin() const noexcept {
		if(x_vStorage.schComplLength >= 0){
			return x_vStorage.achData;
		} else {
			return x_vStorage.pchBegin;
		}
	}
	Char *GetBegin() noexcept {
		if(x_vStorage.schComplLength >= 0){
			return x_vStorage.achData;
		} else {
			return x_vStorage.pchBegin;
		}
	}

	const Char *GetEnd() const noexcept {
		if(x_vStorage.schComplLength >= 0){
			return x_vStorage.achData + X_GetSmallLength();
		} else {
			return x_vStorage.pchBegin + x_vStorage.uLength;
		}
	}
	Char *GetEnd() noexcept {
		if(x_vStorage.schComplLength >= 0){
			return x_vStorage.achData + X_GetSmallLength();
		} else {
			return x_vStorage.pchBegin + x_vStorage.uLength;
		}
	}
	std::size_t GetSize() const noexcept {
		if(x_vStorage.schComplLength >= 0){
			return X_GetSmallLength();
		} else {
			return x_vStorage.uLength;
		}
	}

	const Char *GetData() const noexcept {
		return GetBegin();
	}
	Char *GetData() noexcept {
		return GetBegin();
	}
	const Char *GetConstData() const noexcept {
		return GetData();
	}
	const Char *GetStr() const noexcept {
		if(x_vStorage.schComplLength >= 0){
			const auto &chTerminator = x_vStorage.achData[X_GetSmallLength()];
			if(chTerminator != Char()){
				const_cast<Char &>(chTerminator) = Char();
			}
			return x_vStorage.achData;
		} else {
			auto &chTerminator = x_vStorage.pchBegin[x_vStorage.uLength];
			if(chTerminator != Char()){
				const_cast<Char &>(chTerminator) = Char();
			}
			return x_vStorage.pchBegin;
		}
	}
	Char *GetStr() noexcept {
		if(x_vStorage.schComplLength >= 0){
			x_vStorage.achData[X_GetSmallLength()] = Char();
			return x_vStorage.achData;
		} else {
			x_vStorage.pchBegin[x_vStorage.uLength] = Char();
			return x_vStorage.pchBegin;
		}
	}
	const Char *GetConstStr() const noexcept {
		return GetStr();
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	const Char &Get(std::size_t uIndex) const {
		if(uIndex > GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"String: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	Char &Get(std::size_t uIndex){
		if(uIndex > GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"String: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	const Char &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex <= GetSize());

		return GetStr()[uIndex];
	}
	Char &UncheckedGet(std::size_t uIndex) noexcept {
		MCF_DEBUG_CHECK(uIndex <= GetSize());

		return GetStr()[uIndex];
	}

	View GetView() const noexcept {
		if(x_vStorage.schComplLength >= 0){
			return View(x_vStorage.achData, X_GetSmallLength());
		} else {
			return View(x_vStorage.pchBegin, x_vStorage.uLength);
		}
	}

	std::size_t GetCapacity() const noexcept {
		if(x_vStorage.schComplLength >= 0){
			return CountOf(x_vStorage.achData);
		} else {
			return x_vStorage.uSizeAllocated - 1;
		}
	}
	std::size_t GetCapacityRemaining() const noexcept {
		return GetCapacity() - GetSize();
	}
	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity <= uOldCapacity){
			return;
		}
		const auto uOldSize = GetSize();
		X_ChopAndSplice(uOldSize, uOldSize, 0, uNewCapacity);
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = GetSize();
		const auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaCapacity, uOldSize);
		Reserve(uNewCapacity);
	}

	Char *Resize(std::size_t uNewSize){
		const auto uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Reserve(uNewSize);
			X_SetSize(uNewSize);
		} else if(uNewSize < uOldSize){
			Pop(uOldSize - uNewSize);
		}
		return GetData();
	}
	Char *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		const auto uNewSize = Impl_CheckedSizeArithmetic::Add(uDeltaSize, uOldSize);
		X_ChopAndSplice(uOldSize, uOldSize, 0, uNewSize);
		X_SetSize(uNewSize);
		return GetData() + uOldSize;
	}
	std::pair<Char *, std::size_t> ResizeToCapacity() noexcept {
		const auto uOldSize = GetSize();
		const auto uNewSize = GetCapacity();
		const auto uDeltaSize = uNewSize - uOldSize;
		X_SetSize(uNewSize);
		return std::make_pair(GetData() + uOldSize, uDeltaSize);
	}
	void ShrinkAsZeroTerminated() noexcept {
		const auto uSzLen = View(GetStr()).GetSize();
		MCF_DEBUG_CHECK(uSzLen <= GetSize());
		X_SetSize(uSzLen);
	}

	int Compare(const View &rhs) const noexcept {
		return GetView().Compare(rhs);
	}
	int Compare(const String &rhs) const noexcept {
		return GetView().Compare(rhs.GetView());
	}

	void Assign(Char ch, std::size_t uCount = 1){
		Resize(uCount);
		FillN(GetData(), uCount, ch);
	}
	void Assign(const Char *pszBegin){
		Assign(View(pszBegin));
	}
	void Assign(const Char *pchBegin, const Char *pchEnd){
		Assign(View(pchBegin, pchEnd));
	}
	void Assign(const Char *pchBegin, std::size_t uCount){
		Assign(View(pchBegin, uCount));
	}
	void Assign(const View &rhs){
		Resize(rhs.GetSize());
		Copy(GetData(), rhs.GetBegin(), rhs.GetEnd());
	}
	void Assign(std::initializer_list<Char> rhs){
		Assign(View(rhs));
	}
	template<StringType kOtherTypeT>
	void Assign(const StringView<kOtherTypeT> &rhs){
		const auto uOldSize = GetSize();
		Append(rhs);
		if(uOldSize != 0){
			const auto pchData = GetData();
			const auto uNewSize = GetSize();
			Copy(pchData, pchData + uOldSize, pchData + uNewSize);
			Pop(uOldSize);
		}
	}
	template<StringType kOtherTypeT>
	void Assign(const String<kOtherTypeT> &rhs){
		Assign(StringView<kOtherTypeT>(rhs));
	}
	void Assign(const String &rhs){
		if(&rhs != this){
			Assign(View(rhs));
		}
	}
	void Assign(String &&rhs) noexcept {
		MCF_DEBUG_CHECK(this != &rhs);

		if(x_vStorage.schComplLength < 0){
			::operator delete[](x_vStorage.pchBegin);
		}
		x_vStorage = rhs.x_vStorage;
#ifndef NDEBUG
		std::memset(rhs.x_vStorage.achData, 0xCD, sizeof(rhs.x_vStorage.achData));
#endif
		rhs.x_vStorage.schComplLength = 0;
	}

	void Push(Char ch){
		Append(ch, 1);
	}
	void UncheckedPush(Char ch) noexcept {
		MCF_DEBUG_CHECK(GetSize() < GetCapacity());

		if(x_vStorage.schComplLength >= 0){
			x_vStorage.achData[X_GetSmallLength()] = ch;
			--x_vStorage.schComplLength;
		} else {
			x_vStorage.pchBegin[x_vStorage.uLength] = ch;
			++x_vStorage.uLength;
		}
	}
	void Pop(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		MCF_DEBUG_CHECK(uOldSize >= uCount);
		X_SetSize(uOldSize - uCount);
	}

	void Append(Char ch, std::size_t uCount = 1){
		FillN(ResizeMore(uCount), uCount, ch);
	}
	void Append(const Char *pszBegin){
		Append(View(pszBegin));
	}
	void Append(const Char *pchBegin, const Char *pchEnd){
		Append(View(pchBegin, pchEnd));
	}
	void Append(const Char *pchBegin, std::size_t uCount){
		Append(View(pchBegin, uCount));
	}
	void Append(const View &rhs){
		const auto pWrite = ResizeMore(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd());
	}
	void Append(std::initializer_list<Char> rhs){
		Append(View(rhs));
	}
	void Append(const String &rhs){
		const auto pWrite = ResizeMore(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd()); // 这是正确的即使对于 &rhs == this 的情况。
	}
	template<StringType kOtherTypeT>
	void Append(const StringView<kOtherTypeT> &rhs){
		Impl_String::Transcoder<kOtherTypeT>()(*this, rhs);
	}
	template<StringType kOtherTypeT>
	void Append(const String<kOtherTypeT> &rhs){
		Append(rhs.GetView());
	}

	View Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().Slice(nBegin, nEnd);
	}
	String SliceStr(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const {
		return String(Slice(nBegin, nEnd));
	}

	void Reverse() noexcept {
		auto pchBegin = GetBegin();
		auto pchEnd = GetEnd();
		if(pchBegin != pchEnd){
			--pchEnd;
			while(pchBegin < pchEnd){
				using std::swap;
				swap(*pchBegin, *pchEnd);
				++pchBegin;
				--pchEnd;
			}
		}
	}

	std::size_t Find(const View &vToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().Find(vToFind, nBegin);
	}
	std::size_t FindBackward(const View &vToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindBackward(vToFind, nEnd);
	}
	std::size_t FindRep(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().FindRep(chToFind, uRepCount, nBegin);
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindRepBackward(chToFind, uRepCount, nEnd);
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().Find(chToFind, nBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindBackward(chToFind, nEnd);
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char chChar, std::size_t uCount = 1){
		const auto vCurrent = GetView();
		const auto uOldSize = vCurrent.GetSize();

		const auto vRemoved = vCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = static_cast<std::size_t>(vRemoved.GetBegin() - vCurrent.GetBegin());
		const auto uRemovedEnd = static_cast<std::size_t>(vRemoved.GetEnd() - vCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldSize - (uRemovedEnd - uRemovedBegin);
		const auto uNewSize = Impl_CheckedSizeArithmetic::Add(uCount, uLengthAfterRemoved);

		const auto pchWrite = X_ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uCount);
		FillN(pchWrite, uCount, chChar);
		X_SetSize(uNewSize);
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchReplacementBegin){
		Replace(nBegin, nEnd, View(pchReplacementBegin));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchReplacementBegin, const Char *pchReplacementEnd){
		Replace(nBegin, nEnd, View(pchReplacementBegin, pchReplacementEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchReplacementBegin, std::size_t uLen){
		Replace(nBegin, nEnd, View(pchReplacementBegin, uLen));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const View &vReplacement){
		const auto uReplacementSize = vReplacement.GetSize();

		const auto vCurrent = GetView();
		const auto uOldSize = vCurrent.GetSize();

		const auto vRemoved = vCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = static_cast<std::size_t>(vRemoved.GetBegin() - vCurrent.GetBegin());
		const auto uRemovedEnd = static_cast<std::size_t>(vRemoved.GetEnd() - vCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldSize - (uRemovedEnd - uRemovedBegin);
		const auto uNewSize = Impl_CheckedSizeArithmetic::Add(uReplacementSize, uLengthAfterRemoved);

		if(vCurrent.DoesOverlapWith(vReplacement)){
			String strTemp;
			strTemp.Resize(uNewSize);
			auto pchWrite = strTemp.GetData();
			pchWrite = Copy(pchWrite, vCurrent.GetBegin(), vCurrent.GetBegin() + uRemovedBegin);
			pchWrite = Copy(pchWrite, vReplacement.GetBegin(), vReplacement.GetEnd());
			pchWrite = Copy(pchWrite, vCurrent.GetBegin() + uRemovedEnd, vCurrent.GetEnd());
			Assign(std::move(strTemp));
		} else {
			const auto pchWrite = X_ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + vReplacement.GetSize());
			CopyN(pchWrite, vReplacement.GetBegin(), vReplacement.GetSize());
			X_SetSize(uNewSize);
		}
	}

public:
	operator View() const noexcept {
		return GetView();
	}

	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator const Char *() const noexcept {
		return GetStr();
	}
	explicit operator Char *() noexcept {
		return GetStr();
	}
	const Char &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Char &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

	template<StringType kOtherTypeT>
	String &operator+=(const String<kOtherTypeT> &rhs){
		Append(rhs);
		return *this;
	}
	template<StringType kOtherTypeT>
	String &operator+=(const StringView<kOtherTypeT> &rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(Char rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(const Char *rhs){
		Append(rhs);
		return *this;
	}
	template<StringType kOtherTypeT>
	friend String operator+(const String &lhs, const StringView<kOtherTypeT> &rhs){
		auto temp = lhs;
		temp += rhs;
		return temp;
	}
	template<StringType kOtherTypeT>
	friend String operator+(String &&lhs, const StringView<kOtherTypeT> &rhs){
		lhs += rhs;
		return std::move(lhs);
	}
	friend String operator+(const String &lhs, Char rhs){
		auto temp = lhs;
		temp += rhs;
		return temp;
	}
	friend String operator+(String &&lhs, Char rhs){
		lhs += rhs;
		return std::move(lhs);
	}
	friend String operator+(const String &lhs, const Char *rhs){
		auto temp = lhs;
		temp += rhs;
		return temp;
	}
	friend String operator+(String &&lhs, const Char *rhs){
		lhs += rhs;
		return std::move(lhs);
	}
	friend String operator+(Char lhs, const String &rhs){
		auto temp = String(lhs);
		temp += rhs;
		return temp;
	}
	friend String operator+(Char lhs, String &&rhs){
		auto temp = String(lhs);
		temp += std::move(rhs);
		return temp;
	}
	friend String operator+(const Char *lhs, const String &rhs){
		auto temp = String(lhs);
		temp += rhs;
		return temp;
	}
	friend String operator+(const Char *lhs, String &&rhs){
		auto temp = String(lhs);
		temp += std::move(rhs);
		return temp;
	}

	bool operator==(const String &rhs) const noexcept {
		return GetView() == rhs.GetView();
	}
	bool operator==(const View &rhs) const noexcept {
		return GetView() == rhs;
	}
	friend bool operator==(const View &lhs, const String &rhs) noexcept {
		return lhs == rhs.GetView();
	}

	bool operator!=(const String &rhs) const noexcept {
		return GetView() != rhs.GetView();
	}
	bool operator!=(const View &rhs) const noexcept {
		return GetView() != rhs;
	}
	friend bool operator!=(const View &lhs, const String &rhs) noexcept {
		return lhs != rhs.GetView();
	}

	bool operator<(const String &rhs) const noexcept {
		return GetView() < rhs.GetView();
	}
	bool operator<(const View &rhs) const noexcept {
		return GetView() < rhs;
	}
	friend bool operator<(const View &lhs, const String &rhs) noexcept {
		return lhs < rhs.GetView();
	}

	bool operator>(const String &rhs) const noexcept {
		return GetView() > rhs.GetView();
	}
	bool operator>(const View &rhs) const noexcept {
		return GetView() > rhs;
	}
	friend bool operator>(const View &lhs, const String &rhs) noexcept {
		return lhs > rhs.GetView();
	}

	bool operator<=(const String &rhs) const noexcept {
		return GetView() <= rhs.GetView();
	}
	bool operator<=(const View &rhs) const noexcept {
		return GetView() <= rhs;
	}
	friend bool operator<=(const View &lhs, const String &rhs) noexcept {
		return lhs <= rhs.GetView();
	}

	bool operator>=(const String &rhs) const noexcept {
		return GetView() >= rhs.GetView();
	}
	bool operator>=(const View &rhs) const noexcept {
		return GetView() >= rhs;
	}
	friend bool operator>=(const View &lhs, const String &rhs) noexcept {
		return lhs >= rhs.GetView();
	}

	friend void swap(String &lhs, String &rhs) noexcept {
		lhs.Swap(rhs);
	}

	friend decltype(auto) begin(const String &rhs) noexcept {
		return rhs.GetBegin();
	}
	friend decltype(auto) begin(String &rhs) noexcept {
		return rhs.GetBegin();
	}
	friend decltype(auto) cbegin(const String &rhs) noexcept {
		return begin(rhs);
	}
	friend decltype(auto) end(const String &rhs) noexcept {
		return rhs.GetEnd();
	}
	friend decltype(auto) end(String &rhs) noexcept {
		return rhs.GetEnd();
	}
	friend decltype(auto) cend(const String &rhs) noexcept {
		return end(rhs);
	}
};

namespace Impl_String {
	static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t does not have the same size with char16_t.");
	static_assert(alignof(wchar_t) == alignof(char16_t), "wchar_t does not have the same alignment with char16_t.");

	template<StringType kSrcTypeT>
	struct Transcoder {
		template<StringType kDstTypeT>
		void operator()(String<kDstTypeT> &strDst, const StringView<kSrcTypeT> &svSrc) const {
			using DstTraits = StringEncodingTraits<kDstTypeT>;
			using SrcTraits = StringEncodingTraits<kSrcTypeT>;
			using UnifiedString = String<(DstTraits::kConversionPreference + SrcTraits::kConversionPreference < 0) ? StringType::kUtf16 : StringType::kUtf32>;

			UnifiedString usTemp;
			String<kSrcTypeT>::UnifyAppend(usTemp, svSrc);
			String<kDstTypeT>::DeunifyAppend(strDst, usTemp);
		}

		void operator()(String<StringType::kWide> &wsDst, const StringView<kSrcTypeT> &svSrc) const {
			String<kSrcTypeT>::UnifyAppend(reinterpret_cast<String<StringType::kUtf16> &>(wsDst), svSrc);
		}
		void operator()(String<StringType::kUtf16> &u16sDst, const StringView<kSrcTypeT> &svSrc) const {
			String<kSrcTypeT>::UnifyAppend(u16sDst, svSrc);
		}
		void operator()(String<StringType::kUtf32> &u32sDst, const StringView<kSrcTypeT> &svSrc) const {
			String<kSrcTypeT>::UnifyAppend(u32sDst, svSrc);
		}
	};

	template<>
	struct Transcoder<StringType::kWide> {
		template<StringType kDstTypeT>
		void operator()(String<kDstTypeT> &strDst, const StringView<StringType::kWide> &wsvSrc) const {
			String<kDstTypeT>::DeunifyAppend(strDst, reinterpret_cast<const StringView<StringType::kUtf16> &>(wsvSrc));
		}
	};
	template<>
	struct Transcoder<StringType::kUtf16> {
		template<StringType kDstTypeT>
		void operator()(String<kDstTypeT> &strDst, const StringView<StringType::kUtf16> &u16svSrc) const {
			String<kDstTypeT>::DeunifyAppend(strDst, u16svSrc);
		}
	};
	template<>
	struct Transcoder<StringType::kUtf32> {
		template<StringType kDstTypeT>
		void operator()(String<kDstTypeT> &strDst, const StringView<StringType::kUtf32> &u32svSrc) const {
			String<kDstTypeT>::DeunifyAppend(strDst, u32svSrc);
		}
	};
}

extern template class String<StringType::kUtf8>;
extern template class String<StringType::kUtf16>;
extern template class String<StringType::kUtf32>;
extern template class String<StringType::kCesu8>;
extern template class String<StringType::kAnsi>;
extern template class String<StringType::kModifiedUtf8>;
extern template class String<StringType::kNarrow>;
extern template class String<StringType::kWide>;

using Utf8String         = String<StringType::kUtf8>;
using Utf16String        = String<StringType::kUtf16>;
using Utf32String        = String<StringType::kUtf32>;
using Cesu8String        = String<StringType::kCesu8>;
using AnsiString         = String<StringType::kAnsi>;
using ModifiedUtf8String = String<StringType::kModifiedUtf8>;
using NarrowString       = String<StringType::kNarrow>;
using WideString         = String<StringType::kWide>;

// 字面量运算符。
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u8s(){
	static const Utf8String s_u8sRet{ kCharsT... };
	return s_u8sRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u16s(){
	static const Utf16String s_u16sRet{ kCharsT... };
	return s_u16sRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u32s(){
	static const Utf32String s_u32sRet{ kCharsT... };
	return s_u32sRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_ns(){
	static const NarrowString s_nsRet{ kCharsT... };
	return s_nsRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_ws(){
	static const WideString s_wsRet{ kCharsT... };
	return s_wsRet;
}

}

#endif
