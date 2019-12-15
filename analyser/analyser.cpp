#include "analyser.h"

#include <climits>
#define makeCE(ErrCode) std::make_optional<CompilationError>(_current_pos, ErrCode)
#define debugOut(s) std::cout << s << std::endl

namespace miniplc0 {
    bool mismatchType(std::optional<Token> token, TokenType type) {
        return (!token.has_value() || token.value().GetType() != type);
    }

    std::optional<SymbolType> specifierType(const std::optional<Token>& tk) {
        if (!tk.has_value())
            return {};
        SymbolType type;
        switch (tk.value().GetType()) {
            case TokenType::CHAR:
                type = SymbolType::Char;
                break;
            case TokenType::INT:
                type = SymbolType::Int;
                break;
            case TokenType::DOUBLE:
                type = SymbolType::Double;
                break;
            case TokenType::VOID:
                type = SymbolType::Void;
                break;

            default:
                return {};
        }
        return type;
    }

    bool isStatementFirst(const std::optional<Token>& tk) {
        if (!tk.has_value())
            return false;
        TokenType type = tk.value().GetType();
        return   type == TokenType::LEFT_BRACE || type == TokenType::IF  || type == TokenType::WHILE
              || type == TokenType::RETURN  || type == TokenType::PRINT  || type == TokenType::SCAN
              || type == TokenType::IDENTIFIER  || type == TokenType::SEMICOLON;
    }

    // '<' | '<=' | '>' | '>=' | '!=' | '=='
    bool isRelOpr(const std::optional<Token>& tk) {
        if (!tk.has_value())
            return false;
        TokenType type = tk.value().GetType();
        return   type == TokenType::LESS_SIGN || type == TokenType::LESSEQUAL_SIGN
              || type == TokenType::GREATER_SIGN  || type == TokenType::GREATEREQUAL_SIGN
              || type == TokenType::EQUAL_SIGN  || type == TokenType::NOTEQUAL_SIGN;
    }

	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::=
    //    {<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {
        debugOut("analyse program");
        setSymbolTable();
        peek = nextToken();

        auto err = analyseVariableDeclaration();
		if (err.has_value())
			return err;

		err = analyseFunctionDefinition();
		if (err.has_value())
			return err;

		return {};
	}

	// <variable-declaration> ::=
    //    [<const-qualifier>]<type-specifier>
    //    <identifier>['='<expression> ] {','<identifier>['='<expression> ]}

    // <function-definition> ::=
    // <type-specifier><identifier> '(' ...

    // <compound-statement> ::=
    //    '{' {<variable-declaration>} {<statement>} '}'

    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		while (true) {
		    if (isStatementFirst(peek) || !mismatchType(peek, TokenType::RIGHT_BRACE))
                return {};

            debugOut("analyse var declaration");

            bool isConst = false;

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

            auto type = specifierType(typeSpe);
            if (!type.has_value())
                return makeCE(ErrorCode::ErrNeedTypeSpecifier);

            if (type.value() == SymbolType::Void)
                return makeCE(ErrorCode::ErrVoidVariable);

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
                        addUninitializedVariable(id.value(), type.value());
                } else {
                    if (isConst)
                        addConstant(id.value(), type.value());
                    else
                        addVariable(id.value(), type.value());

                    peek = nextToken();
                    auto err = analyseExpression();
                    if (err.has_value())
                        return err;
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
    //    <type-specifier><identifier> '(' [<parameter-declaration-list>] ')' <compound-statement>
    //
    // <parameter-declaration-list> ::=
    //    <parameter-declaration>{','<parameter-declaration>}
    // <parameter-declaration> ::=
    //    [<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseFunctionDefinition() {
        while (true) {
            if (!peek.has_value())
                return {};

            debugOut("analyse func definition");

            auto type = specifierType(peek);
            if (!type.has_value())
                return makeCE(ErrorCode::ErrNeedTypeSpecifier);
            peek = nextToken();

            if (mismatchType(peek, TokenType::IDENTIFIER))
                return makeCE(ErrorCode::ErrNeedIdentifier);
            auto id = peek.value();
            peek = nextToken();

            // 函数名的作用域是其被声明的作用域
            // 函数的参数名或局部变量名作用域是函数体内部
            int funId = addFunction(id, type.value());
            setSymbolTable();
            // symbol table will be reset in analyse compound statement


            if (mismatchType(peek, TokenType::LEFT_PAREN)) {
                return makeCE(ErrorCode::ErrIncompleteExpression);
            }
            peek = nextToken();

            if (mismatchType(peek, TokenType::RIGHT_PAREN)) {
                // <parameter-declaration>{','<parameter-declaration>}
                // <parameter-declaration> ::=
                //    [<const-qualifier>]<type-specifier><identifier>
                while (true) {
                    bool isConst = false;
                    if (!mismatchType(peek, TokenType::CONST)) {
                        isConst = true;
                        peek = nextToken();
                    }

                    type = specifierType(peek);
                    if (!type.has_value())
                        return makeCE(ErrorCode::ErrNeedTypeSpecifier);
                    if (type.value() == SymbolType::Void)
                        return makeCE(ErrorCode::ErrVoidVariable);
                    peek = nextToken();

                    if (mismatchType(peek, TokenType::IDENTIFIER))
                        return makeCE(ErrorCode::ErrNeedIdentifier);
                    id = peek.value();
                    peek = nextToken();

                    addFuncPara(funId, type.value());
                    if (isConst)
                        addConstant(id, type.value());
                    else
                        addVariable(id, type.value());

                    if (mismatchType(peek, TokenType::COMMA))
                        break;
                    else
                        peek = nextToken();
                }
            }

            if (mismatchType(peek, TokenType::RIGHT_PAREN)) {
                return makeCE(ErrorCode::ErrIncompleteExpression);
            }
            peek = nextToken();

            auto err = analyseCompoundStatement(true);
            if (err.has_value())
                return err;
        }

        // todo: 不能在返回类型为`void`的函数中使用有值的返回语句，也不能在非`void`函数中使用无值的返回语句。
    }

    // <statement> ::=
    //     <compound-statement>   // {
    //    |<condition-statement>  // if
    //    |<loop-statement>       // while
    //    |<jump-statement>       // return
    //    |<print-statement>      // print
    //    |<scan-statement>       // scan
    //    |<assignment-expression>';' // <id> '='
    //    |<function-call>';'     // <id> '('
    //    |';'
    std::optional<CompilationError> Analyser::analyseStatement(){
        if (!isStatementFirst(peek))
            return makeCE(ErrorCode::ErrNeedStatement);

//        debugOut("analyse statement");

        std::optional<CompilationError> err = {};
        auto next = peek;   // get token type

        // wrong peek will not reach here
        switch (peek.value().GetType()) {
            case TokenType::LEFT_BRACE:
                err = analyseCompoundStatement(false);
                break;

            case TokenType::IF:
                err = analyseConditionStatement();
                break;

            case TokenType::WHILE:
                err = analyseLoopStatement();
                break;

            case TokenType::RETURN:
                err = analyseJumpStatement();
                break;

            case TokenType::PRINT:
                err = analysePrintStatement();
                break;

            case TokenType::SCAN:
                err = analyseScanStatement();
                break;

            case TokenType::SEMICOLON:
                peek = nextToken();
                break;

            case TokenType::IDENTIFIER:
                next = nextToken();
                unreadToken();
                if (!next.has_value())
                    err = makeCE(ErrorCode::ErrIncompleteExpression);
                else if (next.value().GetType() == TokenType::ASSIGN_SIGN)
                    err = analyseAssignmentStatement();
                else if (next.value().GetType() == TokenType::LEFT_PAREN)
                    err = analyseFunctionCall();
                else
                    err = makeCE(ErrorCode::ErrSyntaxError);

                if (!err.has_value()) {
                    if (mismatchType(peek, TokenType::SEMICOLON))
                        err = makeCE(ErrorCode::ErrNoSemicolon);
                    else
                        peek = nextToken(); // avoid nextToken after EOF
                }

                break;

            default:
                err = makeCE(ErrorCode::ErrNeedStatement);
        }

        return err;
    }

    // '{' {<variable-declaration>} {<statement>} '}'

    std::optional<CompilationError> Analyser::analyseCompoundStatement(bool funcBody) {
        debugOut("analyse compound statement");
        if (!funcBody)
            setSymbolTable();

        if (mismatchType(peek, TokenType::LEFT_BRACE))
            return makeCE(ErrorCode::ErrMissingBrace);
        peek = nextToken();

        auto err = analyseVariableDeclaration();
        if (err.has_value())
            return err;

        // statement-seq
        while (isStatementFirst(peek)) {
            err = analyseStatement();
            if (err.has_value())
                return err;
        }

        if (mismatchType(peek, TokenType::RIGHT_BRACE))
            return makeCE(ErrorCode::ErrMissingBrace);
        peek = nextToken();

        resetSymbolTable();
        return {};
    }

    // <expression>[<relational-operator><expression>]
    std::optional<CompilationError> Analyser::analyseCondition() {
        debugOut("analyse condition");

        auto err = analyseExpression();
        if (err.has_value())
            return err;

        if (isRelOpr(peek)) {
            auto relation = peek.value();
            peek = nextToken();

            err = analyseExpression();
            if (err.has_value())
                return err;
        }

        return {};
    }

    // 'if' '(' <condition> ')' <statement> ['else' <statement>]
    std::optional<CompilationError> Analyser::analyseConditionStatement() {
        debugOut("analyse condition statement");

        if (mismatchType(peek, TokenType::IF))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        if (mismatchType(peek, TokenType::LEFT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        auto err = analyseCondition();
        if (err.has_value())
            return err;

        if (mismatchType(peek, TokenType::RIGHT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        err = analyseStatement();
        if (err.has_value())
            return err;

        if (!mismatchType(peek, TokenType::ELSE)) {
            peek = nextToken();
            err = analyseStatement();
            if (err.has_value())
                return err;
        }

        return std::optional<CompilationError>();
    }

    // 'while' '(' <condition> ')' <statement>
    std::optional<CompilationError> Analyser::analyseLoopStatement() {
        debugOut("analyse loop statement");

        if (mismatchType(peek, TokenType::WHILE))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        if (mismatchType(peek, TokenType::LEFT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        auto err = analyseCondition();
        if (err.has_value())
            return err;

        if (mismatchType(peek, TokenType::RIGHT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        err = analyseStatement();
        if (err.has_value())
            return err;

        return std::optional<CompilationError>();
    }

    // 'return' [<expression>] ';'
    std::optional<CompilationError> Analyser::analyseJumpStatement() {
        debugOut("analyse jump statement");

        if (mismatchType(peek, TokenType::RETURN))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        if (mismatchType(peek, TokenType::SEMICOLON)) {
            auto err = analyseExpression();
            if (err.has_value())
                return err;
        }

        if (mismatchType(peek, TokenType::SEMICOLON))
            return makeCE(ErrorCode::ErrNoSemicolon);
        peek = nextToken();

        return std::optional<CompilationError>();
    }

    // 'print' '(' [<printable-list>] ')' ';'
    std::optional<CompilationError> Analyser::analysePrintStatement() {
        debugOut("analyse print statement");

        if (mismatchType(peek, TokenType::PRINT))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        if (mismatchType(peek, TokenType::LEFT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        if (mismatchType(peek, TokenType::RIGHT_PAREN)) {
            // <printable> {',' <printable>}
            // <printable> ::= <expression> | <string-literal>
            while (true) {
                if (!mismatchType(peek, TokenType::STRING)) {
                    // do something
                } else {
                    auto err = analyseExpression();
                    if (err.has_value())
                        return err;
                }

                if (mismatchType(peek, TokenType::COMMA))
                    break;
                else
                    peek = nextToken();
            }
        }

        if (mismatchType(peek, TokenType::RIGHT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        if (mismatchType(peek, TokenType::SEMICOLON))
            return makeCE(ErrorCode::ErrNoSemicolon);
        peek = nextToken();

        return {};
    }

    // 'scan' '(' <identifier> ')' ';'
    std::optional<CompilationError> Analyser::analyseScanStatement() {
        debugOut("analyse scan statement");

        if (mismatchType(peek, TokenType::SCAN))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        if (mismatchType(peek, TokenType::LEFT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        if (mismatchType(peek, TokenType::IDENTIFIER))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        std::string id = peek.value().GetValueString();
        peek = nextToken();

        if (!isDeclared(id))
            return makeCE(ErrorCode::ErrNotDeclared);
        if (isConstant(id))
            return makeCE(ErrorCode::ErrAssignToConstant);

        if (mismatchType(peek, TokenType::RIGHT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        if (mismatchType(peek, TokenType::SEMICOLON))
            return makeCE(ErrorCode::ErrNoSemicolon);
        peek = nextToken();

        return std::optional<CompilationError>();
    }

    // <assignment-expression> ::= <identifier> '=' <expression>
    std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
        debugOut("analyse assignment statement");

        if (mismatchType(peek, TokenType::IDENTIFIER))
            return makeCE(ErrorCode::ErrSyntaxError);
        std::string id = peek.value().GetValueString();
        peek = nextToken();

        if (mismatchType(peek, TokenType::ASSIGN_SIGN))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        if (!isDeclared(id))
            return makeCE(ErrorCode::ErrNotDeclared);
        if (isConstant(id))
            return makeCE(ErrorCode::ErrAssignToConstant);

        auto err = analyseExpression();
        if (err.has_value())
            return err;

        if (isUninitializedVariable(id))
            initVariable(id);

        return {};
    }

    // <identifier> '(' [<expression-list>] ')'
    // <expression-list> ::= <expression>{','<expression>}
    std::optional<CompilationError> Analyser::analyseFunctionCall() {
        debugOut("analyse func call");

        if (mismatchType(peek, TokenType::IDENTIFIER))
            return makeCE(ErrorCode::ErrSyntaxError);
        std::string id = peek.value().GetValueString();
        peek = nextToken();

        if (!isFunction(id))
            return makeCE(ErrorCode::ErrFunctionNotDefined);

        if (mismatchType(peek, TokenType::LEFT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        int paraNum = 0;
        if (mismatchType(peek, TokenType::RIGHT_PAREN)) {
            // <expression>{','<expression>}
            while (true) {
                auto err = analyseExpression();
                if (err.has_value())
                    return err;
                paraNum++;

                if (mismatchType(peek, TokenType::COMMA))
                    break;
                else
                    peek = nextToken();
            }
        }

        // all vars are 1 slot long, so paraNum == paraSize
        if (paraNum != getFuncParaSize(id))
            return makeCE(ErrorCode::ErrInvalidFunctionCall);

        if (mismatchType(peek, TokenType::RIGHT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        return std::optional<CompilationError>();
    }

	// <Term>{<additive-operator><Term>}
	std::optional<CompilationError> Analyser::analyseExpression() {
        debugOut("Expression");

        auto err = analyseTerm();
		if (err.has_value())
			return err;

		while (!mismatchType(peek, TokenType::PLUS_SIGN)
		    || !mismatchType(peek, TokenType::MINUS_SIGN)) {

			auto type = peek.value().GetType();
			peek = nextToken();

			err = analyseTerm();
			if (err.has_value())
				return err;

//			if (type == TokenType::PLUS_SIGN)
//				_instructions.emplace_back(Operation::ADD, 0);
//			else if (type == TokenType::MINUS_SIGN)
//				_instructions.emplace_back(Operation::SUB, 0);
		}

        return {};
	}

	// <Factor>{<multiplicative-operator><Factor>}
	std::optional<CompilationError> Analyser::analyseTerm() {
        debugOut("Term");

        auto err = analyseFactor();
        if (err.has_value())
            return err;

        while (!mismatchType(peek, TokenType::MULTIPLICATION_SIGN)
            || !mismatchType(peek, TokenType::DIVISION_SIGN)) {

            auto type = peek.value().GetType();
            peek = nextToken();

            err = analyseTerm();
            if (err.has_value())
                return err;

//            if (type == TokenType::MULTIPLICATION_SIGN)
//                _instructions.emplace_back(Operation::MUL, 0);
//            else if (type == TokenType::DIVISION_SIGN)
//                _instructions.emplace_back(Operation::DIV, 0);
        }

        return {};
	}

	// <Factor> ::= [<unary-operator>]
    //     '('<expression>')'
    //    |<identifier>
    //    |<integer-literal>
    //    |<char-literal>
    //    |<function-call>
	std::optional<CompilationError> Analyser::analyseFactor() {
        debugOut("Factor");

        if (!peek.has_value())
            return makeCE(ErrorCode::ErrIncompleteExpression);
        auto prefix = 1;

        if (peek.value().GetType() == TokenType::PLUS_SIGN) {
            prefix = 1;
            peek = nextToken();
        } else if (peek.value().GetType() == TokenType::MINUS_SIGN) {
			prefix = -1;
			peek = nextToken();
		}

        if (!peek.has_value())
            return makeCE(ErrorCode::ErrIncompleteExpression);

		std::optional<CompilationError> err;
		std::string id;
		long l;
		int i;
		auto next = peek;

		switch (peek.value().GetType()) {
            case TokenType::LEFT_PAREN:
                peek = nextToken();
                err = analyseExpression();
                if (!err.has_value()) {
                    if (mismatchType(peek, TokenType::RIGHT_PAREN))
                        err = makeCE(ErrorCode::ErrIncompleteExpression);
                    else
                        peek = nextToken(); // avoid nextToken after EOF
                }
                break;

            case TokenType::IDENTIFIER:
                next = nextToken();
                if (!mismatchType(next, TokenType::LEFT_PAREN)) {
                    unreadToken();
                    err = analyseFunctionCall();
                } else {
                    id = peek.value().GetValueString();
                    peek = next;

                    if (!isDeclared(id))
                        err = makeCE(ErrorCode::ErrNotDeclared);
                    else if (isFunction(id))
                        err = makeCE(ErrorCode::ErrInvalidFunctionCall);
                    else if (isUninitializedVariable(id))
                        err = makeCE(ErrorCode::ErrNotInitialized);
                    else {
//                        index = getStackIndex(next.value().GetValueString());
//                        _instructions.emplace_back(Operation::LOD, index);
                    }
                }
                break;

		    case TokenType::UNSIGNED_INTEGER:
		        l = std::any_cast<unsigned long>(peek.value().GetValue());
		        if (l - 1 > INT32_MAX || (l - 1 == INT32_MAX && prefix == 1))
		            err = makeCE(ErrorCode::ErrIntegerOverflow);
//                _instructions.emplace_back(Operation::LIT, integer);

		        peek = nextToken();
                break;

            case TokenType::UNSIGNED_CHAR:
                i = (int)std::any_cast<char>(peek.value().GetValue());
                // do something
                peek = nextToken();
                break;

            default:
                err = makeCE(ErrorCode::ErrIncompleteExpression);
		}
		if (err.has_value())
            return err;

		if (prefix == -1) {
//			_instructions.emplace_back(Operation::SUB, 0);
        }
		return {};
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
        for (auto it = _symbols.end() - 1; it >= _symbols.begin(); it--) {
            if (it->getName() == str)
                return std::distance(_symbols.begin(), it);
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
	    _functions.emplace_back(type);

        return funInd;
    }

    void Analyser::addFuncPara(int funcId, SymbolType type) {
        _functions[funcId].addPara(type);
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
        if (isDeclared(id))
            _symbols[_findSymbol(id)].setInited(true);
    }

    inline bool Analyser::isDeclared(const std::string& s) {
        return _findSymbol(s) > -1;
	}

    bool Analyser::isConstant(const std::string&s) {
        return isDeclared(s) && !isFunction(s) && _symbols[_findSymbol(s)].isConst();
    }

    bool Analyser::isUninitializedVariable(const std::string &s){
        return isDeclared(s) && !isFunction(s) && !_symbols[_findSymbol(s)].isInited();
    }

    bool Analyser::isFunction(const std::string &s) {
        return isDeclared(s) && _symbols[_findSymbol(s)].getFuncIndex() > -1;
    }

    bool Analyser::isLocal(const std::string &s) {
        return _findSymbol(s) >= _symbolTableSizes.back();
    }

    int32_t Analyser::getStackIndex(const std::string& id) {
        return isDeclared(id) && _symbols[_findSymbol(id)].getStackIndex();
    }

    int Analyser::getFuncParaSize(const std::string &s) {
        if (!isFunction(s))
            return -1;
        int funcId = _symbols[_findSymbol(s)].getFuncIndex();

        return _functions[funcId].getParaSize();
    }
}