#pragma once

#include <unicorn/unicorn.h>
#include <Uc/RChain.h>
#include <functional>
#include <cstdint>
#include <unordered_map>
#include <memory>

namespace Uc {
	struct Engine;
	struct Hooks {
		Hooks(Engine& uc);

		struct Ev {
			struct Mem {
				uc_mem_type mType;
				std::uint64_t mAddress;
				std::int32_t mSize;
				std::int64_t mValue;
			};

			struct Code {
				std::uint64_t mAddress;
				std::uint32_t mSize;
			};

			struct Intr {
				std::int32_t mNr;
			};

			union {
				Mem mMem;
				Code mCode;
				Intr mIntr;
			};
		};

		using HandlerChain = RChain::HandlerChain<bool(const Ev& e)>;
		using Handler = HandlerChain::_HandlerT;
		using Callback = HandlerChain::_Callback;

		template<typename THookType>
		Hooks& AddHook(THookType typePack, Callback callback)
		{
			for (std::uint32_t i = 0; i < 32; i++)
				if (((std::uint32_t)typePack >> i) & 1)
					AddHookByOrder(i, callback);
			return *this;
		}
		static bool MemoryHandler(uc_engine* uc,
			uc_mem_type type,
			std::uint64_t address,
			std::int32_t size,
			std::int64_t value,
			void* user_data
		);
		static bool CodeHandler(uc_engine* uc,
			std::uint64_t address,
			std::uint32_t size,
			void* user_data);
		static bool IntrHandler(uc_engine* uc, std::int32_t nr, void* user_data);
		static void* HandlerGet(std::uint32_t order);
		Hooks& AddHookByOrder(std::uint32_t typeOrder, Callback callback);

		Engine& mEngine;
		std::unordered_map<std::uint32_t, std::unique_ptr<HandlerChain>> mChains;
		std::unordered_map<std::uint32_t, std::unique_ptr<uc_hook>> mChainHooks;
		std::unordered_map<std::uint32_t, std::unique_ptr<std::function<bool(const Ev&)>>> mChainHandlers;
	};
}