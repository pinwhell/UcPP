#pragma once

#include <unicorn/unicorn.h>
#include <cstdint>
#include <cstddef>
#include <atomic>

namespace Uc {
	struct Engine;
	struct Mem {
		Mem(Engine& uc);

		std::uint64_t Alloc(std::size_t len, std::uint32_t prot = UC_PROT_READ | UC_PROT_WRITE, std::uint64_t at = ~0ull);
		std::uint64_t Map(void* ptr, std::size_t len, std::uint32_t prot = UC_PROT_READ | UC_PROT_WRITE, std::uint64_t at = ~0ull);

		uc_engine* mUc;
		std::uint64_t mVirtualLimit;
		std::atomic<std::uint64_t> mPtr;
	};
}