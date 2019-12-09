#include "analyser.h"

#include <climits>

namespace miniplc0 {
	bool mismatchType(std::optional<Token> token, TokenType type) {
		return (!token.has_value() || token.value().GetType() != type);
	}

	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <程序> ::= 'begin'<主过程>'end'
	std::optional<CompilationError> Analyser::analyseProgram() {
		// 示例函数，示例如何调用子程序

		// 'begin'
		auto bg = nextToken();
		// if (!bg.has_value() || bg.value().GetType() != TokenType::BEGIN)
//		if (mismatchType(bg, TokenType::BEGIN))
//			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoBegin);

		// <主过程>
		auto err = analyseMain();
		if (err.has_value())
			return err;

		// 'end'
		auto ed = nextToken();
		// if (!ed.has_value() || ed.value().GetType() != TokenType::END)
//		if (mismatchType(ed, TokenType::END))
//			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
		return {};
	}

	// <主过程> ::= <常量声明><变量声明><语句序列>
	std::optional<CompilationError> Analyser::analyseMain() {
		// <常量声明>
        auto err = analyseConstantDeclaration();
        if (err.has_value())
            return err;

		// <变量声明>
        err = analyseVariableDeclaration();
        if (err.has_value())
            return err;

		// <语句序列>
        err = analyseStatementSequence();
        if (err.has_value())
            return err;
	}

	// <常量声明> ::= {<常量声明语句>}
	// <常量声明语句> ::= 'const'<标识符>'='<常表达式>';'
	std::optional<CompilationError> Analyser::analyseConstantDeclaration() {
		// 常量声明语句可能有 0 或无数个
		while (true) {
			// 预读一个 token，不然不知道是否应该用 <常量声明> 推导
			auto next = nextToken();
			if (!next.has_value())
				return {};
			// 如果是 const 那么说明应该推导 <常量声明> 否则直接返回
			if (next.value().GetType() != TokenType::CONST) {
				unreadToken();
				return {};
			}

			// <常量声明语句>
			next = nextToken();
			if (mismatchType(next, TokenType::IDENTIFIER))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			if (isDeclared(next.value().GetValueString()))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
			addConstant(next.value());

			// '='
			next = nextToken();
			if (mismatchType(next, TokenType::ASSIGN_SIGN))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);

			// <常表达式>
			int32_t val;
			auto err = analyseConstantExpression(val);
			if (err.has_value())
				return err;

			// ';'
			next = nextToken();
			if (mismatchType(next, TokenType::SEMICOLON))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			// 生成一次 LIT 指令加载常量
			_instructions.emplace_back(Operation::LIT, val);
		}
	}

	// <变量声明> ::= {<变量声明语句>}
	// <变量声明语句> ::= 'var'<标识符>['='<表达式>]';'
	// 需要补全
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		// 变量声明语句可能有 0 个或者多个
		while (true) {
            // 预读
            auto next = nextToken();
            if (!next.has_value())
                return {};

            // 如果是 var 那么说明应该推导 <常量声明> 否则直接返回
//            if (next.value().GetType() != TokenType::VAR) {
//                unreadToken();
//                return {};
//            }

            // <标识符>
            next = nextToken();
            if (mismatchType(next, TokenType::IDENTIFIER))
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
            if (isDeclared(next.value().GetValueString()))
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

            // 变量可能没有初始化，仍然需要一次预读
            Token varId = next.value();
            next = nextToken();
            if (!next.has_value())
                return {};

            // 如果不是 = 那么说明变量未初始化
            if (next.value().GetType() != TokenType::ASSIGN_SIGN) {
                unreadToken();
                addUninitializedVariable(varId);

                // 如果变量没有初始化也需要往栈里压入一个值
                // 保证符号表中的偏移与栈中值位置对应
                _instructions.emplace_back(Operation::LIT, 0);
            } else {
                addVariable(varId);
                auto err = analyseExpression();
                if (err.has_value())
                    return err;
                // 初始化变量的值在解析表达式后压栈
            }

            // ';'
            next = nextToken();
            if (mismatchType(next, TokenType::SEMICOLON))
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        }
	}

	// <语句序列> ::= {<语句>}
	// <语句> :: = <赋值语句> | <输出语句> | <空语句>
	// <赋值语句> :: = <标识符>'='<表达式>';'
	// <输出语句> :: = 'print' '(' <表达式> ')' ';'
	// <空语句> :: = ';'
	// 需要补全
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
                    err = analyseOutputStatement();
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

	// <常表达式> ::= [<符号>]<无符号整数>
	std::optional<CompilationError> Analyser::analyseConstantExpression(int32_t& out) {
		// out 是常表达式的结果
		// 注意以下均为常表达式：+1 -1 1
		auto next = nextToken();
		if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

        int32_t sign = 1;
        if (next.value().GetType() == TokenType::MINUS_SIGN) {
            sign = -1;
            next = nextToken();
        } else if (next.value().GetType() == TokenType::PLUS_SIGN) {
            next = nextToken();
        }

        if (mismatchType(next, TokenType::UNSIGNED_INTEGER))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

        // 无符号整数字面量的值必须在[0, 2^{31}-1]以内，不会溢出
        out = std::any_cast<int32_t>(next.value().GetValue()) * sign;
		return {};
	}

	// <表达式> ::= <项>{<加法型运算符><项>}
	std::optional<CompilationError> Analyser::analyseExpression() {
		// <项>
		auto err = analyseItem();
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
			err = analyseItem();
			if (err.has_value())
				return err;

			// 根据结果生成指令
			if (type == TokenType::PLUS_SIGN)
				_instructions.emplace_back(Operation::ADD, 0);
			else if (type == TokenType::MINUS_SIGN)
				_instructions.emplace_back(Operation::SUB, 0);
		}
	}

	// <赋值语句> ::= <标识符>'='<表达式>';'
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
        _instructions.emplace_back(Operation::STO, getIndex(id));

		return {};
	}

	// <输出语句> ::= 'print' '(' <表达式> ')' ';'
	std::optional<CompilationError> Analyser::analyseOutputStatement() {
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

	// <项> :: = <因子>{ <乘法型运算符><因子> }
	std::optional<CompilationError> Analyser::analyseItem() {
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

	// <因子> ::= [<符号>]( <标识符> | <无符号整数> | '('<表达式>')' )
	// 需要补全
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

                index = getIndex(next.value().GetValueString());
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

	void Analyser::_add(const Token& tk, std::map<std::string, int32_t>& mp) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		mp[tk.GetValueString()] = _nextTokenIndex;
		_nextTokenIndex++;
	}

	void Analyser::addVariable(const Token& tk) {
		_add(tk, _vars);
	}

	void Analyser::addConstant(const Token& tk) {
		_add(tk, _consts);
	}

	void Analyser::addUninitializedVariable(const Token& tk) {
		_add(tk, _uninitialized_vars);
	}

	void Analyser::initVariable(const std::string & id) {
	    if (!isUninitializedVariable(id))
            DieAndPrint("only uninitialized variable can be initiated.");
        auto offset = _uninitialized_vars[id];
	    _uninitialized_vars.erase(id);
	    _vars[id] = offset;
    }

	int32_t Analyser::getIndex(const std::string& s) {
		if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
			return _uninitialized_vars[s];
		else if (_vars.find(s) != _vars.end())
			return _vars[s];
		else
			return _consts[s];
	}

	bool Analyser::isDeclared(const std::string& s) {
		return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s);
	}

	bool Analyser::isUninitializedVariable(const std::string& s) {
		return _uninitialized_vars.find(s) != _uninitialized_vars.end();
	}
	bool Analyser::isInitializedVariable(const std::string&s) {
		return _vars.find(s) != _vars.end();
	}

	bool Analyser::isConstant(const std::string&s) {
		return _consts.find(s) != _consts.end();
	}
}