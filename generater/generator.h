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
    class funcInfo {
    public:
        std::int16_t name_index;
        std::int16_t params_size;
        std::int16_t level;

        funcInfo(int16_t name, int16_t size, int16_t level)
            : name_index(name), params_size(size), level(level) {}
    };

    class byteCode {
    public:
        byteCode(std::vector<std::pair<char, std::string>> constants,
                 std::vector<Instruction> start,
                 std::vector<funcInfo> functions,
                 std::vector<std::vector<Instruction>> instructions)
                  : constants(std::move(constants)), start(std::move(start)),
                    functions(std::move(functions)), instructions(std::move(instructions)) {}

        std::vector<std::pair<char, std::string>> constants;
        std::vector<Instruction> start;
        std::vector<funcInfo> functions;
        std::vector<std::vector<Instruction>> instructions;
    };

	class Generator final {
    private:
        using uint32_t = std::uint32_t;
        using int32_t = std::int32_t;
        using uint16_t = std::uint16_t;
        using int16_t = std::int16_t;

	public:
		Generator(std::vector<Quadruple> q)
		    : _quads(std::move(q)), _constants({}), _start({}), _functions({}), _instructions({}) {}
		Generator(Generator&&) = delete;
		Generator(const Generator&) = delete;
		Generator& operator=(Generator) = delete;

		// 唯一接口
        byteCode Generate();

    private:
        std::vector<Quadruple> _quads;
        std::map<int, int> _labels;

	    std::vector<std::pair<char, std::string>> _constants;
	    std::vector<Instruction> _start;
	    std::vector<funcInfo> _functions;
	    std::vector<std::vector<Instruction>> _instructions;

        void generate();

	    int addFunction(const Quadruple&);
	    int getFuncId(const std::string&);
	    int constString(const std::string&);

	    void preTreat();
	    void generateCode(std::vector<Instruction>&, const Quadruple&);
//	    void getAddr(std::vector<Instruction>&, const std::string&);
//	    void loadI(std::vector<Instruction>&, const std::string&);

	    void setLabel(const std::string&);
	    void backfillLabel(std::vector<Instruction>&);
	};
}
