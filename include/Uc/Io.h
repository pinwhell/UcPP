#pragma once

#include <simplistic/io/IIO.h>
#include <simplistic/io/Object.h>

namespace Uc {
	struct Engine;
	struct Io : public simplistic::io::IIO {
		Io(Engine& uc);

		simplistic::io::Object Address(std::uint64_t addr);
		std::size_t ReadRaw(const std::uint8_t* where, std::uint8_t* out, std::size_t len) override;
		std::size_t WriteRaw(std::uint8_t* where, const std::uint8_t* in, std::size_t len) override;

		Engine& mUc;
	};
}