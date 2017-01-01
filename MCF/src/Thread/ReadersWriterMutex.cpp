// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "ReadersWriterMutex.hpp"

namespace MCF {

bool ReadersWriterMutex::TryAsReader(std::uint64_t u64UntilFastMonoClock) noexcept {
	if(!x_mtxReaderGuard.Try(u64UntilFastMonoClock)){
		return false;
	}
	if(x_uReaders.Increment(kAtomicRelaxed) == 1){
		if(!x_mtxExclusive.Try(u64UntilFastMonoClock)){
			x_uReaders.Decrement(kAtomicRelaxed);
			x_mtxReaderGuard.Unlock();
			return false;
		}
	}
	x_mtxReaderGuard.Unlock();
	return true;
}
void ReadersWriterMutex::LockAsReader() noexcept {
	x_mtxReaderGuard.Lock();
	if(x_uReaders.Increment(kAtomicRelaxed) == 1){
		x_mtxExclusive.Lock();
	}
	x_mtxReaderGuard.Unlock();
}
void ReadersWriterMutex::UnlockAsReader() noexcept {
	if(x_uReaders.Decrement(kAtomicRelaxed) == 0){
		x_mtxExclusive.Unlock();
	}
}

bool ReadersWriterMutex::TryAsWriter(std::uint64_t u64UntilFastMonoClock) noexcept {
	if(!x_mtxReaderGuard.Try(u64UntilFastMonoClock)){
		return false;
	}
	if(!x_mtxExclusive.Try(u64UntilFastMonoClock)){
		x_mtxReaderGuard.Unlock();
		return false;
	}
	return true;
}
void ReadersWriterMutex::LockAsWriter() noexcept {
	x_mtxReaderGuard.Lock();
	x_mtxExclusive.Lock();
}
void ReadersWriterMutex::UnlockAsWriter() noexcept {
	x_mtxExclusive.Unlock();
	x_mtxReaderGuard.Unlock();
}

}
