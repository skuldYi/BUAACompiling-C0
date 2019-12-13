#include "analyser.h"

#include <climits>
#define makeCE(ErrCode) std::make_optional<CompilationError>(_current_pos, ErrCode)
#define debugOut(s) std::cout << s << std::endl

namespace miniplc0 {
    bool mismatchType(std::optional<Token> token, TokenType type) {
        return (!token.has_value() || token.value().GetType() != type);
    }

	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		peek = nextToken();
		setSymbolTable();
	    auto err = analyseProgram();
        for (Symbol s : _symbols) {
            std::cout << s.toString() << std::endl;
        }
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::=
    //    {<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {
		auto err = analyseVariableDeclaration();
		if (err.has_value())
			return err;

//		err = analyseFunctionDefinition();
//		if (err.has_value())
//			return err;

		return {};
	}

	// <variable-declaration> ::=
    //    [<const-qualifier>]<type-specifier>
    //    <identifier>['='<expression> ]
    //    {','<identifier>['='<expression> ]}

    // <function-definition> ::=
    // <type-specifier><identifier> '(' ...

    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		while (true) {
		    if (!peek.has_value())
                return {};

		    bool isConst = false;
		    SymbolType type;

            if (peek.value().GetType() == TokenType::CONST) {
                isConst = true;
                peek = nextToken();
                if (!peek.has_value())
                    return makeCE(ErrorCode::ErrIncompleteExpression);
            }

            // whether is function definition
            auto typeSpe = peek;
            auto id = nextToken();
            if (!id.has_value())
                return makeCE(ErrorCode::ErrIncompleteExpression);

            peek = nextToken();
            if (!isConst && !mismatchType(peek, TokenType::LEFT_PAREN)) {
                unreadToken();
                unreadToken();
                peek = typeSpe;
                return {};
            }


            // typeSpe must have value
            switch (typeSpe.value().GetType()) {
                case TokenType::INT:
                case TokenType::CHAR:
                    type = SymbolType::Int;
                    break;
                case TokenType::DOUBLE:
                    type = SymbolType::Double;
                    break;
                case TokenType::VOID:
                    return makeCE(ErrorCode::ErrVoidVariable);
                default:
                    return makeCE(ErrorCode::ErrNeedTypeSpecifier);
            }

            peek = id;
            unreadToken();
            while(true) {
                id = peek;
                if (mismatchType(id, TokenType::IDENTIFIER)) {
                    return makeCE(ErrorCode::ErrNeedIdentifier);
                }

                if (isLocal(id.value().GetValueString()))
                    return makeCE(ErrorCode::ErrDuplicateDeclaration);

                // char next to id
                peek = nextToken();
                if (!peek.has_value())
                    return makeCE(ErrorCode::ErrIncompleteExpression);

                if (peek.value().GetType() != TokenType::ASSIGN_SIGN) {
                    if (isConst)
                        return makeCE(ErrorCode::ErrConstantNeedValue);
                    else
                        addUninitializedVariable(id.value(), type);
                    // 如果变量没有初始化也需要往栈里压入一个值
                    // 保证符号表中的偏移与栈中值位置对应
                    _instructions.emplace_back(Operation::LIT, 0);
                } else {
                    peek = nextToken();
                    if (isConst)
                        addConstant(id.value(), type);
                    else
                        addVariable(id.value(), type);
                    peek = nextToken(); // pretend to analyse expression
//                    auto err = analyseExpression();
//                    if (err.has_value())
//                        return err;
                    // 初始化变量的值在解析表达式后压栈
                }

                if (mismatchType(peek, TokenType::COMMA))
                    break;
                else
                    peek = nextToken();
            }

            // ';'
            if (mismatchType(peek, TokenType::SEMICOLON))
                return makeCE(ErrorCode::ErrNoSemicolon);
            peek = nextToken();
        }
	}


	// <function-definition> ::=
    //    <type-specifier><identifier><parameter-clause><compound-statement>
    //
    // <parameter-clause> ::=
    //    '(' [<parameter-declaration-list>] ')'
    // <parameter-declaration-list> ::=
    //    <parameter-declaration>{','<parameter-declaration>}
    // <parameter-declaration> ::=
    //    [<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseFunctionDefinition() {
        return std::optional<CompilationError>();
    }

    // <compound-statement> ::=
    //    '{' {<variable-declaration>} <statement-seq> '}'
    std::optional<CompilationError> Analyser::analyseCompoundStatement() {
        return std::optional<CompilationError>();
    }


    // <statement-seq> ::=
    //	{<statement>}
    //<statement> ::=
    //     <compound-statement>|<condition-statement>|<loop-statement>
    //     |<jump-statement>|<print-statement>|<scan-statement>
    //    |<assignment-expression>';'|<function-call>';'|';'
    std::optional<CompilationError> Analyser::analyseStatementSequence() {
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};
            unreadToken();
            if (next.value().GetType() != TokenType::IDENTIFIER &&
                next.value().GetType() != TokenType::PRINT &&
                next.value().GetType() != TokenType::SEMICOLON) {
                return {};
                // 语句序列可以包含 0 个语句
                // 如果没有语句直接返回，不报错
            }
            std::optional<CompilationError> err;
            switch (next.value().GetType()) {
                // 这里需要你针对不同的预读结果来调用不同的子程序
                // 注意我们没有针对空语句单独声明一个函数，因此可以直接在这里返回
                case TokenType::IDENTIFIER:
                    err = analyseAssignmentStatement();
                    if (err.has_value())
                        return err;
                    break;

                case TokenType::PRINT:
                    err = analysePrintStatement();
                    if (err.has_value())
                        return err;
                    break;

                case TokenType::SEMICOLON:
                    // 把这个分号读掉
                    nextToken();
                    break;

                default:
                    break;
            }
        }
    }

    // <condition-statement> ::=
    //     'if' '(' <condition> ')' <statement> ['else' <statement>]
    //    |'switch' '(' <expression> ')' '{' {<labeled-statement>} '}'
    std::optional<CompilationError> Analyser::analyseConditionStatement() {
        return std::optional<CompilationError>();
    }

    std::optional<CompilationError> Analyser::analyseLoopStatement() {
        return std::optional<CompilationError>();
    }

    // <jump-statement> ::=     ('break'|'continue'|'return' [<expression>]) ';'
    std::optional<CompilationError> Analyser::analyseJumpStatement() {
        return std::optional<CompilationError>();
    }

    // <print-statement> ::=
    //    'print' '(' [<printable-list>] ')' ';'
    //<printable-list>  ::=
    //    <printable> {',' <printable>}
    //<printable> ::=
    //    <expression> | <string-literal>
    std::optional<CompilationError> Analyser::analysePrintStatement() {
        // 如果之前 <语句序列> 的实现正确，这里第一个 next 一定是 TokenType::PRINT
        auto next = nextToken();

        // '('
        next = nextToken();
        if (mismatchType(next, TokenType::LEFT_PAREN))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        // <表达式>
        auto err = analyseExpression();
        if (err.has_value())
            return err;

        // ')'
        next = nextToken();
        if (mismatchType(next, TokenType::RIGHT_PAREN))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        // ';'
        next = nextToken();
        if (mismatchType(next, TokenType::SEMICOLON))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

        // 生成相应的指令 WRT
        _instructions.emplace_back(Operation::WRT, 0);
        return {};
    }

    // <scan-statement> ::=
    //    'scan' '(' <identifier> ')' ';'
    std::optional<CompilationError> Analyser::analyseScanStatement() {
        return std::optional<CompilationError>();
    }

    // <assignment-expression> ::= <identifier> '=' <expression>
    std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
        // 从语句序列处跳转，一定是标识符
        auto next = nextToken();
        std::string id = next.value().GetValueString();

        // 标识符声明过吗？标识符是常量吗？
        if (!isDeclared(id))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
        if (isConstant(id))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);

        // =
        next = nextToken();
        if (mismatchType(next, TokenType::ASSIGN_SIGN))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

        // <表达式>
        auto err = analyseExpression();
        if (err.has_value())
            return err;

        // ;
        next = nextToken();
        if (mismatchType(next, TokenType::SEMICOLON))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

        if (isUninitializedVariable(id))
            initVariable(id);
        _instructions.emplace_back(Operation::STO, getStackIndex(id));

        return {};
    }

    // <function-call> ::=
    //    <identifier> '(' [<expression-list>] ')'
    std::optional<CompilationError> Analyser::analyseFunctionCall() {
        return std::optional<CompilationError>();
    }

    // <condition> ::=
    //     <expression>[<relational-operator><expression>]
    //
    //<condition-statement> ::=
    //     'if' '(' <condition> ')' <statement> ['else' <statement>]
    //    |'switch' '(' <expression> ')' '{' {<labeled-statement>} '}'
    //
    //<labeled-statement> ::=
    //     'case' (<integer-literal>|<char-literal>) ':' <statement>
    //    |'default' ':' <statement>
    std::optional<CompilationError> Analyser::analyseCondition() {
        return std::optional<CompilationError>();
    }

	// <expression> ::= <Term>{<additive-operator><Term>}
	std::optional<CompilationError> Analyser::analyseExpression() {
		// <项>
		auto err = analyseTerm();
		if (err.has_value())
			return err;

		// {<加法型运算符><项>}
		while (true) {
			// 预读
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto type = next.value().GetType();
			if (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {
				unreadToken();
				return {};
			}

			// <项>
			err = analyseTerm();
			if (err.has_value())
				return err;

			// 根据结果生成指令
			if (type == TokenType::PLUS_SIGN)
				_instructions.emplace_back(Operation::ADD, 0);
			else if (type == TokenType::MINUS_SIGN)
				_instructions.emplace_back(Operation::SUB, 0);
		}
	}


	// <Term> ::= <Factor>{<multiplicative-operator><Factor>}
	std::optional<CompilationError> Analyser::analyseTerm() {
        // <因子>
        auto err = analyseFactor();
        if (err.has_value())
            return err;

        // {<乘法型运算符><因子>}
        while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};
            auto type = next.value().GetType();
            if (type != TokenType::MULTIPLICATION_SIGN && type != TokenType::DIVISION_SIGN) {
                unreadToken();
                return {};
            }

            // <因子>
            err = analyseFactor();
            if (err.has_value())
                return err;

            // 根据结果生成指令
            if (type == TokenType::MULTIPLICATION_SIGN)
                _instructions.emplace_back(Operation::MUL, 0);
            else if (type == TokenType::DIVISION_SIGN)
                _instructions.emplace_back(Operation::DIV, 0);
        }
	}

	// <Factor> ::= {'('<type-specifier>')'}[<unary-operator>]<primary-expression>
	std::optional<CompilationError> Analyser::analyseFactor() {
		// [<符号>]
		auto next = nextToken();
		auto prefix = 1;
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if (next.value().GetType() == TokenType::PLUS_SIGN)
			prefix = 1;
		else if (next.value().GetType() == TokenType::MINUS_SIGN) {
			prefix = -1;
			_instructions.emplace_back(Operation::LIT, 0);
		}
		else
			unreadToken();

		// 预读
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		int32_t index, integer;
		std::optional<CompilationError> err;
		switch (next.value().GetType()) {
            case TokenType::IDENTIFIER:
                // 未定义的变量不能参与表达式的运算
                if (!isDeclared(next.value().GetValueString()))
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);

                // 未初始化的变量不能参与表达式的运算
                if (isUninitializedVariable(next.value().GetValueString()))
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);

                index = getStackIndex(next.value().GetValueString());
                _instructions.emplace_back(Operation::LOD, index);
                break;

		    case TokenType::UNSIGNED_INTEGER:
		        integer = std::any_cast<int32_t>(next.value().GetValue());
                _instructions.emplace_back(Operation::LIT, integer);
                break;

		    case TokenType::LEFT_PAREN:
		        err = analyseExpression();
		        if (err.has_value())
                    return err;

		        next = nextToken();
		        if (mismatchType(next, TokenType::RIGHT_PAREN))
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
                break;
			// 这里和 <语句序列> 类似，需要根据预读结果调用不同的子程序
			// 但是要注意 default 返回的是一个编译错误
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		// 取负
		if (prefix == -1)
			_instructions.emplace_back(Operation::SUB, 0);
		return {};
	}

	// <primary-expression> ::=
    //     '('<expression>')'|<identifier>
    //     |<integer-literal>|<char-literal>|<function-call>
    std::optional<CompilationError> Analyser::analysePrimaryExpression(int32_t &out) {
        return std::optional<CompilationError>();
    }

    std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

	void Analyser::_addVar(const Token& tk, SymbolType type, bool isConst, bool isInit, int16_t funInd) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");

		_symbols.emplace_back(tk.GetValueString(), _nextTokenIndex, type, isConst, isInit, funInd);
		_nextTokenIndex++;
	}

    int Analyser::_findSymbol(const std::string & str) {
        if (_symbols.empty())
            return -1;
	    auto begin = _symbols.begin();
        for (auto it = _symbols.end() - 1; it >= begin; it--) {
            if (it->getName() == str)
                return std::distance(begin, it);
        }
        return -1;
    }

	void Analyser::addVariable(const Token& tk, SymbolType type) {
        _addVar(tk, type, false, true, -1);
	}

	void Analyser::addConstant(const Token& tk, SymbolType type) {
        _addVar(tk, type, true, true, -1);
    }

	void Analyser::addUninitializedVariable(const Token& tk, SymbolType type) {
        _addVar(tk, type, false, false, -1);
    }

    int Analyser::addFunction(const Token & tk, SymbolType type) {
	    int16_t funInd = _functions.size();
	    _addVar(tk, SymbolType::String, true, true, funInd);

	    struct function_struct function;
	    function.level = 1;
	    function.returnType = type;
	    function.para_size = 0;
	    _functions.push_back(function);

        return funInd;
    }

    void Analyser::addFuncPara(int funcId, const std::string &name, SymbolType type) {
        _functions[funcId].paraSeq.emplace_back(type, name);
        _functions[funcId].para_size++;
    }

    void Analyser::setSymbolTable() {
        _symbolTableSizes.push_back(_symbols.size());
    }

    void Analyser::resetSymbolTable() {
        int size = _symbolTableSizes.back();
        _symbolTableSizes.pop_back();
        _symbols.erase(_symbols.begin() + size, _symbols.end());
    }

    void Analyser::initVariable(const std::string & id) {
	    _symbols[_findSymbol(id)].setInited(true);
    }

    bool Analyser::isDeclared(const std::string& s) {
        return _findSymbol(s) > -1;
	}

    bool Analyser::isConstant(const std::string&s) {
        return !isFunction(s) && _symbols[_findSymbol(s)].isConst();
    }

    bool Analyser::isUninitializedVariable(const std::string &s){
        return !isFunction(s) && !_symbols[_findSymbol(s)].isInited();
    }

    bool Analyser::isFunction(const std::string &s) {
        return _symbols[_findSymbol(s)].getFuncIndex() > -1;
    }

    bool Analyser::isLocal(const std::string &s) {
        return _findSymbol(s) >= _symbolTableSizes.back();
    }

    int32_t Analyser::getStackIndex(const std::string& id) {
        return _symbols[_findSymbol(id)].getStackIndex();
    }

    bool Analyser::isTypeSpecifier(const Token &tk) {
        TokenType type = tk.GetType();
	    return type == TokenType::VOID || type == TokenType::INT||
	           type == TokenType::CHAR || type == TokenType::DOUBLE;
    }
}