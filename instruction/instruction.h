#pragma once

#include <cstdint>
#include <utility>

namespace c0 {
    using u1 = std::uint8_t;
    enum opCode : u1 {
        nop = 0x00,
        biPush = 0x01,
        iPush = 0x02,
        pop1 = 0x04,
        popN = 0x06,
        loadC = 0x09,
        loadA = 0x0a,
        iLoad = 0x10,
        iStore = 0x20,
        iAdd = 0x30,
        iSub = 0x34,
        iMul = 0x38,
        iDiv = 0x3c,
        iNeg = 0x40,
        iCmp = 0x44,
        i2c = 0x62,
        jmp = 0x70,
        je = 0x71,
        jne = 0x72,
        jl = 0x73,
        jge = 0x74,
        jg = 0x75,
        jle = 0x76,
        call = 0x80,
        ret = 0x88,
        iRet = 0x89,
        iPrint = 0xa0,
        cPrint = 0xa2,
        sPrint = 0xa3,
        printL = 0xaf,
        iScan = 0xb0,
        cScan = 0xb2,
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