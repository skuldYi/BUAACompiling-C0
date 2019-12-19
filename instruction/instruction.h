#pragma once

#include <cstdint>
#include <utility>

namespace miniplc0 {

	enum Operation {
		ASN,
		NEG,
		ADD,
		SUB,
		MUL,
		DIV,

		LAB,
		FUNC,
		PUSH,
		POP,
		CAL,
		RET,

		EQU, NE,
		LT, LE,
		GT, GE,

		GOTO,
		BNZ,
		BZ,

		PRT,
		SCN
	};
	
	class Instruction final {
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
	public:
		Instruction(Operation opr, std::string x) : _opr(opr), _x(std::move(x)), _y(""), _r("") {}
		Instruction(Operation opr, std::string x, std::string y) : _opr(opr), _x(std::move(x)), _y(std::move(y)), _r("") {}
		Instruction(Operation opr, std::string x, std::string y, std::string r)
		    : _opr(opr), _x(std::move(x)), _y(std::move(y)), _r(std::move(r)) {}

		Instruction() : Instruction(Operation::LAB, 0){}
		Instruction(const Instruction& i) { _opr = i._opr; _x = i._x; _y = i._y; _r = i._r;}
		Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x; }

		Operation getOperation() const { return _opr; }
		std::string getX() const { return _x; }
		std::string getY() const { return _y; }
		std::string getR() const { return _r; }
	private:
		Operation _opr;
		std::string _x, _y, _r;
	};

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
		swap(lhs._y, rhs._y);
		swap(lhs._r, rhs._r);
	}
}