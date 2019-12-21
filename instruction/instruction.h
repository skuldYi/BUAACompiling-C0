#pragma once

#include <cstdint>
#include <utility>

namespace c0 {

	enum opCode {
        nop,
        biPush,
        iPush,
        pop1,
        popN,
        loadC,
        loadA,
        iLoad,
        iStore,
        iAdd,
        iSub,
        iMul,
        iDiv,
        iNeg,
        iCmp,
        i2c,
        jmp,
        je,
        jne,
        jl,
        jge,
        jg,
        jle,
        call,
        ret,
        iRet,
        iPrint,
        cPrint,
        sPrint,
        printL,
        iScan,
        cScan
	};
	
	class Instruction final {
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
	public:


        Instruction(opCode opr) : _opr(opr), _x(0), _y(0) {}
        Instruction(opCode opr, int x) : _opr(opr), _x(x), _y(0) {}
        Instruction(opCode opr, int x, int y) : _opr(opr), _x(x), _y(y) {}

		Instruction() : Instruction(opCode::nop){}
		Instruction(const Instruction& i) { _opr = i._opr; _x = i._x; _y = i._y;}
		Instruction(Instruction&& i) : Instruction() { swap(*this, i); }
		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x; }

        opCode getOpr() const {return _opr;}
        int getX() const {return _x;}
        int getY() const {return _y;}

        void setOpr(opCode opr) {_opr = opr;}
        void setX(int x) {_x = x;}
        void setY(int y) {_y = y;}

    private:
		opCode _opr;
		int _x, _y;
	};

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
		swap(lhs._y, rhs._y);
	}
}