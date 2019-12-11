#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {
    enum symbolType {
        Int,
        String,
        Double
    };

    struct symbol_struct {
        std::string name;
        int32_t stackIndex;
        symbolType type;    // String for function
        bool isConstant;
        bool isInitialed;
        int16_t functionTableIndex;     // -1 for nonfunction
    };

    struct function_struct {
        int para_size;
        int level;
        symbolType returnType;
        std::vector<std::map<symbolType, std::string>> paraSeq;
    };

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
			_symbols({}), _symbolTableSizes({}), _functions({}), _nextTokenIndex(0) {}
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

		std::optional<CompilationError> analyseCompoundStatement();
		std::optional<CompilationError> analyseStatementSequence();
        std::optional<CompilationError> analyseConditionStatement();
        std::optional<CompilationError> analyseLoopStatement();
        std::optional<CompilationError> analyseJumpStatement();
        std::optional<CompilationError> analysePrintStatement();
        std::optional<CompilationError> analyseScanStatement();
        std::optional<CompilationError> analyseAssignmentStatement();
        std::optional<CompilationError> analyseFunctionCall();

		std::optional<CompilationError> analyseCondition();
		std::optional<CompilationError> analyseExpression();
		std::optional<CompilationError> analyseTerm();
		std::optional<CompilationError> analyseFactor();
        std::optional<CompilationError> analysePrimaryExpression(int32_t& out);

		// Token 缓冲区
        std::vector<Token> _tokens;
        std::size_t _offset;
        std::vector<Instruction> _instructions;
        std::pair<uint64_t, uint64_t> _current_pos;

        std::optional<Token> nextToken();
		void unreadToken();

		// 符号表
        std::vector<struct symbol_struct> _symbols;
        std::vector<int> _symbolTableSizes;
        std::vector<struct function_struct> _functions;

		void _addVar(const Token&, symbolType type, bool isConst, bool isInit, int16_t funInd);
		int _findSymbol(const std::string&);    // return index in symbol table

		void addVariable(const Token&, symbolType);
		void addConstant(const Token&, symbolType);
		void addUninitializedVariable(const Token&, symbolType);
		int addFunction(const Token&, symbolType);     // return function index
        void addFuncPara(int funcId, const std::string &, symbolType);

		void setSymbolTable();
		void resetSymbolTable();

		void initVariable(const std::string&);
		bool isDeclared(const std::string&);
        bool isConstant(const std::string&);
        bool isUninitializedVariable(const std::string &);
        bool isFunction(const std::string&);
        bool isLocal(const std::string &);
		int32_t getStackIndex(const std::string&);

		// 下一个 token 在栈的偏移
		int32_t _nextTokenIndex;
	};
}
