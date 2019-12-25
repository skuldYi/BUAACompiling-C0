#pragma once

#include <cstdint>
#include <utility>
#include <sstream>
#include "symbol.h"

namespace c0 {

    class Func final {
    private:
        using int32_t = std::int32_t;

    public:
//
//		Quadruple() : Quadruple(QuadOpr::ILL, 0){}
//		Quadruple(const Quadruple& i) { _opr = i._opr; _x = i._x; }
//		Quadruple(Quadruple&& i) :Quadruple() { swap(*this, i); }
//		Quadruple& operator=(Quadruple i) { swap(*this, i); return *this; }
//		bool operator==(const Quadruple& i) const { return _opr == i._opr && _x == i._x; }

        Func(int paraSize, int level, SymbolType returnType) : _paraSize(paraSize), _level(level),
                                                               _returnType(returnType) {}
        Func(SymbolType returnType) : _paraSize(0), _level(1), _returnType(returnType) {}

        void addPara(SymbolType type) {
//            _paraSeq.emplace_back(type, name);
            if (type == SymbolType::Double)
                _paraSize += 2;
            else
                _paraSize++;
        }

        int getParaSize() const {
            return _paraSize;
        }

        SymbolType getReturnType() const {
            return _returnType;
        }

    private:
        int _paraSize;
        int _level;
        SymbolType _returnType;
//        std::vector<std::pair<SymbolType, std::string>> _paraSeq;
    };

}