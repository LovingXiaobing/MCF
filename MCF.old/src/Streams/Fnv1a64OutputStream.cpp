// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "Fnv1a64OutputStream.hpp"
#include "../Core/Array.hpp"
#include "../Core/Endian.hpp"

// http://www.isthe.com/chongo/tech/comp/fnv/

namespace MCF {

Fnv1a64OutputStream::~Fnv1a64OutputStream(){ }

void Fnv1a64OutputStream::X_Initialize() noexcept {
	x_u64Reg = 14695981039346656037u;
}
void Fnv1a64OutputStream::X_Update(const std::uint8_t (&abyChunk)[8]) noexcept {
	auto u64Word = LoadLe(reinterpret_cast<const std::uint64_t *>(abyChunk)[0]);
	for(unsigned uIndex = 0; uIndex < sizeof(u64Word); ++uIndex){
		const unsigned uLow = static_cast<unsigned char>(u64Word);
		u64Word >>= 8;
		x_u64Reg ^= uLow;
		x_u64Reg *= 1099511628211u;
	}
}
void Fnv1a64OutputStream::X_Finalize(std::uint8_t (&abyChunk)[8], unsigned uBytesInChunk) noexcept {
	for(unsigned uIndex = 0; uIndex < uBytesInChunk; ++uIndex){
		const unsigned uLow = abyChunk[uIndex];
		x_u64Reg ^= uLow;
		x_u64Reg *= 1099511628211u;
	}
}

void Fnv1a64OutputStream::Put(unsigned char byData) noexcept {
	Put(&byData, 1);
}
void Fnv1a64OutputStream::Put(const void *pData, std::size_t uSize) noexcept {
	if(x_nChunkOffset < 0){
		X_Initialize();
		x_nChunkOffset = 0;
	}

	auto pbyRead = static_cast<const unsigned char *>(pData);
	auto uBytesRemaining = uSize;
	const auto uChunkAvail = sizeof(x_abyChunk) - static_cast<unsigned>(x_nChunkOffset);
	if(uBytesRemaining >= uChunkAvail){
		if(x_nChunkOffset != 0){
			std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uChunkAvail);
			pbyRead += uChunkAvail;
			uBytesRemaining -= uChunkAvail;
			X_Update(x_abyChunk);
			x_nChunkOffset = 0;
		}
		while(uBytesRemaining >= sizeof(x_abyChunk)){
			X_Update(reinterpret_cast<const decltype(x_abyChunk) *>(pbyRead)[0]);
			pbyRead += sizeof(x_abyChunk);
			uBytesRemaining -= sizeof(x_abyChunk);
		}
	}
	if(uBytesRemaining != 0){
		std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uBytesRemaining);
		x_nChunkOffset += static_cast<int>(uBytesRemaining);
	}
}
void Fnv1a64OutputStream::Flush(bool bHard) noexcept {
	(void)bHard;
}

void Fnv1a64OutputStream::Reset() noexcept {
	x_nChunkOffset = -1;
}
std::uint64_t Fnv1a64OutputStream::Finalize() noexcept {
	if(x_nChunkOffset >= 0){
		X_Finalize(x_abyChunk, static_cast<unsigned>(x_nChunkOffset));
	} else {
		X_Initialize();
		X_Finalize(x_abyChunk, 0);
	}
	x_nChunkOffset = -1;

	return x_u64Reg;
}

}
