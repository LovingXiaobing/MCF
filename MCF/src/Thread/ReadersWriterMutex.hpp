// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_READER_WRITER_MUTEX_HPP_
#define MCF_THREAD_READER_WRITER_MUTEX_HPP_

#include "Mutex.hpp"
#include "../Core/Atomic.hpp"

namespace MCF {

class ReadersWriterMutex : MCF_NONCOPYABLE {
public:
	using UniqueReaderLock = Impl_UniqueLockTemplate::UniqueLockTemplate<ReadersWriterMutex, 1>;
	using UniqueWriterLock = Impl_UniqueLockTemplate::UniqueLockTemplate<ReadersWriterMutex, 2>;

	using SharedLock    = UniqueReaderLock;
	using ExclusiveLock = UniqueWriterLock;

private:
	Mutex x_mtxReaderGuard;
	Mutex x_mtxExclusive;
	Atomic<std::size_t> x_uReaders;

public:
	explicit constexpr ReadersWriterMutex(std::size_t uSpinCount = Mutex::kSuggestedSpinCount) noexcept
		: x_mtxReaderGuard(uSpinCount), x_mtxExclusive(0), x_uReaders(0)
	{
	}

public:
	std::size_t GetSpinCount() const noexcept {
		return x_mtxReaderGuard.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_mtxReaderGuard.SetSpinCount(uSpinCount);
	}

	bool TryAsReader(std::uint64_t u64UntilFastMonoClock = 0) noexcept;
	void LockAsReader() noexcept;
	void UnlockAsReader() noexcept;

	UniqueReaderLock TryGetReaderLock(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		UniqueReaderLock vLock(*this, false);
		vLock.Try(u64UntilFastMonoClock);
		return vLock;
	}
	UniqueReaderLock GetReaderLock() noexcept {
		return UniqueReaderLock(*this);
	}

	bool TryAsWriter(std::uint64_t u64UntilFastMonoClock = 0) noexcept;
	void LockAsWriter() noexcept;
	void UnlockAsWriter() noexcept;

	UniqueWriterLock TryGetWriterLock(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		UniqueWriterLock vLock(*this, false);
		vLock.Try(u64UntilFastMonoClock);
		return vLock;
	}
	UniqueWriterLock GetWriterLock() noexcept {
		return UniqueWriterLock(*this);
	}
};

static_assert(std::is_trivially_destructible<ReadersWriterMutex>::value, "Hey!");

namespace Impl_UniqueLockTemplate {
	template<>
	inline bool ReadersWriterMutex::UniqueReaderLock::X_DoTry(std::uint64_t u64UntilFastMonoClock) const noexcept {
		return x_pOwner->TryAsReader(u64UntilFastMonoClock);
	}
	template<>
	inline void ReadersWriterMutex::UniqueReaderLock::X_DoLock() const noexcept {
		x_pOwner->LockAsReader();
	}
	template<>
	inline void ReadersWriterMutex::UniqueReaderLock::X_DoUnlock() const noexcept {
		x_pOwner->UnlockAsReader();
	}

	template<>
	inline bool ReadersWriterMutex::UniqueWriterLock::X_DoTry(std::uint64_t u64UntilFastMonoClock) const noexcept {
		return x_pOwner->TryAsWriter(u64UntilFastMonoClock);
	}
	template<>
	inline void ReadersWriterMutex::UniqueWriterLock::X_DoLock() const noexcept {
		x_pOwner->LockAsWriter();
	}
	template<>
	inline void ReadersWriterMutex::UniqueWriterLock::X_DoUnlock() const noexcept {
		x_pOwner->UnlockAsWriter();
	}
}

}

#endif
