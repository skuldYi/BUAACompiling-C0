#pragma once

#include "error/error.h"
#include "instruction/quadruple.h"
#include "instruction/instruction.h"

#include <utility>
#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace c0 {
	class Generator final {
	public:
		Generator(std::vector<Quadruple> q) : _quads(std::move(q)) {}
		Generator(Generator&&) = delete;
		Generator(const Generator&) = delete;
		Generator& operator=(Generator) = delete;

		// 唯一接口
		std::ostream* Generate();

    private:
        std::vector<Quadruple> _quads;

		
	};
}
