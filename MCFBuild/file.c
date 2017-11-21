// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "file.h"
#include <windows.h>

bool MCFBUILD_FileGetContents(void *restrict *restrict ppData, MCFBUILD_STD size_t *puSize, const wchar_t *restrict pwcPath){
	DWORD dwErrorCode;
	// Open the file for reading. Fail if it does not exist.
	HANDLE hFile = CreateFileW(pwcPath, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	// Query the number of bytes in it.
	LARGE_INTEGER liFileSize;
	if(!GetFileSizeEx(hFile, &liFileSize)){
		dwErrorCode = GetLastError();
		CloseHandle(hFile);
		SetLastError(dwErrorCode);
		return false;
	}
	// Make sure the size fits into 32 bits.
	DWORD dwBytesToReadTotal = (DWORD)liFileSize.QuadPart;
	if((LONGLONG)dwBytesToReadTotal != liFileSize.QuadPart){
		CloseHandle(hFile);
		SetLastError(ERROR_ARITHMETIC_OVERFLOW);
		return false;
	}
	// Allocate the buffer that is to be freed using `MCFBUILD_FileFreeContents()`.
	void *pData = HeapAlloc(GetProcessHeap(), 0, dwBytesToReadTotal);
	if(!pData){
		CloseHandle(hFile);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}
	// Read data in a loop, up to the specified number of bytes.
	DWORD dwBytesReadTotal = 0;
	for(;;){
		if(dwBytesReadTotal >= dwBytesToReadTotal){
			break;
		}
		// If an error occurs, deallocate the buffer and bail out.
		DWORD dwBytesRead;
		if(!ReadFile(hFile, (char *)pData + dwBytesReadTotal, dwBytesToReadTotal - dwBytesReadTotal, &dwBytesRead, NULL)){
			dwErrorCode = GetLastError();
			HeapFree(GetProcessHeap(), 0, pData);
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		// EOF? This should not happen.
		if(dwBytesRead == 0){
			break;
		}
		dwBytesReadTotal += dwBytesRead;
	}
	CloseHandle(hFile);
	// Return the buffer to our caller.
	*ppData = pData;
	*puSize = dwBytesReadTotal;
	return true;
}
void MCFBUILD_FileFreeContents(void *pData){
	// Be warned that passing a null pointer to `HeapFree()` is undefined behavior.
	if(!pData){
		return;
	}
	HeapFree(GetProcessHeap(), 0, pData);
}

bool MCFBUILD_FilePutContents(const wchar_t *pwcPath, const void *pData, size_t uSize){
	DWORD dwErrorCode;
	// Make sure the size fits into 32 bits.
	DWORD dwBytesToWriteTotal = (DWORD)uSize;
	if(dwBytesToWriteTotal != uSize){
		SetLastError(ERROR_ARITHMETIC_OVERFLOW);
		return false;
	}
	// Open the file for writing. Create one if it does not exist. Any existent data are discarded.
	HANDLE hFile = CreateFileW(pwcPath, FILE_WRITE_DATA, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	// Write data in a loop, up to the specified number of bytes.
	DWORD dwBytesWrittenTotal = 0;
	for(;;){
		if(dwBytesWrittenTotal >= dwBytesToWriteTotal){
			break;
		}
		// If an error occurs, bail out.
		DWORD dwBytesWritten;
		if(!WriteFile(hFile, (const char *)pData + dwBytesWrittenTotal, dwBytesToWriteTotal - dwBytesWrittenTotal, &dwBytesWritten, NULL)){
			dwErrorCode = GetLastError();
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		// What the hell? This should not happen.
		if(dwBytesWritten == 0){
			CloseHandle(hFile);
			SetLastError(ERROR_BROKEN_PIPE);
			return false;
		}
		dwBytesWrittenTotal += dwBytesWritten;
	}
	CloseHandle(hFile);
	return true;
}
bool MCFBUILD_FileAppendContents(const wchar_t *pwcPath, const void *pData, size_t uSize){
	DWORD dwErrorCode;
	// Make sure the size fits into 32 bits.
	DWORD dwBytesToWriteTotal = (DWORD)uSize;
	if(dwBytesToWriteTotal != uSize){
		SetLastError(ERROR_ARITHMETIC_OVERFLOW);
		return false;
	}
	// Open the file for appending. Create one if it does not exist. Any existent data are left alone.
	HANDLE hFile = CreateFileW(pwcPath, FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	// Set the file pointer to the EOF.
	LARGE_INTEGER liFilePointer;
	liFilePointer.QuadPart = 0;
	if(!SetFilePointerEx(hFile, liFilePointer, NULL, FILE_END)){
		dwErrorCode = GetLastError();
		CloseHandle(hFile);
		SetLastError(dwErrorCode);
		return false;
	}
	// Write data in a loop, up to the specified number of bytes.
	DWORD dwBytesWrittenTotal = 0;
	for(;;){
		if(dwBytesWrittenTotal >= dwBytesToWriteTotal){
			break;
		}
		// If an error occurs, bail out.
		DWORD dwBytesWritten;
		if(!WriteFile(hFile, (const char *)pData + dwBytesWrittenTotal, dwBytesToWriteTotal - dwBytesWrittenTotal, &dwBytesWritten, NULL)){
			dwErrorCode = GetLastError();
			CloseHandle(hFile);
			SetLastError(dwErrorCode);
			return false;
		}
		// What the hell? This should not happen.
		if(dwBytesWritten == 0){
			CloseHandle(hFile);
			SetLastError(ERROR_BROKEN_PIPE);
			return false;
		}
		dwBytesWrittenTotal += dwBytesWritten;
	}
	CloseHandle(hFile);
	return true;
}
