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
    struct byteCode {
        std::vector<std::pair<char, std::string>> _constants;
        std::vector<Instruction> _start;
        std::vector<struct funtionInfo> _functions;
        std::vector<std::vector<Instruction>> _instructions;
    };

    struct functionInfo {
        int name_index;
        int params_size;
        int level;
    };

	class Generator final {
	public:
		Generator(std::vector<Quadruple> q)
		    : _quads(std::move(q)), _constants({}), _start({}), _functions({}), _instructions({}) {}
		Generator(Generator&&) = delete;
		Generator(const Generator&) = delete;
		Generator& operator=(Generator) = delete;

		// 唯一接口
        struct byteCode* Generate();

    private:
        std::vector<Quadruple> _quads;

	    std::vector<std::pair<char, std::string>> _constants;
	    std::vector<Instruction> _start;
	    std::vector<struct funtionInfo> _functions;
	    std::vector<std::vector<Instruction>> _instructions;

	};
}
