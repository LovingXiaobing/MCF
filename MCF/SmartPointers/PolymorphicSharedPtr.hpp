// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLYMORPHIC_SHARED_PTR_HPP_
#define MCF_SMART_POINTERS_POLYMORPHIC_SHARED_PTR_HPP_

#include "SharedPtr.hpp"

namespace MCF {

namespace Impl {
	struct PolymorphicSharedPtrContainerBase {
		virtual ~PolymorphicSharedPtrContainerBase() = 0;
	};

	template<typename ObjectT>
	struct PolymorphicSharedPtrContainer : public PolymorphicSharedPtrContainerBase {
		ObjectT m_vObjectT;

		template<typename ...ParamsT>
		explicit PolymorphicSharedPtrContainer(ParamsT &&...vParams)
			: m_vObjectT(std::forward<ParamsT>(vParams)...)
		{
		}
	};
}

template<typename ObjectT>
using PolymorphicSharedPtr = SharedPtr<ObjectT, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;

template<typename ObjectT>
using PolymorphicWeakPtr = WeakPtr<ObjectT, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;

template<typename ObjectT, typename ...ParamsT>
auto MakePolymorphicShared(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	const auto pContainer =
		new Impl::PolymorphicSharedPtrContainer<std::remove_cv_t<ObjectT>>(
			std::forward<ParamsT>(vParams)...);
	return PolymorphicSharedPtr<ObjectT>(
		SharedPtr<Impl::PolymorphicSharedPtrContainerBase,
			DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>(pContainer),
		&(pContainer->m_vObjectT));
}

template<typename DstT, typename SrcT>
auto DynamicPointerCast(PolymorphicSharedPtr<SrcT> rhs) noexcept {
	static_assert((std::is_const<DstT>::value == std::is_const<SrcT>::value) &&
		(std::is_volatile<DstT>::value == std::is_volatile<SrcT>::value), "cv-qualifiers mismatch.");

	const auto pContainer =
		dynamic_cast<Impl::PolymorphicSharedPtrContainer<std::remove_cv_t<DstT>> *>(rhs.GetRaw());
	if(!pContainer){
		return PolymorphicSharedPtr<DstT>();
	}
	return PolymorphicSharedPtr<DstT>(std::move(rhs), &(pContainer->m_vObjectT));
}

}

#endif
