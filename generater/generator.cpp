#include "generator.h"

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

    void Generator::preTreat() {
        for (int i = 0; i < (int)_quads.size(); i++) {
            if (_quads[i].getOperation() == QuadOpr::BZ
                || _quads[i].getOperation() == QuadOpr::BNZ) {
                std::string opr;
                switch (_quads[i - 1].getOperation()) {
                    case QuadOpr::EQU:
                        opr = "equ";
                        break;
                    case QuadOpr::NE:
                        opr = "ne";
                        break;
                    case QuadOpr::LT:
                        opr = "lt";
                        break;
                    case QuadOpr::LE:
                        opr = "le";
                        break;
                    case QuadOpr::GT:
                        opr = "gt";
                        break;
                    case QuadOpr::GE:
                        opr = "ge";
                        break;
                    default:
                        opr = "";
                }
                _quads[i].setY(opr);
            }
        }
    }

    void getAddr(std::vector<Instruction> & seq, const std::string& pos) {
        int global = 0, offset;
        if (pos[0] == 'c') {
            global = 1;
            offset = std::stoi(pos.substr(1));
        } else
            offset = std::stoi(pos);

        //loada level_diff(2), offset(4)
        seq.emplace_back(opCode::loadA, global, offset);
    }

    inline void loadI(std::vector<Instruction> & seq, const std::string & opr) {
        if (opr[0] == '$')
            seq.emplace_back(opCode::iPush, std::stoi(opr.substr(1)));
        else {
            getAddr(seq, opr);
            seq.emplace_back(opCode::iLoad);
        }
    }

    inline opCode calOpr(const QuadOpr & opr) {
        switch (opr) {
            case ADD:
                return opCode::iAdd;
            case SUB:
                return opCode::iSub;
            case MUL:
                return opCode::iMul;
            case DIV:
                return opCode::iDiv;
            default:
                return opCode::nop;
        }
    }

    inline opCode relOpr(const QuadOpr & opr, const std::string& rel) {
        opCode op = opCode::nop;
        switch (opr) {
            case GOTO:
                op = opCode::jmp;
                break;
            case BNZ:   // 满足条件
                if (rel == "equ")
                    op = opCode::je;
                else if (rel == "ne")
                    op = opCode::jne;
                else if (rel == "lt")
                    op = opCode::jl;
                else if (rel == "le")
                    op = opCode::jle;
                else if (rel == "gt")
                    op = opCode::jg;
                else if (rel == "ge")
                    op = opCode::jge;
                break;
            case BZ:    // 不满足条件
                if (rel == "equ")
                    op = opCode::jne;
                else if (rel == "ne")
                    op = opCode::je;
                else if (rel == "lt")
                    op = opCode::jge;
                else if (rel == "le")
                    op = opCode::jg;
                else if (rel == "gt")
                    op = opCode::jle;
                else if (rel == "ge")
                    op = opCode::jl;
                break;

            default:
                op = opCode::nop;
        }
        return op;
    }

    void Generator::generateCode(std::vector<Instruction>& seq, const Quadruple& quad) {
        switch (quad.getOperation()) {
            // a = t    ASN t	-	a
            case QuadOpr::ASN:
                getAddr(seq, quad.getR());
                loadI(seq, quad.getX());
                seq.emplace_back(opCode::iStore);
                break;
            // t = -a   a	-	t
            case QuadOpr::NEG:
                getAddr(seq, quad.getR());
                loadI(seq, quad.getX());
                seq.emplace_back(opCode::iNeg);
                seq.emplace_back(opCode::iStore);
                break;
            // t = a + b	ADD/SUB/MUL/DIV	a	b	t
            case QuadOpr::ADD:
            case QuadOpr::SUB:
            case QuadOpr::MUL:
            case QuadOpr::DIV:
                getAddr(seq, quad.getR());
                loadI(seq, quad.getX());
                loadI(seq, quad.getY());
                seq.emplace_back(calOpr(quad.getOperation()));
                seq.emplace_back(opCode::iStore);
                break;

            case QuadOpr::LAB:
                setLabel(quad.getX());
                break;
            case QuadOpr::FUNC:
                // wont happen
                break;

            // PUSH	a
            case QuadOpr::PUSH:
                if (quad.getX()[0] == '$')
                    seq.emplace_back(opCode::iPush, std::stoi(quad.getX().substr(1)));
                else
                    loadI(seq, quad.getX());
                break;
            //pop{a}	POP	a
            case QuadOpr::POP:
                seq.emplace_back(opCode::popN, std::stoi(quad.getX().substr(1)));
                break;
            //foo(a)		CAL	foo
            case QuadOpr::CAL:
                seq.emplace_back(opCode::call, getFuncId(quad.getX().substr(1)));
                break;
            //return a	RET	a/-
            case QuadOpr::RET:
                if (quad.getX().empty())
                    seq.emplace_back(opCode::ret);
                else {
                    loadI(seq, quad.getX());
                    seq.emplace_back(opCode::iRet);
                }
                break;

            case QuadOpr::EQU:
            case QuadOpr::NE:
            case QuadOpr::LT:
            case QuadOpr::LE:
            case QuadOpr::GT:
            case QuadOpr::GE:
                seq.emplace_back(opCode::iCmp);
                break;

            // 无条件跳转	GOTO	LABEL1
            // 满足跳转		BNZ 	LABEL1	opr
            // 不满足跳转	BZ 		LABEL1	opr
            case QuadOpr::GOTO:
            case QuadOpr::BNZ:
            case QuadOpr::BZ:
                seq.emplace_back(relOpr(quad.getOperation(), quad.getY()), std::stoi(quad.getX().substr(1)));
                break;

            //print(a)	PRT 	a 		i/c/s/ln
            case QuadOpr::PRT:
                if (quad.getY() == "@i") {
                    loadI(seq, quad.getX());
                    seq.emplace_back(opCode::iPrint);
                } else if (quad.getY() == "@c") {
                    loadI(seq, quad.getX());
                    seq.emplace_back(opCode::cPrint);
                } else if (quad.getY() == "@s") {
                    seq.emplace_back(opCode::loadC, constString(quad.getX().substr(1)));
                    seq.emplace_back(opCode::sPrint);
                } else if (quad.getY() == "@ln")
                    seq.emplace_back(opCode::printL);
                break;
            //scan(a)		SCN 	a
            case QuadOpr::SCN:
                getAddr(seq, quad.getX());
                seq.emplace_back(opCode::iScan);
                seq.emplace_back(opCode::iStore);
                break;
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
        return -1;
    }

    void Generator::setLabel(const std::string & label) {
        _labels[std::stoi(label.substr(1))] = _instructions.size();
   }

}