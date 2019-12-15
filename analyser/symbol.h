#pragma once

#include <cstdint>
#include <utility>
#include <sstream>

namespace miniplc0 {
    enum SymbolType {
        Char,
        Int,
        String,
        Double,
        Void
    };

    class Symbol final {
    private:
        using int32_t = std::int32_t;

    public:
//
//		Instruction() : Instruction(Operation::ILL, 0){}
//		Instruction(const Instruction& i) { _opr = i._opr; _x = i._x; }
//		Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
//		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
//		bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x; }

        Symbol(std::string name, int32_t stackIndex, SymbolType type,
               bool isConst, bool isInited, int16_t funcIndex)
                : _name(std::move(name)), _stackIndex(stackIndex), _type(type),
                  _isConst(isConst), _isInited(isInited), _funcIndex(funcIndex) {}

        const std::string &getName() const { return _name; }
        int32_t getStackIndex() const { return _stackIndex; }
        SymbolType getType() const { return _type; }
        bool isConst() const { return _isConst; }
        bool isInited() const { return _isInited; }
        void setInited(bool isInited) { _isInited = isInited; }
        int16_t getFuncIndex() const { return _funcIndex; }

        std::string toString() {
            char type = ' ';
            switch (_type) {
                case Char:
                    type = 'C';
                    break;
                case Int:
                    type = 'I';
                    break;
                case String:
                    type = 'S';
                    break;
                case Double:
                    type = 'D';
                    break;
                case Void:
                    type = 'V';
                    break;
            }
            std::stringstream ss;
            ss << _name << "\t" << type << "\t";
            ss << (_isConst ? "is" : "not") << " const\t";
            ss << (_isInited ? "is" : "not") << " inited\tfuncInd = " << _funcIndex;
            return ss.str();
        }

    private:
        std::string _name;
        int32_t _stackIndex;
        SymbolType _type;    // String for function
        bool _isConst;
        bool _isInited;
        int16_t _funcIndex;     // -1 for nonfunction
    };

}