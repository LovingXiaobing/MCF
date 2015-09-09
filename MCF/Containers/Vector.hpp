// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_VECTOR_HPP_
#define MCF_CONTAINERS_VECTOR_HPP_

#include "_EnumeratorTemplate.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Core/Exception.hpp"
#include <utility>
#include <new>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT>
class Vector {
private:
	void *x_pStorage;
	std::size_t x_uSize;
	std::size_t x_uCapacity;

public:
	constexpr Vector() noexcept
		: x_pStorage(nullptr), x_uSize(0), x_uCapacity(0)
	{
	}
	template<typename ...ParamsT>
	explicit Vector(std::size_t uSize, const ParamsT &...vParams)
		: Vector()
	{
		Append(uSize, vParams...);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	Vector(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: Vector()
	{
		Append(itBegin, itEnd);
	}
	Vector(std::initializer_list<ElementT> rhs)
		: Vector(rhs.begin(), rhs.end())
	{
	}
	Vector(const Vector &rhs)
		: Vector()
	{
		Reserve(rhs.GetSize());

		for(auto pElem = rhs.GetBegin(); pElem != rhs.GetEnd(); ++pElem){
			UncheckedPush(*pElem);
		}
	}
	Vector(Vector &&rhs) noexcept
		: Vector()
	{
		rhs.Swap(*this);
	}
	Vector &operator=(const Vector &rhs){
		Vector(rhs).Swap(*this);
		return *this;
	}
	Vector &operator=(Vector &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}
	~Vector(){
		Clear();
		::operator delete[](x_pStorage);
	}

private:
	ElementT *X_PrepareForInsertion(const ElementT *pPos, std::size_t uDeltaSize){
		ASSERT(std::is_nothrow_move_constructible<ElementT>::value);
		ASSERT(!IsEmpty());
		ASSERT(pPos);

		const auto uOffset = static_cast<std::size_t>(pPos - GetBegin());
		ReserveMore(uDeltaSize);
		const auto pBegin = GetBegin();

		for(std::size_t i = x_uSize; i > uOffset; --i){
			const auto pRead = pBegin + i - 1;
			const auto pWrite = pRead + uDeltaSize;
			Construct(pWrite, std::move(*pRead));
			Destruct(pRead);
		}

		return pBegin + uOffset;
	}
	void X_UndoPreparation(ElementT *pPrepared, std::size_t uDeltaSize) noexcept {
		ASSERT(std::is_nothrow_move_constructible<ElementT>::value);
		ASSERT(!IsEmpty());
		ASSERT(pPrepared);
		ASSERT(uDeltaSize <= GetCapacity() - GetSize());

		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPrepared - pBegin);

		for(std::size_t i = uOffset; i < x_uSize; ++i){
			const auto pWrite = pBegin + i;
			const auto pRead = pWrite + uDeltaSize;
			Construct(pWrite, std::move(*pRead));
			Destruct(pRead);
		}
	}

public:
	// 容器需求。
	using ElementType     = ElementT;
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <Vector>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <Vector>;

	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	void Clear() noexcept {
		Pop(x_uSize);
	}

	const ElementType *GetFirst() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	ElementType *GetFirst() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	const ElementType *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const ElementType *GetLast() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	ElementType *GetLast() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	const ElementType *GetConstLast() const noexcept {
		return GetLast();
	}

	const ElementType *GetPrev(const ElementType *pPos) const noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	ElementType *GetPrev(ElementType *pPos) noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	const ElementType *GetNext(const ElementType *pPos) const noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == x_uSize){
			return nullptr;
		}
		return pBegin + uOffset;
	}
	ElementType *GetNext(ElementType *pPos) noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == x_uSize){
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

	void Swap(Vector &rhs) noexcept {
		std::swap(x_pStorage,  rhs.x_pStorage);
		std::swap(x_uSize,     rhs.x_uSize);
		std::swap(x_uCapacity, rhs.x_uCapacity);
	}

	// Vector 需求。
	const ElementType *GetData() const noexcept {
		return static_cast<const ElementType *>(x_pStorage);
	}
	ElementType *GetData() noexcept {
		return static_cast<ElementType *>(x_pStorage);
	}
	const ElementType *GetConstData() const noexcept {
		return GetData();
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	std::size_t GetCapacity() noexcept {
		return x_uCapacity;
	}

	const ElementType *GetBegin() const noexcept {
		return GetData();
	}
	ElementType *GetBegin() noexcept {
		return GetData();
	}
	const ElementType *GetConstBegin() const noexcept {
		return GetBegin();
	}
	const ElementType *GetEnd() const noexcept {
		return GetData() + x_uSize;
	}
	ElementType *GetEnd() noexcept {
		return GetData() + x_uSize;
	}
	const ElementType *GetConstEnd() const noexcept {
		return GetEnd();
	}

	const ElementType &Get(std::size_t uIndex) const {
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	ElementType &Get(std::size_t uIndex){
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	const ElementType &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < x_uSize);

		return GetData()[uIndex];
	}
	ElementType &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < x_uSize);

		return GetData()[uIndex];
	}

	template<typename ...ParamsT>
	void Resize(std::size_t uSize, const ParamsT &...vParams){
		if(uSize > x_uSize){
			Append(uSize - x_uSize, vParams...);
		} else {
			Pop(x_uSize - uSize);
		}
	}
	template<typename ...ParamsT>
	ElementType *ResizeMore(std::size_t uDeltaSize, const ParamsT &...vParams){
		const auto uOldSize = x_uSize;
		Append(uDeltaSize - x_uSize, vParams...);
		return GetData() + uOldSize;
	}

	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity <= uOldCapacity){
			return;
		}

		auto uElementsToAlloc = uOldCapacity + 1;
		uElementsToAlloc += (uElementsToAlloc >> 1);
		uElementsToAlloc = (uElementsToAlloc + 0x0F) & (std::size_t)-0x10;
		if(uElementsToAlloc < uNewCapacity){
			uElementsToAlloc = uNewCapacity;
		}
		const std::size_t uBytesToAlloc = sizeof(ElementType) * uElementsToAlloc;
		if(uBytesToAlloc / sizeof(ElementType) != uElementsToAlloc){
			throw std::bad_array_new_length();
		}

		const auto pNewStorage = static_cast<ElementType *>(::operator new[](uBytesToAlloc));
		const auto pOldStorage = static_cast<ElementType *>(x_pStorage);
		auto pWrite = pNewStorage;
		try {
			for(std::size_t i = 0; i < x_uSize; ++i){
				Construct(pWrite, std::move_if_noexcept(pOldStorage[i]));
				++pWrite;
			}
		} catch(...){
			while(pWrite != pNewStorage){
				--pWrite;
				Destruct(pWrite);
			}
			::operator delete[](pNewStorage);
			throw;
		}
		for(std::size_t i = x_uSize; i > 0; --i){
			Destruct(pOldStorage + i - 1);
		}
		::operator delete[](pOldStorage);

		x_pStorage  = pNewStorage;
		x_uCapacity = uElementsToAlloc;
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = x_uSize;
		const auto uNewCapacity = uOldSize + uDeltaCapacity;
		if(uNewCapacity < uOldSize){
			throw std::bad_array_new_length();
		}
		Reserve(uNewCapacity);
	}

	template<typename ...ParamsT>
	void Push(ParamsT &&...vParams){
		ReserveMore(1);
		UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	void UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ElementType, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - x_uSize > 0);

		const auto pBegin = GetBegin();
		DefaultConstruct(pBegin + x_uSize, std::forward<ParamsT>(vParams)...);
		++x_uSize;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= x_uSize);

		const auto pBegin = GetBegin();
		for(std::size_t i = 0; i < uCount; ++i){
			Destruct(pBegin + x_uSize - i - 1);
		}
		x_uSize -= uCount;
	}

	template<typename ...ParamsT>
	void Append(std::size_t uDeltaSize, const ParamsT &...vParams){
		ReserveMore(uDeltaSize);

		std::size_t uElementsPushed = 0;
		try {
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				UncheckedPush(vParams...);
				++uElementsPushed;
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		if(kHasDeltaSizeHint){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			ReserveMore(uDeltaSize);
		}

		std::size_t uElementsPushed = 0;
		try {
			if(kHasDeltaSizeHint){
				for(auto it = itBegin; it != itEnd; ++it){
					UncheckedPush(*it);
					++uElementsPushed;
				}
			} else {
				for(auto it = itBegin; it != itEnd; ++it){
					Push(*it);
					++uElementsPushed;
				}
			}
		} catch(...){
			Pop(uElementsPushed);
			throw;
		}
	}
	void Append(std::initializer_list<ElementType> ilElements){
		Append(ilElements.begin(), ilElements.end());
	}

	template<typename ...ParamsT>
	void Emplace(const ElementType *pPos, ParamsT &&...vParams){
		if(!pPos || (pPos == GetEnd())){
			Push(std::forward<ParamsT>(vParams)...);
			return;
		}
		ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

		if(std::is_nothrow_move_constructible<ElementType>::value){
			const auto pWriteBegin = X_PrepareForInsertion(pPos, 1);
			try {
				DefaultConstruct(pWriteBegin, std::forward<ParamsT>(vParams)...);
			} catch(...){
				X_UndoPreparation(pWriteBegin, 1);
				throw;
			}
			++x_uSize;
		} else {
			auto uNewCapacity = GetSize() + 1;
			if(uNewCapacity < GetSize()){
				throw std::bad_array_new_length();
			}
			if(uNewCapacity < GetCapacity()){
				uNewCapacity = GetCapacity();
			}
			Vector vecTemp;
			vecTemp.Reserve(uNewCapacity);
			for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			vecTemp.UncheckedPush(std::forward<ParamsT>(vParams)...);
			for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			*this = std::move(vecTemp);
		}
	}

	template<typename ...ParamsT>
	void Insert(const ElementType *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
		if(!pPos || (pPos == GetEnd())){
			Append(uDeltaSize, vParams...);
			return;
		}
		ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

		if(std::is_nothrow_move_constructible<ElementType>::value){
			const auto pWriteBegin = X_PrepareForInsertion(pPos, uDeltaSize);
			auto pWrite = pWriteBegin;
			try {
				for(std::size_t i = 0; i < uDeltaSize; ++i){
					DefaultConstruct(pWrite, vParams...);
					++pWrite;
				}
			} catch(...){
				while(pWrite != pWriteBegin){
					--pWrite;
					Destruct(pWrite);
				}
				X_UndoPreparation(pWriteBegin, uDeltaSize);
				throw;
			}
			x_uSize += uDeltaSize;
		} else {
			auto uNewCapacity = GetSize() + uDeltaSize;
			if(uNewCapacity < GetSize()){
				throw std::bad_array_new_length();
			}
			if(uNewCapacity < GetCapacity()){
				uNewCapacity = GetCapacity();
			}
			Vector vecTemp;
			vecTemp.Reserve(uNewCapacity);
			for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			for(std::size_t i = 0; i < uDeltaSize; ++i){
				vecTemp.UncheckedPush(vParams...);
			}
			for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			*this = std::move(vecTemp);
		}
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	void Insert(const ElementType *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		if(!pPos || (pPos == GetEnd())){
			Append(itBegin, itEnd);
			return;
		}
		ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

		if(kHasDeltaSizeHint && std::is_nothrow_move_constructible<ElementType>::value){
			const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
			const auto pWriteBegin = X_PrepareForInsertion(pPos, uDeltaSize);
			auto pWrite = pWriteBegin;
			try {
				for(auto it = itBegin; it != itEnd; ++it){
					Construct(pWrite, *it);
					++pWrite;
				}
			} catch(...){
				while(pWrite != pWriteBegin){
					--pWrite;
					Destruct(pWrite);
				}
				X_UndoPreparation(pWriteBegin, uDeltaSize);
				throw;
			}
			x_uSize += uDeltaSize;
		} else {
			if(kHasDeltaSizeHint){
				const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
				auto uNewCapacity = GetSize() + uDeltaSize;
				if(uNewCapacity < GetSize()){
					throw std::bad_array_new_length();
				}
				if(uNewCapacity < GetCapacity()){
					uNewCapacity = GetCapacity();
				}
				Vector vecTemp;
				vecTemp.Reserve(uNewCapacity);
				for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				for(auto it = itBegin; it != itEnd; ++it){
					vecTemp.UncheckedPush(*it);
				}
				for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				*this = std::move(vecTemp);
			} else {
				Vector vecTemp;
				vecTemp.Reserve(GetCapacity());
				for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				for(auto it = itBegin; it != itEnd; ++it){
					vecTemp.Push(*it);
				}
				for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
					vecTemp.Push(*pCur);
				}
				*this = std::move(vecTemp);
			}
		}
	}
	void Insert(const ElementType *pPos, std::initializer_list<ElementType> ilElements){
		Insert(pPos, ilElements.begin(), ilElements.end());
	}

	void Erase(const ElementType *pBegin, const ElementType *pEnd) noexcept(std::is_nothrow_move_constructible<ElementType>::value) {
		if(pBegin == pEnd){
			return;
		}
		ASSERT(pBegin);

		if(!pEnd || (pEnd == GetEnd())){
			const auto uDeltaCount = static_cast<std::size_t>(GetEnd() - pBegin);

			Pop(uDeltaCount);
			return;
		}
		ASSERT((GetBegin() <= pBegin) && (pBegin <= pEnd) && (pEnd <= GetEnd()));

		if(std::is_nothrow_move_constructible<ElementType>::value){
			const auto uDeltaCount = static_cast<std::size_t>(pEnd - pBegin);

			auto pWrite = const_cast<ElementType *>(pBegin);
			for(auto pCur = pWrite; pCur != pEnd; ++pCur){
				Destruct(pCur);
			}
			for(auto pCur = const_cast<ElementType *>(pEnd); pCur != GetEnd(); ++pCur){
				Construct(pWrite, std::move(*pCur));
				Destruct(pCur);
				++pWrite;
			}
			x_uSize -= uDeltaCount;
		} else {
			Vector vecTemp;
			vecTemp.Reserve(GetCapacity());
			for(auto pCur = GetBegin(); pCur != pBegin; ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			for(auto pCur = pEnd; pCur != GetEnd(); ++pCur){
				vecTemp.UncheckedPush(*pCur);
			}
			*this = std::move(vecTemp);
		}
	}
	void Erase(const ElementType *pPos) noexcept(noexcept(std::declval<Vector &>().Erase(pPos, pPos))) {
		ASSERT(pPos);

		Erase(pPos, pPos + 1);
	}

	const ElementType &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	ElementType &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}
};

template<typename ElementT>
void swap(Vector<ElementT> &lhs, Vector<ElementT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ElementT>
decltype(auto) begin(const Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT>
decltype(auto) begin(Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}

template<typename ElementT>
decltype(auto) end(const Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT>
decltype(auto) end(Vector<ElementT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}

}

#endif