#include "analyser.h"

#include <climits>
#define makeCE(ErrCode) std::make_optional<CompilationError>(_current_pos, ErrCode)
#define debugOut(s) std::cout << s << std::endl

namespace c0 {
    bool mismatchType(std::optional<Token> token, TokenType type) {
        return (!token.has_value() || token.value().GetType() != type);
    }

    std::optional<SymbolType> specifierType(const std::optional<Token>& tk) {
        if (!tk.has_value())
            return {};
        SymbolType type;
        switch (tk.value().GetType()) {
//            case TokenType::CHAR:
//                type = SymbolType::Char;
//                break;
            case TokenType::INT:
                type = SymbolType::Int;
                break;
//            case TokenType::DOUBLE:
//                type = SymbolType::Double;
//                break;
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

	std::pair<std::vector<Quadruple>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Quadruple>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::=
    //    {<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {
//        debugOut("analyse program");
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

    // <function-definition> ::= <type-specifier><identifier> '(' ...
    // <compound-statement> ::= '{' {<variable-declaration>} {<statement>} '}'
    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		while (true) {
		    if (isStatementFirst(peek) || !mismatchType(peek, TokenType::RIGHT_BRACE))
                return {};

//            debugOut("analyse var declaration");

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
                    // uninitialized
                    if (isConst)
                        return makeCE(ErrorCode::ErrConstantNeedValue);
                    else
                        addUninitializedVariable(id.value(), type.value());
                } else {
                    // initialized
                    if (isConst)
                        addConstant(id.value(), type.value());
                    else
                        addVariable(id.value(), type.value());

                    std::string value;
                    peek = nextToken();
                    auto err = analyseExpression(value);
                    if (err.has_value())
                        return err;
                    addInstruction(QuadOpr::ASN, value, "", id.value().GetValueString());
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


	// <type-specifier><identifier> '(' [<parameter-declaration-list>] ')' <compound-statement>
    //
    // <parameter-declaration-list> ::=
    //    <parameter-declaration>{','<parameter-declaration>}
    // <parameter-declaration> ::=
    //    [<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseFunctionDefinition() {
        while (true) {
            if (!peek.has_value())
                return {};

//            debugOut("analyse func definition");

            auto funcType = specifierType(peek);
            if (!funcType.has_value())
                return makeCE(ErrorCode::ErrNeedTypeSpecifier);
            peek = nextToken();

            if (mismatchType(peek, TokenType::IDENTIFIER))
                return makeCE(ErrorCode::ErrNeedIdentifier);
            auto id = peek.value();
            std::string name = id.GetValueString();
            peek = nextToken();

            // 函数名的作用域是其被声明的作用域
            // 函数的参数名或局部变量名作用域是函数体内部
            if (isLocal(name))
                return makeCE(ErrorCode::ErrDuplicateDeclaration);

            int funId = addFunction(id, funcType.value());
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

                    auto type = specifierType(peek);
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
                    addPara(id, type.value(), isConst);

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


            addInstruction(QuadOpr::FUNC, "@" + name, "$" + std::to_string(getFuncParaSize(name)), "$1");

            bool returned;
            auto err = analyseCompoundStatement(true, returned);
            if (err.has_value())
                return err;

            if (funcType.value() != SymbolType::Void && !returned) {
                return makeCE(ErrorCode::ErrNeedReturnValue);
            }

            if (funcType.value() == SymbolType::Void && !returned)
                addInstruction(QuadOpr::RET, "");
        }
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
    std::optional<CompilationError> Analyser::analyseStatement(bool& returned){
        returned = false;
        if (!isStatementFirst(peek))
            return makeCE(ErrorCode::ErrNeedStatement);

        std::optional<CompilationError> err = {};
        auto next = peek;   // get token type
        std::string str = "";

        // wrong peek will not reach here
        switch (peek.value().GetType()) {
            case TokenType::LEFT_BRACE:
                err = analyseCompoundStatement(false, returned);
                break;

            case TokenType::IF:
                err = analyseConditionStatement(returned);
                break;

            case TokenType::WHILE:
                err = analyseLoopStatement();
                break;

            case TokenType::RETURN:
                err = analyseJumpStatement();
                returned = true;
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
                    err = analyseFunctionCall(str);
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
    std::optional<CompilationError> Analyser::analyseCompoundStatement(bool funcBody, bool& returned) {
//        debugOut("analyse compound statement");
        returned = false;

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
            bool stateReturned;
            err = analyseStatement(stateReturned);
            if (err.has_value())
                return err;
            returned |= stateReturned;
        }

        if (mismatchType(peek, TokenType::RIGHT_BRACE))
            return makeCE(ErrorCode::ErrMissingBrace);
        peek = nextToken();

        resetSymbolTable();
        return {};
    }

    // <expression>[<relational-operator><expression>]
    std::optional<CompilationError> Analyser::analyseCondition() {
//        debugOut("analyse condition");

        std::string opr1;
        auto err = analyseExpression(opr1);
        if (err.has_value())
            return err;

        if (isRelOpr(peek)) {
            auto relation = peek.value();
            peek = nextToken();

            std::string opr2;
            err = analyseExpression(opr2);
            if (err.has_value())
                return err;

            QuadOpr op;
            switch (relation.GetType()) {
                case TokenType::LESS_SIGN:
                    op = QuadOpr::LT;
                    break;
                case TokenType::LESSEQUAL_SIGN:
                    op = QuadOpr::LE;
                    break;
                case TokenType::GREATER_SIGN:
                    op = QuadOpr::GT;
                    break;
                case TokenType::GREATEREQUAL_SIGN:
                    op = QuadOpr::GE;
                    break;
                case TokenType::EQUAL_SIGN:
                    op = QuadOpr::EQU;
                    break;
                case TokenType::NOTEQUAL_SIGN:
                    op = QuadOpr::NE;
                    break;
                default:
                    return makeCE(ErrorCode::ErrSyntaxError);
            }

            addInstruction(op, opr1, opr2);
        } else {
            addInstruction(QuadOpr::NE, opr1, "$0");
        }

        return {};
    }

    // 'if' '(' <condition> ')' <statement> ['else' <statement>]
    std::optional<CompilationError> Analyser::analyseConditionStatement(bool& returned) {
        /*
         *  condition
         *  BZ #label-else
         *  {if-statement}
         *  #label-else
         *
         *  condition
         *  BZ #label-else
         *  {if-statement}
         *  GOTO #label-end
         *  #label-else
         *  {else-statement}
         *  #label-end
         * */

//        debugOut("analyse condition statement");
        std::string labelElse = getLabel();

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

        addInstruction(QuadOpr::BZ, labelElse);
        // if-statements
        setSymbolTable();
        err = analyseStatement(returned);
        resetSymbolTable();
        if (err.has_value())
            return err;

        bool elseReturned = false;
        if (!mismatchType(peek, TokenType::ELSE)) {
            std::string labelEnd = getLabel();
            addInstruction(QuadOpr::GOTO, labelEnd);
            addInstruction(QuadOpr::LAB, labelElse);
            peek = nextToken();
            setSymbolTable();
            err = analyseStatement(elseReturned);
            resetSymbolTable();
            if (err.has_value())
                return err;
            addInstruction(QuadOpr::LAB, labelEnd);
        } else {
            addInstruction(QuadOpr::LAB, labelElse);
        }
        returned &= elseReturned;

        return std::optional<CompilationError>();
    }

    // 'while' '(' <condition> ')' <statement>
    std::optional<CompilationError> Analyser::analyseLoopStatement() {
//        debugOut("analyse loop statement");
        /*  #label-begin
         *  condition
         *  BZ #label-end
         *  {while-statement}
         *  GOTO #label-begin
         *  #label-end
         * */
        std::string labelBegin = getLabel(), labelEnd = getLabel();

        if (mismatchType(peek, TokenType::WHILE))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        addInstruction(QuadOpr::LAB, labelBegin);

        if (mismatchType(peek, TokenType::LEFT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        auto err = analyseCondition();
        if (err.has_value())
            return err;

        if (mismatchType(peek, TokenType::RIGHT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        addInstruction(QuadOpr::BZ, labelEnd);

        // while-statement
        // not used, for while-statement may not be executed
        bool returned;
        setSymbolTable();
        err = analyseStatement(returned);
        resetSymbolTable();
        if (err.has_value())
            return err;

        addInstruction(QuadOpr::GOTO, labelBegin);
        addInstruction(QuadOpr::LAB, labelEnd);

        return std::optional<CompilationError>();
    }

    // 'return' [<expression>] ';'
    std::optional<CompilationError> Analyser::analyseJumpStatement() {
//        debugOut("analyse jump statement");

        if (mismatchType(peek, TokenType::RETURN))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        auto type = currentFuncType();
        bool hasRet = false;

        std::string value = "";
        if (mismatchType(peek, TokenType::SEMICOLON)) {
            if (type == SymbolType::Void)
                return makeCE(ErrorCode::ErrInvalidReturnValue);
            hasRet = true;

            auto err = analyseExpression(value);
            if (err.has_value())
                return err;
        }
        if (mismatchType(peek, TokenType::SEMICOLON))
            return makeCE(ErrorCode::ErrNoSemicolon);
        peek = nextToken();

        addInstruction(QuadOpr::RET, value);

        if (type != SymbolType::Void && !hasRet)
            return makeCE(ErrorCode::ErrInvalidReturnValue);

        return std::optional<CompilationError>();
    }

    // 'print' '(' [<printable-list>] ')' ';'
    std::optional<CompilationError> Analyser::analysePrintStatement() {
//        debugOut("analyse print statement");

        if (mismatchType(peek, TokenType::PRINT))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        if (mismatchType(peek, TokenType::LEFT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        if (mismatchType(peek, TokenType::RIGHT_PAREN)) {
            // <printable> {',' <printable>}
            // <printable> ::= <expression> | <string-literal> | <char-literal>
            while (true) {
                if (!mismatchType(peek, TokenType::STRING)) {
                    addInstruction(QuadOpr::PRT, "@" + peek.value().GetValueString(), "@s");
                    peek = nextToken();
                } else if (!mismatchType(peek, TokenType::UNSIGNED_CHAR)) {
                    char c = std::any_cast<char>(peek.value().GetValue());
                    addInstruction(QuadOpr::PRT, "$" + std::to_string(c), "@c");
                    peek = nextToken();
                } else {
                    std::string value;
                    auto err = analyseExpression(value);
                    if (err.has_value())
                        return err;
                    addInstruction(QuadOpr::PRT, value, "@i");
                }

                if (mismatchType(peek, TokenType::COMMA))
                    break;
                else {
                    peek = nextToken();
                    addInstruction(QuadOpr::PRT, "$" + std::to_string(' '), "@c");
                }
            }
        }
        addInstruction(QuadOpr::PRT, "", "@ln");


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
//        debugOut("analyse scan statement");

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
        if (isConstant(id) || isFunction(id))
            return makeCE(ErrorCode::ErrAssignToConstant);

        if (mismatchType(peek, TokenType::RIGHT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        if (mismatchType(peek, TokenType::SEMICOLON))
            return makeCE(ErrorCode::ErrNoSemicolon);
        peek = nextToken();

        addInstruction(QuadOpr::SCN, id);
        initVariable(id);
        return std::optional<CompilationError>();
    }

    // <assignment-expression> ::= <identifier> '=' <expression>
    std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
//        debugOut("analyse assignment statement");

        if (mismatchType(peek, TokenType::IDENTIFIER))
            return makeCE(ErrorCode::ErrSyntaxError);
        std::string id = peek.value().GetValueString();
        if (isFunction(id))
            return makeCE(ErrorCode::ErrNotDeclared);
        peek = nextToken();

        if (mismatchType(peek, TokenType::ASSIGN_SIGN))
            return makeCE(ErrorCode::ErrSyntaxError);
        peek = nextToken();

        if (!isDeclared(id))
            return makeCE(ErrorCode::ErrNotDeclared);
        if (isConstant(id))
            return makeCE(ErrorCode::ErrAssignToConstant);

        std::string value;
        auto err = analyseExpression(value);
        if (err.has_value())
            return err;

        addInstruction(QuadOpr::ASN, value, "", id);
        initVariable(id);

        return {};
    }

    // <identifier> '(' [<expression-list>] ')'
    // <expression-list> ::= <expression>{','<expression>}
    std::optional<CompilationError> Analyser::analyseFunctionCall(std::string& ret) {
//        debugOut("analyse func call");

        if (mismatchType(peek, TokenType::IDENTIFIER))
            return makeCE(ErrorCode::ErrSyntaxError);
        std::string id = peek.value().GetValueString();
        if (!isFunction(id))
            return makeCE(ErrorCode::ErrFunctionNotDefined);
        peek = nextToken();

        if (mismatchType(peek, TokenType::LEFT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        std::vector<std::string> paras;
        if (mismatchType(peek, TokenType::RIGHT_PAREN)) {
            // <expression>{','<expression>}
            while (true) {
                std::string para;
                auto err = analyseExpression(para);
                if (err.has_value())
                    return err;
                paras.push_back(para);

                if (mismatchType(peek, TokenType::COMMA))
                    break;
                else
                    peek = nextToken();
            }
        }

        // all vars are 1 slot long, so paraNum == paraSize
        if ((int)paras.size() != getFuncParaSize(id))
            return makeCE(ErrorCode::ErrInvalidFunctionCall);

        if (mismatchType(peek, TokenType::RIGHT_PAREN))
            return makeCE(ErrorCode::ErrIncompleteExpression);
        peek = nextToken();

        for (auto p : paras)
            addInstruction(QuadOpr::PUSH, p);

        addInstruction(QuadOpr::CAL, "@" + id);

        auto type = getSymbolType(id);
        if (type != SymbolType::Void) {
            std::string returnValue = "#t" + std::to_string(_nextStackIndex);
            _addSymbol(returnValue, type, false, true, -1, true, false);
            ret = returnValue;
        }

        return std::optional<CompilationError>();
    }

	// <Term>{<additive-operator><Term>}
	std::optional<CompilationError> Analyser::analyseExpression(std::string& ret) {
//        debugOut("Expression");

        std::string term1;
        auto err = analyseTerm(term1);
		if (err.has_value())
			return err;

		while (!mismatchType(peek, TokenType::PLUS_SIGN)
		    || !mismatchType(peek, TokenType::MINUS_SIGN)) {

            auto type = peek.value().GetType();
			peek = nextToken();

            std::string term2;
			err = analyseTerm(term2);
			if (err.has_value())
				return err;

            std::string tmp = getTempName(term1, term2);
            if (type == TokenType::PLUS_SIGN)
                addInstruction(QuadOpr::ADD, term1, term2, tmp);
            else if (type == TokenType::MINUS_SIGN)
                addInstruction(QuadOpr::SUB, term1, term2, tmp);

            term1 = tmp;
		}

		ret = term1;

        return {};
	}

	// <Factor>{<multiplicative-operator><Factor>}
	std::optional<CompilationError> Analyser::analyseTerm(std::string& ret) {
//        debugOut("Term");

        std::string factor1;
        auto err = analyseFactor(factor1);
        if (err.has_value())
            return err;

        while (!mismatchType(peek, TokenType::MULTIPLICATION_SIGN)
            || !mismatchType(peek, TokenType::DIVISION_SIGN)) {

            auto type = peek.value().GetType();
            peek = nextToken();

            std::string factor2;
            err = analyseFactor(factor2);
            if (err.has_value())
                return err;

            std::string tmp = getTempName(factor1, factor2);
            if (type == TokenType::MULTIPLICATION_SIGN)
                addInstruction(QuadOpr::MUL, factor1, factor2, tmp);
            else if (type == TokenType::DIVISION_SIGN)
                addInstruction(QuadOpr::DIV, factor1, factor2, tmp);

            factor1 = tmp;
        }

        ret = factor1;

        return {};
	}

	// <Factor> ::= [<unary-operator>]
    //     '('<expression>')'
    //    |<identifier>
    //    |<integer-literal>
    //    |<char-literal>
    //    |<function-call>
	std::optional<CompilationError> Analyser::analyseFactor(std::string& ret) {
//        debugOut("Factor");
        std::string ret1;

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
		long long l;
		int i;
		auto next = peek;

		switch (peek.value().GetType()) {
            case TokenType::LEFT_PAREN:
                peek = nextToken();
                err = analyseExpression(ret1);
                if (!err.has_value()) {
                    if (mismatchType(peek, TokenType::RIGHT_PAREN))
                        err = makeCE(ErrorCode::ErrIncompleteExpression);
                    else
                        peek = nextToken(); // avoid nextToken after EOF
                }
                break;

            case TokenType::IDENTIFIER:
                next = nextToken();
                id = peek.value().GetValueString();

                if (!mismatchType(next, TokenType::LEFT_PAREN)) {
                    // function-call
                    if (getSymbolType(id) == SymbolType::Void)
                        return makeCE(ErrorCode::ErrVoidVariable);

                    unreadToken();
                    err = analyseFunctionCall(ret1);
                } else {
                    // variable
                    peek = next;

                    if (!isDeclared(id) || isFunction(id))
                        err = makeCE(ErrorCode::ErrNotDeclared);
                    else if (isUninitializedVariable(id))
                        err = makeCE(ErrorCode::ErrNotInitialized);
                    else {
                        ret1 = id;
                    }
                }
                break;

		    case TokenType::UNSIGNED_INTEGER:
		        l = std::any_cast<unsigned long>(peek.value().GetValue());
		        if (l - 1 > INT32_MAX || (l - 1 == INT32_MAX && prefix == 1))
		            err = makeCE(ErrorCode::ErrIntegerOverflow);
                ret1 = "$" + std::to_string(l);

		        peek = nextToken();
                break;

            case TokenType::UNSIGNED_CHAR:
                i = (int)std::any_cast<char>(peek.value().GetValue());
                ret1 = "$" + std::to_string(i);

                peek = nextToken();
                break;

            default:
                err = makeCE(ErrorCode::ErrIncompleteExpression);
		}
		if (err.has_value())
            return err;

		std::string tmp;
		if (prefix == -1) {
		    tmp = getTempName(ret1);
		    addInstruction(QuadOpr::NEG, ret1, "", tmp);
        } else {
		    tmp = ret1;
		}

		ret = tmp;
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

	void Analyser::_addSymbol(const std::string& s, SymbolType type, bool isConst, bool isInit, int16_t funInd,
                              bool isVar, bool needSpace) {
		_symbols.emplace_back(s, _nextStackIndex, type, isConst, isInit, funInd);
        if (isVar)
		    _nextStackIndex++;
        if (needSpace)
            addInstruction(QuadOpr::PUSH, "$0");
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
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        _addSymbol(tk.GetValueString(), type, false, true, -1, true, true);
	}

	void Analyser::addConstant(const Token& tk, SymbolType type) {
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        _addSymbol(tk.GetValueString(), type, true, true, -1, true, true);
    }

	void Analyser::addUninitializedVariable(const Token& tk, SymbolType type) {
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        _addSymbol(tk.GetValueString(), type, false, false, -1, true, true);
    }

	void Analyser::addPara(const Token& tk, SymbolType type, bool isConst) {
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        _addSymbol(tk.GetValueString(), type, isConst, true, -1, true, false);
    }

    int Analyser::addFunction(const Token & tk, SymbolType type) {
	    int16_t funInd = _functions.size();
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        _addSymbol(tk.GetValueString(), type, true, true, funInd, false, false);
	    _functions.emplace_back(type);

        return funInd;
    }

    void Analyser::addFuncPara(int funcId, SymbolType type) {
        _functions[funcId].addPara(type);
    }

    void Analyser::setSymbolTable() {
        _lastSymbolTable.push_back(_symbols.size());
        _lastIndex.push_back(_nextStackIndex);
    }

    void Analyser::resetSymbolTable() {
        int size = _lastSymbolTable.back();
        _lastSymbolTable.pop_back();
        _symbols.erase(_symbols.begin() + size, _symbols.end());

        int diff = _nextStackIndex - _lastIndex.back();
        addInstruction(QuadOpr::POP, "$" + std::to_string(diff));
        _nextStackIndex = _lastIndex.back();
        _lastIndex.pop_back();
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
        return _findSymbol(s) >= _lastSymbolTable.back();
    }

    int32_t Analyser::getStackIndex(const std::string& id) {
        return _symbols[_findSymbol(id)].getStackIndex();
    }

    SymbolType Analyser::getSymbolType(const std::string& id) {
        return _symbols[_findSymbol(id)].getType();
    }

    int Analyser::getFuncParaSize(const std::string &s) {
        if (!isFunction(s))
            return -1;
        int funcId = _symbols[_findSymbol(s)].getFuncIndex();

        return _functions[funcId].getParaSize();
    }

    std::string Analyser::getTempName() {
        std::string tmp = "#t" + std::to_string(_nextStackIndex);
        addVariable(Token(TokenType::IDENTIFIER, tmp, 0, 0, 0, 0), SymbolType::Int);
        return tmp;
    }

    std::string Analyser::getTempName(const std::string &opr1) {
        if (opr1[0] == '#')
            return opr1;
        else
            return getTempName();
    }

    std::string Analyser::getTempName(const std::string &opr1, const std::string &opr2) {
        if (opr1[0] == '#')
            return opr1;
        else if (opr2[0] == '#')
            return opr2;
        else
            return getTempName();
    }

    std::string Analyser::getOpr(std::string str) {
//        return str;
        if (str.empty() || str[0] == '$' || str[0] == '@' )
            return str;
        int index = getStackIndex(str);
        if (_lastIndex.size() > 1 && index < _lastIndex[1]) {
            // global variable
            return "c" + std::to_string(index);
        } else if (_lastIndex.size() == 1){
            return std::to_string(index);
        } else {
            return std::to_string(index - _lastIndex[1]);
        }
    }

    void Analyser::addInstruction(QuadOpr opr, const std::string& x) {
        _instructions.emplace_back(opr, getOpr(x));
    }

    void Analyser::addInstruction(QuadOpr opr, const std::string& x, const std::string& y) {
        _instructions.emplace_back(opr, getOpr(x), getOpr(y));
    }

    void Analyser::addInstruction(QuadOpr opr, const std::string& x, const std::string& y, const std::string& r) {
        _instructions.emplace_back(opr, getOpr(x), getOpr(y), getOpr(r));
    }

    SymbolType Analyser::currentFuncType() {
        return _functions[_functions.size() - 1].getReturnType();
    }


}