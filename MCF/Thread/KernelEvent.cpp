// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelEvent.hpp"
#include "../Core/Exception.hpp"
#include <winternl.h>
#include <ntdef.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtCreateEvent(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, OBJECT_ATTRIBUTES *pObjectAttributes, EVENT_TYPE eEventType, BOOLEAN bInitialState) noexcept;

struct EventBasicInformation {
	EVENT_TYPE eEventType;
	LONG lState;
};

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtQueryEvent(HANDLE hEvent, int nInfoClass, void *pInfo, DWORD dwInfoSize, DWORD *pdwInfoSizeRet) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtSetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtResetEvent(HANDLE hEvent, LONG *plPrevState) noexcept;

namespace MCF {

// 构造函数和析构函数。
KernelEvent::KernelEvent(bool bInitSet, const WideStringView &wsvName){
	const auto uSize = wsvName.GetSize() * sizeof(wchar_t);
	if(uSize > UINT16_MAX){
		DEBUG_THROW(SystemError, ERROR_INVALID_PARAMETER, "The name for a kernel event is too long"_rcs);
	}
	::UNICODE_STRING ustrObjectName;
	ustrObjectName.Length        = uSize;
	ustrObjectName.MaximumLength = uSize;
	ustrObjectName.Buffer        = (PWSTR)wsvName.GetBegin();
	::OBJECT_ATTRIBUTES vObjectAttributes;
	InitializeObjectAttributes(&vObjectAttributes, &ustrObjectName, 0, nullptr, nullptr);

	HANDLE hEvent;
	const auto lStatus = ::NtCreateEvent(&hEvent, EVENT_ALL_ACCESS, &vObjectAttributes, NotificationEvent, bInitSet);
	if(!NT_SUCCESS(lStatus)){
		DEBUG_THROW(SystemError, ::RtlNtStatusToDosError(lStatus), "NtCreateEvent"_rcs);
	}
	x_hEvent.Reset(hEvent);
}

// 其他非静态成员函数。
bool KernelEvent::Wait(std::uint64_t u64MilliSeconds) const noexcept {
	if(u64MilliSeconds > static_cast<std::uint64_t>(INT64_MIN) / 10000){
		Wait();
		return true;
	}

	::LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = -static_cast<std::int64_t>(u64MilliSeconds * 10000);
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		return false;
	}
	return true;
}
void KernelEvent::Wait() const noexcept {
	const auto lStatus = ::NtWaitForSingleObject(x_hEvent.Get(), false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
}
bool KernelEvent::IsSet() const noexcept {
	EventBasicInformation vBasicInfo;
	const auto lStatus = ::NtQueryEvent(x_hEvent.Get(), 0 /* EventBasicInformation */, &vBasicInfo, sizeof(vBasicInfo), nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtQueryEvent() 失败。");
	}
	return vBasicInfo.lState;
}
bool KernelEvent::Set() noexcept {
	LONG lPrevState;
	const auto lStatus = ::NtSetEvent(x_hEvent.Get(), &lPrevState);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtSetEvent() 失败。");
	}
	return lPrevState;
}
bool KernelEvent::Reset() noexcept {
	LONG lPrevState;
	const auto lStatus = ::NtResetEvent(x_hEvent.Get(), &lPrevState);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtResetEvent() 失败。");
	}
	return lPrevState;
}

}