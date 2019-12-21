#pragma once

#include <cstdint>
#include <utility>

namespace c0 {

	enum QuadOpr {
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
	
	class Quadruple final {
	public:
		friend void swap(Quadruple& lhs, Quadruple& rhs);
	public:
		Quadruple(QuadOpr opr, std::string x) : _opr(opr), _x(std::move(x)), _y(""), _r("") {}
		Quadruple(QuadOpr opr, std::string x, std::string y) : _opr(opr), _x(std::move(x)), _y(std::move(y)), _r("") {}
		Quadruple(QuadOpr opr, std::string x, std::string y, std::string r)
		    : _opr(opr), _x(std::move(x)), _y(std::move(y)), _r(std::move(r)) {}

		Quadruple() : Quadruple(QuadOpr::LAB, 0){}
		Quadruple(const Quadruple& i) { _opr = i._opr; _x = i._x; _y = i._y; _r = i._r;}
		Quadruple(Quadruple&& i) : Quadruple() { swap(*this, i); }
		Quadruple& operator=(Quadruple i) { swap(*this, i); return *this; }
		bool operator==(const Quadruple& i) const { return _opr == i._opr && _x == i._x; }

		QuadOpr getOperation() const { return _opr; }
		std::string getX() const { return _x; }
		std::string getY() const { return _y; }
		std::string getR() const { return _r; }
        void setX(const std::string &x) {_x = x;}
        void setY(const std::string &y) {_y = y;}


    private:
		QuadOpr _opr;
		std::string _x, _y, _r;
	};

	inline void swap(Quadruple& lhs, Quadruple& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
		swap(lhs._y, rhs._y);
		swap(lhs._r, rhs._r);
	}

}