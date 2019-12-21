#include "generator.h"

#include <climits>

namespace c0 {
    byteCode Generator::Generate() {
        generate();

        byteCode code(_constants, _start, _functions, _instructions);

        return code;
    }

    void Generator::generate() {
        preTreat();

        int i, len = _quads.size();
        for (i = 0; i < len && _quads[i].getOperation() != QuadOpr::FUNC; i++) {
            generateCode(_start, _quads[i]);
        }

        while (i < len) {
            int16_t funcId = addFunction(_quads[i++]);
            _labels.clear();
            _instructions.emplace_back();

            for (; i < len && _quads[i].getOperation() != QuadOpr::FUNC; i++) {
                generateCode(_instructions[funcId], _quads[i]);
            }

            backfillLabel(_instructions[funcId]);
        }
    }


    void Generator::backfillLabel(std::vector<Instruction>& insSeq) {
        for (Instruction& i : insSeq) {
            switch (i.getOpr()) {
                case opCode::jmp:
                case opCode::je:	case opCode::jne:
                case opCode::jl:	case opCode::jge:
                case opCode::jg:	case opCode::jle:
                    i.setX(_labels[i.getX()]);
                    break;
                default:
                    break;
            }
        }
    }

    int Generator::addFunction(const Quadruple& quad) {
        // FUNC 	name	para_size	level
        int16_t name = constString(quad.getX().substr(1));
        int16_t size = std::stoi(quad.getY().substr(1));
        int16_t level = std::stoi(quad.getR().substr(1));

        _functions.emplace_back(name, size, level);
        return (int)_functions.size() - 1;
    }

    int Generator::constString(const std::string & s) {
        int i;
        for (i = 0; i < (int)_constants.size(); i++) {
            if (_constants[i].second == s)
                return i;
        }
        _constants.emplace_back('S', s);
        return i;
    }

    int Generator::getFuncId(const std::string & s) {
        int name = constString(s);
        for (int i = 0; i < (int)_functions.size(); i++) {
            if (_functions[i].name_index == name)
                return i;
        }
    }

    void Generator::setLabel(const std::string & label) {
        _labels[std::stoi(label.substr(1))] = _instructions.size();
   }

}