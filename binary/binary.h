#include "../generater/generator.h"

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <unordered_map>

namespace vm {

    using u1 = std::uint8_t;
    using u2 = std::uint16_t;
    using u4 = std::uint32_t;
    using u8 = std::uint64_t;

    using i1 = std::int8_t;
    using i2 = std::int16_t;
    using i4 = std::int32_t;
    using i8 = std::int64_t;

//    using slot_t   = i4;
//    using int_t    = slot_t;
//    using double_t = f8;
//    using addr_t   = slot_t;
//    using char_t   = unsigned char;
//    using str_t    = std::string;
}

namespace c0 {
    class Binary {
    public:
        Binary(std::vector<std::pair<char, std::string>> constants, std::vector<Instruction> start,
               std::vector<funcInfo> functions, std::vector<std::vector<Instruction>> instructions)
                : _constants(std::move(constants)), _start(std::move(start)),
                  _functions(std::move(functions)), _instructions(std::move(instructions)) {}

        void output_binary(std::ofstream &out);

    private:
        std::vector<std::pair<char, std::string>> _constants;
        std::vector<Instruction> _start;
        std::vector<funcInfo> _functions;
        std::vector<std::vector<Instruction>> _instructions;

        static void writeNBytes (std::ostream &out, void *addrstatic , int count);
        void to_binary (std::ostream &out, const std::vector<c0::Instruction> &v);

        const std::unordered_map<opCode, std::vector<int>> oprPara = {
            { opCode::biPush, {1} },
            { opCode::iPush, {4} },
            { opCode::popN, {4} },
            { opCode::loadC, {2} },
            { opCode::loadA, {2, 4} },
            { opCode::jmp, {2} },
            { opCode::je, {2} },
            { opCode::jne, {2} },
            { opCode::jl, {2} },
            { opCode::jge, {2} },
            { opCode::jg, {2} },
            { opCode::jle, {2} },
            { opCode::call, {2} },
        };
    };
}