#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"
#include "func.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {
	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
              _symbols({}), _functions({}), _nextStackIndex(0), _lastSymbolTable({}), _lastIndex({}) {}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;

		// 唯一接口
		std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyse();

	private:
		// 所有的递归子程序

		std::optional<CompilationError> analyseProgram();
		std::optional<CompilationError> analyseVariableDeclaration();
		std::optional<CompilationError> analyseFunctionDefinition();

		std::optional<CompilationError> analyseStatement();
		std::optional<CompilationError> analyseCompoundStatement(bool funcBody);
        std::optional<CompilationError> analyseCondition();
        std::optional<CompilationError> analyseConditionStatement();
        std::optional<CompilationError> analyseLoopStatement();
        std::optional<CompilationError> analyseJumpStatement();
        std::optional<CompilationError> analysePrintStatement();
        std::optional<CompilationError> analyseScanStatement();
        std::optional<CompilationError> analyseAssignmentStatement();
        std::optional<CompilationError> analyseFunctionCall();

        std::optional<CompilationError> analyseExpression(std::string&);
		std::optional<CompilationError> analyseTerm(std::string&);
		std::optional<CompilationError> analyseFactor(std::string&);

		// Token 缓冲区
        std::vector<Token> _tokens;
        std::size_t _offset;
        std::vector<Instruction> _instructions;
        std::pair<uint64_t, uint64_t> _current_pos;
        std::optional<Token> peek;

        std::optional<Token> nextToken();
		void unreadToken();

		// 符号表
        std::vector<Symbol> _symbols;
        std::vector<Func> _functions;
        int32_t _nextStackIndex;
        std::vector<int> _lastSymbolTable;
        std::vector<int> _lastIndex;

        void _addVar(const Token&, SymbolType type, bool isConst, bool isInit, int16_t funInd);
		int _findSymbol(const std::string&);    // return index in symbol table

		void addVariable(const Token&, SymbolType);
        void addConstant(const Token&, SymbolType);
        void addUninitializedVariable(const Token&, SymbolType);
        int addFunction(const Token&, SymbolType);     // return function index
        void addFuncPara(int funcId, SymbolType);

        int getFuncParaSize(const std::string&);
        SymbolType getSymbolType(const std::string&);

        void setSymbolTable();
		void resetSymbolTable();

        void initVariable(const std::string&);
        bool isDeclared(const std::string&);
        bool isConstant(const std::string&);
        bool isUninitializedVariable(const std::string &);
        bool isFunction(const std::string&);
        bool isLocal(const std::string &);
		int32_t getStackIndex(const std::string&);

        std::string getTempName();
        void addInstruction(Operation opr, const std::string& x);
        void addInstruction(Operation opr, const std::string& x, const std::string& y);
        void addInstruction(Operation opr, const std::string& x, const std::string& y, const std::string& r);

	};
}
