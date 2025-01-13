#pragma once

#include <unicorn/unicorn.h>
#include <Uc/Io.h>
#include <Uc/Hooks.h>
#include <Uc/Regs.h>
#include <Uc/Mem.h>
#include <cstdint>
#include <cstddef>

namespace Uc {
	struct Engine {
		Engine(uc_arch arch, uc_mode mode);
		~Engine();

		operator uc_engine* ();
		template<typename TAt, typename TUntil = std::uint64_t>
		inline uc_err Ignite(TAt at, TUntil until = {}, std::size_t nrIns = {})
		{
			return uc_emu_start(mUc, (std::uint64_t)at, (std::uint64_t)until, 0, nrIns);
		}

		uc_err Stop();

		uc_engine* mUc;
		bool mIs64;
		Io mIo;
		Hooks mHooks;
		Regs mRegs;
		Mem mMem;
	};
}