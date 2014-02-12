// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Z.hpp"
#include "../Core/NoCopy.hpp"
#include "../Core/Exception.hpp"

#define ZLIB_CONST
#define Z_PREFIX
#include "../../External/zlib/zlib.h"

#include <iterator>
#include <memory>
using namespace MCF;

namespace {
	inline void CopyOut(const std::function<std::pair<void *, std::size_t>(std::size_t)> &fnDataCallback, const void *pSrc, std::size_t uBytesToCopy){
		std::size_t uBytesCopied = 0;
		while(uBytesCopied < uBytesToCopy){
			const std::size_t uBytesRemaining = uBytesToCopy - uBytesCopied;
			const auto Result = fnDataCallback(uBytesRemaining);
			const std::size_t uBytesToCopyThisTime = std::min(Result.second, uBytesRemaining);
			__builtin_memcpy(Result.first, (const unsigned char *)pSrc + uBytesCopied, uBytesToCopyThisTime);
			uBytesCopied += uBytesToCopyThisTime;
		}
	};

	inline unsigned long ZErrorToWin32Error(int nZError) noexcept {
		switch(nZError){
		case Z_OK:
			return ERROR_SUCCESS;
		case Z_STREAM_END:
			return ERROR_HANDLE_EOF;
		case Z_NEED_DICT:
			return ERROR_INVALID_PARAMETER;
		case Z_ERRNO:
			return ERROR_OPEN_FAILED;
		case Z_STREAM_ERROR:
			return ERROR_INVALID_PARAMETER;
		case Z_DATA_ERROR:
			return ERROR_INVALID_DATA;
		case Z_MEM_ERROR:
			return ERROR_NOT_ENOUGH_MEMORY;
		case Z_BUF_ERROR:
			return ERROR_SUCCESS;
		case Z_VERSION_ERROR:
			return ERROR_NOT_SUPPORTED;
		default:
			return ERROR_INVALID_FUNCTION;
		}
	}
}

// ========== ZEncoder ==========
// 嵌套类定义。
class ZEncoder::xDelegate : private ::z_stream, NO_COPY {
private:
	const std::function<std::pair<void *, std::size_t>(std::size_t)> xm_fnDataCallback;
	bool xm_bInited;

	unsigned char xm_abyTemp[0x1000];
public:
	xDelegate(std::function<std::pair<void *, std::size_t>(std::size_t)> &&fnDataCallback, int nLevel)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_bInited(false)
	{
		zalloc = Z_NULL;
		zfree = Z_NULL;
		opaque = Z_NULL;

		const auto ulErrorCode = ZErrorToWin32Error(::deflateInit2(this, nLevel, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY));
		if(ulErrorCode != ERROR_SUCCESS){
			MCF_THROW(ulErrorCode, L"::deflateInit2() 失败。");
		}
	}
	~xDelegate(){
		::deflateEnd(this);
	}
public:
	void Abort() noexcept {
		xm_bInited = false;
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_bInited){
			::deflateReset(this);

			next_out = xm_abyTemp;
			avail_out = sizeof(xm_abyTemp);

			xm_bInited = true;
		}

		if(uSize != 0){
			auto pbyRead = (const unsigned char *)pData;
			const auto pbyEnd = pbyRead + uSize;
			do {
				const auto uBytesToProcess = std::min<std::size_t>(0x100000, pbyEnd - pbyRead);

				next_in = pbyRead;
				avail_in = uBytesToProcess;
				do {
					const auto ulErrorCode = ZErrorToWin32Error(::deflate(this, Z_NO_FLUSH));
					if(ulErrorCode == ERROR_HANDLE_EOF){
						break;
					}
					if(ulErrorCode != ERROR_SUCCESS){
						MCF_THROW(ulErrorCode, L"::deflate() 失败。");
					}
					if(avail_out == 0){
						CopyOut(xm_fnDataCallback, xm_abyTemp, sizeof(xm_abyTemp));

						next_out = xm_abyTemp;
						avail_out = sizeof(xm_abyTemp);
					}
				} while(avail_in != 0);

				pbyRead += uBytesToProcess;
			} while(pbyRead != pbyEnd);
		}
	}
	void Finalize(){
		if(xm_bInited){
			unsigned long ulErrorCode;
			for(;;){
				ulErrorCode = ZErrorToWin32Error(::deflate(this, Z_FINISH));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::deflate() 失败。");
				}
				if(avail_out == 0){
					CopyOut(xm_fnDataCallback, xm_abyTemp, sizeof(xm_abyTemp));

					next_out = xm_abyTemp;
					avail_out = sizeof(xm_abyTemp);
				}
			}
			if(avail_out < sizeof(xm_abyTemp)){
				CopyOut(xm_fnDataCallback, xm_abyTemp, sizeof(xm_abyTemp) - avail_out);
			}

			xm_bInited = false;
		}
	}
};

// 构造函数和析构函数。
ZEncoder::ZEncoder(std::function<std::pair<void *, std::size_t>(std::size_t)> fnDataCallback, int nLevel)
	: xm_pDelegate(new xDelegate(std::move(fnDataCallback), nLevel))
{
}
ZEncoder::~ZEncoder(){
}

// 其他非静态成员函数。
void ZEncoder::Update(const void *pData, std::size_t uSize){
	xm_pDelegate->Update(pData, uSize);
}
void ZEncoder::Finalize(){
	xm_pDelegate->Finalize();
}

// ========== ZDecoder ==========
// 嵌套类定义。
class ZDecoder::xDelegate : private ::z_stream, NO_COPY {
private:
	const std::function<std::pair<void *, std::size_t>(std::size_t)> xm_fnDataCallback;
	bool xm_bInited;

	unsigned char xm_abyTemp[0x1000];
public:
	xDelegate(std::function<std::pair<void *, std::size_t>(std::size_t)> &&fnDataCallback)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_bInited(false)
	{
		zalloc = Z_NULL;
		zfree = Z_NULL;
		opaque = Z_NULL;

		next_in = nullptr;
		avail_in = 0;

		const auto ulErrorCode = ZErrorToWin32Error(::inflateInit2(this, -15));
		if(ulErrorCode != ERROR_SUCCESS){
			MCF_THROW(ulErrorCode, L"::inflateInit2() 失败。");
		}
	}
	~xDelegate(){
		::inflateEnd(this);
	}
public:
	void Abort() noexcept {
		xm_bInited = false;
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_bInited){
			::inflateReset(this);

			next_out = xm_abyTemp;
			avail_out = sizeof(xm_abyTemp);

			xm_bInited = true;
		}

		if(uSize != 0){
			auto pbyRead = (const unsigned char *)pData;
			const auto pbyEnd = pbyRead + uSize;
			do {
				const auto uBytesToProcess = std::min<std::size_t>(0x100000, pbyEnd - pbyRead);

				next_in = pbyRead;
				avail_in = uBytesToProcess;
				do {
					const auto ulErrorCode = ZErrorToWin32Error(::inflate(this, Z_NO_FLUSH));
					if(ulErrorCode == ERROR_HANDLE_EOF){
						break;
					}
					if(ulErrorCode != ERROR_SUCCESS){
						MCF_THROW(ulErrorCode, L"::inflate() 失败。");
					}
					if(avail_out == 0){
						CopyOut(xm_fnDataCallback, xm_abyTemp, sizeof(xm_abyTemp));

						next_out = xm_abyTemp;
						avail_out = sizeof(xm_abyTemp);
					}
				} while(avail_in != 0);

				pbyRead += uBytesToProcess;
			} while(pbyRead != pbyEnd);
		}
	}
	void Finalize(){
		if(xm_bInited){
			unsigned long ulErrorCode;
			for(;;){
				ulErrorCode = ZErrorToWin32Error(::inflate(this, Z_FINISH));
				if(ulErrorCode == ERROR_HANDLE_EOF){
					break;
				}
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::inflate() 失败。");
				}
				if(avail_out == 0){
					CopyOut(xm_fnDataCallback, xm_abyTemp, sizeof(xm_abyTemp));

					next_out = xm_abyTemp;
					avail_out = sizeof(xm_abyTemp);
				}
			}
			if(avail_out < sizeof(xm_abyTemp)){
				CopyOut(xm_fnDataCallback, xm_abyTemp, sizeof(xm_abyTemp) - avail_out);
			}

			xm_bInited = false;
		}
	}
};

// 构造函数和析构函数。
ZDecoder::ZDecoder(std::function<std::pair<void *, std::size_t>(std::size_t)> fnDataCallback)
	:  xm_pDelegate(new xDelegate(std::move(fnDataCallback)))
{
}
ZDecoder::~ZDecoder(){
}

// 其他非静态成员函数。
void ZDecoder::Update(const void *pData, std::size_t uSize){
	xm_pDelegate->Update(pData, uSize);
}
void ZDecoder::Finalize(){
	xm_pDelegate->Finalize();
}
