// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_LOCAL_HPP_
#define MCF_THREAD_THREAD_LOCAL_HPP_

#include "../Core/Assert.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/AlignedStorage.hpp"
#include "../Core/ConstructDestruct.hpp"
#include "../Core/ReconstructOrAssign.hpp"
#include <MCFCRT/pre/tls.h>
#include <MCFCRT/env/last_error.h>
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

template<typename ElementT>
class ThreadLocal {
	static_assert(alignof(ElementT) <= alignof(std::max_align_t), "ElementT is over-aligned.");

private:
	struct X_TlsKeyDeleter {
		constexpr ::_MCFCRT_TlsKeyHandle operator()() const noexcept {
			return nullptr;
		}
		void operator()(::_MCFCRT_TlsKeyHandle hTlsKey) const noexcept {
			::_MCFCRT_TlsFreeKey(hTlsKey);
		}
	};

	struct X_TlsContainer {
		AlignedStorage<ElementT> vStorage;
		bool bConstructed;
	};

	static unsigned long X_ContainerConstructor(std::intptr_t, void *pContainerRaw) noexcept {
		const auto pContainer = static_cast<X_TlsContainer *>(pContainerRaw);
		pContainer->bConstructed = false;
		return 0;
	}
	static void X_ContainerDestructor(std::intptr_t, void *pContainerRaw) noexcept {
		const auto pContainer = static_cast<X_TlsContainer *>(pContainerRaw);
		if(!pContainer->bConstructed){
			return;
		}
		void *const pElementRaw = &(pContainer->vStorage);
		const auto pElement = static_cast<ElementT *>(pElementRaw);
		Destruct(pElement);
	}

private:
	UniqueHandle<X_TlsKeyDeleter> x_hTlsKey;

public:
	explicit ThreadLocal(){
		const auto hTemp = ::_MCFCRT_TlsAllocKey(sizeof(X_TlsContainer), nullptr, &X_ContainerDestructor, 0);
		if(!hTemp){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"ThreadLocal: _MCFCRT_TlsAllocKey() 失败。"));
		}
		x_hTlsKey.Reset(hTemp);
	}

public:
	ElementT *Get() const noexcept {
		void *pContainerRaw;
		const bool bResult = ::_MCFCRT_TlsGet(x_hTlsKey.Get(), &pContainerRaw);
		MCF_ASSERT_MSG(bResult, L"_MCFCRT_TlsGet() 失败。");
		const auto pContainer = static_cast<X_TlsContainer *>(pContainerRaw);
		if(!pContainer){
			return nullptr;
		}

		if(!pContainer->bConstructed){
			return nullptr;
		}
		void *const pElementRaw = &(pContainer->vStorage);
		const auto pElement = static_cast<ElementT *>(pElementRaw);
		return pElement;
	}
	template<typename ...ParamsT>
	ElementT *Require(ParamsT &&...vParams) const {
		void *pContainerRaw;
		const bool bResult = ::_MCFCRT_TlsRequire(x_hTlsKey.Get(), &pContainerRaw);
		if(!bResult){
			MCF_THROW(Exception, ::_MCFCRT_GetLastWin32Error(), Rcntws::View(L"ThreadLocal: _MCFCRT_TlsRequire() 失败。"));
		}
		const auto pContainer = static_cast<X_TlsContainer *>(pContainerRaw);
		MCF_ASSERT(pContainer);

		void *const pElementRaw = &(pContainer->vStorage);
		const auto pElement = static_cast<ElementT *>(pElementRaw);
		if(!pContainer->bConstructed){
			DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
			pContainer->bConstructed = true;
		} else {
			ReconstructOrAssign(pElement, std::forward<ParamsT>(vParams)...);
		}
		return pElement;
	}

	void Swap(ThreadLocal &vOther) noexcept {
		using std::swap;
		swap(x_hTlsKey, vOther.x_hTlsKey);
	}

public:
	friend void swap(ThreadLocal &vSelf, ThreadLocal &vOther) noexcept {
		vSelf.Swap(vOther);
	}
};

}

#endif
