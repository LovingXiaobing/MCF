// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_READER_WRITER_MUTEX_HPP_
#define MCF_THREAD_READER_WRITER_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "_UniqueLockTemplate.hpp"
#include "RecursiveMutex.hpp"
#include "Mutex.hpp"
#include "ThreadLocal.hpp"
#include <cstddef>

namespace MCF {

class ReaderWriterMutex : NONCOPYABLE, public RecursiveMutexResults {
public:
	using UniqueReaderLock = Impl_UniqueLockTemplate::UniqueLockTemplate<ReaderWriterMutex, 0u>;
	using UniqueWriterLock = Impl_UniqueLockTemplate::UniqueLockTemplate<ReaderWriterMutex, 1u>;

private:
	mutable RecursiveMutex x_mtxWriterGuard;
	mutable Mutex x_mtxExclusive;
	Atomic<std::size_t> x_uReaderCount;
	ThreadLocal<std::size_t> x_tlsReaderReentranceCount;

public:
	explicit ReaderWriterMutex(std::size_t uSpinCount = 0x100);

public:
	std::size_t GetSpinCount() const noexcept {
		return x_mtxWriterGuard.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_mtxWriterGuard.SetSpinCount(uSpinCount);
	}

	Result TryAsReader() noexcept;
	Result LockAsReader() noexcept;
	Result UnlockAsReader() noexcept;

	Result TryAsWriter() noexcept;
	Result LockAsWriter() noexcept;
	Result UnlockAsWriter() noexcept;

	UniqueReaderLock TryReaderLock() noexcept {
		UniqueReaderLock vLock(*this, false);
		vLock.Try();
		return vLock;
	}
	UniqueReaderLock GetReaderLock() noexcept {
		return UniqueReaderLock(*this);
	}

	UniqueWriterLock TryWriterLock() noexcept {
		UniqueWriterLock vLock(*this, false);
		vLock.Try();
		return vLock;
	}
	UniqueWriterLock GetWriterLock() noexcept {
		return UniqueWriterLock(*this);
	}
};

}

#endif
