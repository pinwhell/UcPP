#include <Uc/Engine.h>

using namespace Uc;

Engine::Engine(uc_arch arch, uc_mode mode)
	: mUc([&] {
	uc_open(arch, mode, &mUc);
	return mUc;
		}())
	, mIs64(mode& UC_MODE_64)
	, mIo(*this)
	, mHooks(*this)
	, mRegs(*this)
	, mMem(*this)
{}

Engine::~Engine()
{
	uc_close(mUc);
}

Engine::operator uc_engine* ()
{
	return mUc;
}

uc_err Engine::Stop()
{
	return uc_emu_stop(mUc);
}

Io::Io(Engine& uc)
	: mUc(uc)
{}

simplistic::io::Object Io::Address(std::uint64_t addr)
{
	return simplistic::io::Object(this, addr, mUc.mIs64);
}

std::size_t Io::ReadRaw(const std::uint8_t* where, std::uint8_t* out, std::size_t len)
{
	return uc_mem_read(mUc, (std::uint64_t)where, out, len) == UC_ERR_OK ? len : 0;
}

std::size_t Io::WriteRaw(std::uint8_t* where, const std::uint8_t* in, std::size_t len)
{
	return uc_mem_write(mUc, (std::uint64_t)where, in, len) == UC_ERR_OK ? len : 0;
}

Hooks::Hooks(Engine& uc)
	: mEngine(uc)
{
	AddHook(UC_HOOK_MEM_VALID, [](const auto& _, auto& nxt) {return true;});
	AddHook(UC_HOOK_CODE, [](const auto& _, auto& nxt) {return true;});
	AddHook(UC_HOOK_INTR, [this](const auto& _, auto& nxt) { mEngine.Stop(); return false;});
}

bool Hooks::MemoryHandler(uc_engine* uc, uc_mem_type type, std::uint64_t address, std::int32_t size, std::int64_t value, void* user_data) {
	Ev ev{}; ev.mMem = { type, address, size, value };
	std::function<bool(const Ev&)>& handler = *(std::function<bool(const Ev&)>*)user_data;
	return handler(ev);
}

bool Hooks::CodeHandler(uc_engine* uc, std::uint64_t address, std::uint32_t size, void* user_data)
{
	Ev ev{}; ev.mCode = { address, size };
	std::function<bool(const Ev&)>& handler = *(std::function<bool(const Ev&)>*)user_data;
	return handler(ev);
}

bool Hooks::IntrHandler(uc_engine* uc, std::int32_t nr, void* user_data) {
	Ev ev{}; ev.mIntr = { nr };
	std::function<bool(const Ev&)>& handler = *(std::function<bool(const Ev&)>*)user_data;
	return handler(ev);
}

void* Hooks::HandlerGet(std::uint32_t order)
{
	/*
	* To Impl
	UC_HOOK_INSN = 1 << 1,
	UC_HOOK_BLOCK = 1 << 3,
	UC_HOOK_INSN_INVALID = 1 << 14,
	UC_HOOK_EDGE_GENERATED = 1 << 15,
	UC_HOOK_TCG_OPCODE = 1 << 16,
	UC_HOOK_TLB_FILL = 1 << 17,
	*/
	const auto UC_HOOK_MEM =
		UC_HOOK_MEM_READ_UNMAPPED |
		UC_HOOK_MEM_WRITE_UNMAPPED |
		UC_HOOK_MEM_FETCH_UNMAPPED |
		UC_HOOK_MEM_READ_PROT |
		UC_HOOK_MEM_WRITE_PROT |
		UC_HOOK_MEM_FETCH_PROT |
		UC_HOOK_MEM_READ |
		UC_HOOK_MEM_WRITE |
		UC_HOOK_MEM_FETCH |
		UC_HOOK_MEM_READ_AFTER;

	std::uint32_t raw = 1u << order;

	if (raw & UC_HOOK_MEM)	return MemoryHandler;
	if (raw & UC_HOOK_CODE) return CodeHandler;
	if (raw & UC_HOOK_INTR) return IntrHandler;

	throw std::runtime_error("Unimplemented Handler 0x" + std::to_string(raw));
}

Hooks& Hooks::AddHookByOrder(std::uint32_t typeOrder, Callback callback)
{
	auto chainIt = mChains.find(typeOrder);
	if (chainIt != mChains.end())
	{
		(*chainIt->second) += callback;
		return *this;
	}

	// At this point chain doesnt exist yet. lets create one
	mChainHandlers.insert_or_assign(typeOrder,
		std::make_unique<std::function<bool(const Ev&)>>([this, typeOrder](const Ev& e) -> bool {
			return (*mChains[typeOrder])(e);
			}));
	mChainHooks[typeOrder] = std::make_unique<uc_hook>();
	uc_hook_add(mEngine, mChainHooks[typeOrder].get(), 1u << typeOrder,
		HandlerGet(typeOrder), mChainHandlers[typeOrder].get(), 0, ~(0xFFFull));
	mChains.insert_or_assign(typeOrder, std::make_unique<HandlerChain>(Handler(
		[](const auto& e, auto&) {
			return false;
		})));
	(*mChains[typeOrder]) += callback;
	return *this;
}

Regs::Regs(uc_engine* uc)
	: mUc(uc)
{}

Mem::Mem(Engine& uc)
	: mUc(uc)
	, mVirtualLimit(uc.mIs64 ? 0x7FFFFFFFFFFFFFFFull : 0xF1000000ull)
	, mPtr(mVirtualLimit) /*Upper Half*/
{}

std::uint64_t Mem::Alloc(std::size_t len, std::uint32_t prot, std::uint64_t at)
{
	if (!len) return 0;
	constexpr auto PAGESZ = 0x1000ull;
	constexpr auto PAGEMASK = PAGESZ - 1ull;
	len = (len + PAGEMASK) & ~PAGEMASK;
	auto allocBase = at == ~0ull ? mPtr.fetch_add(len) : at;
	if (uc_mem_map(mUc, allocBase, len, prot) != UC_ERR_OK) return 0;
	return allocBase;
}

std::uint64_t Mem::Map(void* ptr, std::size_t len, std::uint32_t prot, std::uint64_t at)
{
	if (!len) return 0;
	constexpr auto PAGESZ = 0x1000ull;
	constexpr auto PAGEMASK = PAGESZ - 1ull;
	const auto ptrOff = (std::uint64_t)ptr & PAGEMASK;
	ptr = (void*)((std::uint64_t)ptr & ~PAGEMASK);
	len = (len + PAGESZ) & ~PAGEMASK;
	auto allocBase = at == ~0ull ? mPtr.fetch_add(len) : at;
	if (uc_mem_map_ptr(mUc, allocBase, len, prot, ptr) != UC_ERR_OK) return 0;
	return allocBase | ptrOff;
}