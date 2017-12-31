// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_RECONSTRUCT_OR_ASSIGN_HPP_
#define MCF_CORE_RECONSTRUCT_OR_ASSIGN_HPP_

#include "ConstructDestruct.hpp"
#include <type_traits>

namespace MCF {

namespace Impl_ReconstructOrAssign {
	struct ToReconstruct {
		template<typename ObjectT, typename ...ParamsT>
		void operator()(ObjectT *pObject, ParamsT &&...vParams) const noexcept {
			Destruct(pObject);
			Construct(pObject, std::forward<ParamsT>(vParams)...);
		}
	};
	struct ToAssign {
		template<typename ObjectT, typename ParamT>
		void operator()(ObjectT *pObject, ParamT &&vParam) const {
			*pObject = std::forward<ParamT>(vParam);
		}
		template<typename ObjectT, typename ...ParamsT>
		void operator()(ObjectT *pObject, ParamsT &&...vParams) const {
			*pObject = ObjectT(std::forward<ParamsT>(vParams)...);
		}
	};
}

template<typename ObjectT, typename ...ParamsT>
void ReconstructOrAssign(ObjectT *pObject, ParamsT &&...vParams) noexcept(std::is_nothrow_destructible<ObjectT>::value && std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value) {
	std::conditional_t<
		std::is_nothrow_destructible<ObjectT>::value && std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value,
		Impl_ReconstructOrAssign::ToReconstruct, Impl_ReconstructOrAssign::ToAssign
		>()(pObject, std::forward<ParamsT>(vParams)...);
}

}

#endif
