#include <MCF/StdMCF.hpp>
#include <MCF/Core/StringObserver.hpp>
#include <cstdio>
using namespace MCF;

extern "C" __declspec(dllexport) int __stdcall dlltest(int a, int b) noexcept {
	return a + b;
}

bool MCFDll_OnProcessAttach(bool bDynamic){
	std::printf("on process attach, dynamic = %d\n", bDynamic);
	return true;
}
void MCFDll_OnProcessDetach(bool bDynamic){
	std::printf("on process detach, dynamic = %d\n", bDynamic);
}

void MCFDll_OnThreadAttach(){
	std::printf("on thread attach\n");
}
void MCFDll_OnThreadDetach(){
	std::printf("on thread detach\n");
}
