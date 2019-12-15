#include "tokenizer/tokenizer.h"

#include <cctype>
#include <sstream>

#define makeCE(pos, ErrCode) \
    std::make_pair(\
        std::optional<Token>(),\
        std::make_optional<CompilationError>(pos, ErrCode))

#define makeTk(type, value)\
    std::make_pair(\
        std::make_optional<Token>(type, value, pos, currentPos()),\
        std::optional<CompilationError>())

namespace miniplc0 {
    TokenType Tokenizer::idType (const std::string& s) {
        TokenType t;
        try {
            t = _reservedWord.at(s);
        } catch (std::out_of_range& e) {
            t = TokenType::IDENTIFIER;
        }
        return t;
    }

    std::optional<unsigned long> strToLong(const std::string& s) {
        unsigned long ret;
        try {
            // auto choose base: dec or hex
            ret = std::stoul(s, 0, 0);
        } catch (std::out_of_range& e) {
            return {};
        }
        return ret;
    }

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
		if (!_initialized)
			tkzInit();
		if (_rdr.bad())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
		if (isEOF())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
		auto p = nextToken();
		if (p.second.has_value())
			return std::make_pair(p.first, p.second);
		return std::make_pair(p.first, std::optional<CompilationError>());
	}

	std::pair<std::vector<Token>, std::optional<CompilationError>> Tokenizer::AllTokens() {
		std::vector<Token> result;
		while (true) {
			auto p = NextToken();
			if (p.second.has_value()) {
				if (p.second.value().GetCode() == ErrorCode::ErrEOF)
					return std::make_pair(result, std::optional<CompilationError>());
				else
					return std::make_pair(std::vector<Token>(), p.second);
			}
			result.emplace_back(p.first.value());
		}
	}

	// 注意：这里的返回值中 Token 和 CompilationError 只能返回一个，不能同时返回。
	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::nextToken() {
		std::stringstream ss;
		std::pair<std::optional<Token>, std::optional<CompilationError>> result;
		std::pair<int64_t, int64_t> pos;	// <line, column>
		DFAState current_state = DFAState::INITIAL_STATE;
		DFAState previous_state;

		while (true) {
			switch (current_state) {

			case INITIAL_STATE: {
				if (peek < 0)
					// 遇到文件尾
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));

				auto invalid = false;

				// 使用了自己封装的判断字符类型的函数，定义于 tokenizer/utils.hpp
				if (miniplc0::isblank(peek) || peek == '\n') {
				    // 0x20 ' ', 0x09 '\t'
				    peek = nextChar();
                    break;
				} else if (peek == '0')
                    current_state = DFAState::ZERO_STATE;
				else if (miniplc0::isdigit(peek))
					current_state = DFAState::UNSIGNED_INTEGER_STATE;
				else if (miniplc0::isalpha(peek))
					current_state = DFAState::IDENTIFIER_STATE;
				else {
					switch (peek) {
					case '=':
						current_state = DFAState::EQUAL_SIGN_STATE;
						break;
					case '<':
						current_state = DFAState::LESS_SIGN_STATE;
						break;
					case '>':
						current_state = DFAState::GREATER_SIGN_STATE;
						break;
					case '!':
						current_state = DFAState::EXCLAM_SIGN_STATE;
						break;
					case '/':
						current_state = DFAState::DIVISION_SIGN_STATE;
						break;
					case '\'':
						current_state = DFAState::CHAR_STATE;
						break;
					case '\"':
						current_state = DFAState::STRING_STATE;
						break;
					case ';':   case ',':
                    case '+':   case '-':   case '*':
					case '(':   case ')':   case '{':   case '}':
						current_state = DFAState::SINGLE_SIGN_STATE;
						break;
					// 不接受的字符导致的不合法的状态
					default:
						invalid = true;
						break;
					}
				}

                // 读到了不合法的字符
                if (invalid)
                    return makeCE(previousPos(), ErrInvalidInput);

                // 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE) {
                    pos = previousPos(); // 记录该字符的的位置为token的开始位置
                    ss.str(std::string());
                    ss << peek;
                    peek = nextChar();
                }

				break;
			}

            case ZERO_STATE: {
                if (peek == 'x' || peek == 'X') {
                    current_state = DFAState::HEX_INTEGER_STATE;
                    ss << 'x';
                    peek = nextChar();
                } else if (miniplc0::isdigit(peek)) {
                    return makeCE(pos, ErrInvalidNumberFormat);
                } else if (miniplc0::isalpha(peek)) {
                    return makeCE(pos, ErrInvalidIdentifier);
                } else {
                    return makeTk(TokenType::UNSIGNED_INTEGER, (unsigned long) 0);
                }
                break;
            }

			case UNSIGNED_INTEGER_STATE: {
				// will not get EOF
				if (miniplc0::isdigit(peek)) {
				// 如果读到的字符是数字，则存储读到的字符
					ss << peek;
					peek = nextChar();
				} else if (miniplc0::isalpha(peek)) {
					return makeCE(pos, ErrInvalidIdentifier);
				} else {
				// 将字符串解析为整数
					auto integer = strToLong(ss.str());
					if (integer.has_value())
                        return makeTk(TokenType::UNSIGNED_INTEGER, integer.value());
					else
                        return makeCE(pos, ErrIntegerOverflow);
				}
				break;
			}

			case HEX_INTEGER_STATE: {
				if (miniplc0::isxdigit(peek)) {
					ss << peek;
					peek = nextChar();
				} else if (miniplc0::isalpha(peek)) {
					return makeCE(pos, ErrInvalidNumberFormat);
				} else {
				// 将字符串解析为整数
					auto integer = strToLong(ss.str());
					if (integer.has_value())
                        return makeTk(TokenType::UNSIGNED_INTEGER, integer.value());
					else
                        return makeCE(pos, ErrIntegerOverflow);
				}
				break;
			}

			case IDENTIFIER_STATE: {
                if (miniplc0::isdigit(peek) || miniplc0::isalpha(peek)) {
                    // 如果读到的是字符或字母，则存储读到的字符
                    ss << peek;
                    peek = nextChar();
                } else {
                    return makeTk(idType(ss.str()), ss.str());
                }
                break;
			}

            case EQUAL_SIGN_STATE: {
                if (peek == '=') {
                    peek = nextChar();
                    return makeTk(TokenType::EQUAL_SIGN, std::string("=="));
                } else {
                    return makeTk(TokenType::ASSIGN_SIGN, '=');
                }
            }

            case LESS_SIGN_STATE: {
                if (peek == '=') {
                    peek = nextChar();
                    return makeTk(TokenType::LESSEQUAL_SIGN, std::string("<="));
                } else {
                    return makeTk(TokenType::LESS_SIGN, '<');
                }
            }

            case GREATER_SIGN_STATE: {
                if (peek == '=') {
                    peek = nextChar();
                    return makeTk(TokenType::GREATEREQUAL_SIGN, std::string(">="));
                } else {
                    return makeTk(TokenType::GREATER_SIGN, '>');
                }
            }

            case EXCLAM_SIGN_STATE: {
                if (peek == '=') {
                    peek = nextChar();
                    return makeTk(TokenType::NOTEQUAL_SIGN, std::string("!="));
                } else {
                    return makeCE(pos, ErrInvalidInput);
                }
            }

			case DIVISION_SIGN_STATE: {
//            *	->	multi comment
//            /	->	single comment
//            other	->	/
                if (peek == '*') {
                    current_state = DFAState::MULTI_COMMENT_STATE;
                    peek = nextChar();
                } else if (peek == '/') {
                    current_state = DFAState::SINGLE_COMMENT_STATE;
                    peek = nextChar();
                } else {
                    return makeTk(TokenType::DIVISION_SIGN, '/');
                }
                break;
            }

            case MULTI_COMMENT_STATE: {
                if (peek == -1) {
                    return makeCE(previousPos(), ErrIncompleteComment);
                } else if (peek == '*') {
                    peek = nextChar();
                    if (peek == '/') {
                        peek = nextChar();
                        current_state = DFAState::INITIAL_STATE;
                    }
                } else {
                    peek = nextChar();
                }

                break;
            }

            case SINGLE_COMMENT_STATE: {
                if (peek == -1) {
                    return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));
                } else if (peek == '\n') {
                    current_state = DFAState::INITIAL_STATE;
                }
                peek = nextChar();

                break;
            }

            case SINGLE_SIGN_STATE: {
                char ch = ss.str()[0];
                switch (ch) {
                    case ';':
                        return makeTk(TokenType::SEMICOLON, ';');
                    case ',':
                        return makeTk(TokenType::COMMA, ',');
                    case '+':
                        return makeTk(TokenType::PLUS_SIGN, '+');
                    case '-':
                        return makeTk(TokenType::MINUS_SIGN, '-');
                    case '*':
                        return makeTk(TokenType::MULTIPLICATION_SIGN, '*');
                    case '(':
                        return makeTk(TokenType::LEFT_PAREN, '(');
                    case ')':
                        return makeTk(TokenType::RIGHT_PAREN, ')');
                    case '{':
                        return makeTk(TokenType::LEFT_BRACE, '{');
                    case '}':
                        return makeTk(TokenType::RIGHT_BRACE, '}');

                    default:
                        DieAndPrint("wrong sign in single_sign_state.");
                        break;
                }
                break;
            }

            case CHAR_STATE: {
                if (peek == '\'') {
                    if (ss.str().length() == 2) {
                        // ', ch
                        peek = nextChar();
                        return makeTk(TokenType::UNSIGNED_CHAR, ss.str()[1]);
                    } else {
                        return makeCE(pos, ErrInvalidCharacter);
                    }
                } else if (peek == '\\') {
                    previous_state = current_state;
                    current_state = DFAState::ESCAPE_STATE;
                    peek = nextChar();
                } else {
                    ss << peek;
                    peek = nextChar();
                }

                break;
            }

            case STRING_STATE: {
                if (peek == '"') {
                    // empty string is legal
                    peek = nextChar();
                    return makeTk(TokenType::STRING, ss.str().substr(1));
                } else if (peek == '\\') {
                    previous_state = current_state;
                    current_state = DFAState::ESCAPE_STATE;
                    peek = nextChar();
                } else {
                    ss << peek;
                    peek = nextChar();
                }

                break;
            }

            case ESCAPE_STATE: {
//            '\\' | "\'" | '\"' | '\n' | '\r' | '\t'
//            '\x'<hexadecimal-digit><hexadecimal-digit>
                char escape;
                switch (peek) {
                    case '\\':
                        escape = '\\';
                        break;
                    case '\'':
                        escape = '\'';
                        break;
                    case '"':
                        escape = '"';
                        break;
                    case 'n':
                        escape = '\n';
                        break;
                    case 'r':
                        escape = '\r';
                        break;
                    case 't':
                        escape = '\t';
                        break;
                    case 'x': {
                        std::string s;
                        peek = nextChar();
                        s += peek;
                        peek = nextChar();
                        s += peek;
                        if (!miniplc0::isxdigit(s[0]) || !miniplc0::isxdigit(s[1]))
                            return makeCE(pos, ErrInvalidCharacter);
                        escape = (char) std::stoi(s, 0, 16);
                        break;
                    }
                    default:
                        return makeCE(pos, ErrInvalidCharacter);
                }
                ss << escape;
                peek = nextChar();
                current_state = previous_state;
                break;
            }

			default:
				DieAndPrint("unhandled state.");
				break;
			}
		}
		// 预料之外的状态，如果执行到了这里，说明程序异常
		return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
	}

	void Tokenizer::tkzInit() {
        readAll();
        addReservedWord();
    }

	void Tokenizer::readAll() {
		if (_initialized)
			return;
		for (std::string tp; std::getline(_rdr, tp);)
			_lines_buffer.emplace_back(std::move(tp + "\n"));
		_initialized = true;
		_ptr = std::make_pair<int64_t, int64_t>(0, 0);
		return;
	}

	void Tokenizer::addReservedWord() {
        _reservedWord["const"] = TokenType::CONST;
        _reservedWord["void"] = TokenType::VOID;
        _reservedWord["int"] = TokenType::INT;
        _reservedWord["char"] = TokenType::CHAR;
        _reservedWord["double"] = TokenType::DOUBLE;
        _reservedWord["struct"] = TokenType::STRUCT;
        _reservedWord["if"] = TokenType::IF;
        _reservedWord["else"] = TokenType::ELSE;
        _reservedWord["switch"] = TokenType::SWITCH;
        _reservedWord["case"] = TokenType::CASE;
        _reservedWord["default"] = TokenType::DEFAULT;
        _reservedWord["while"] = TokenType::WHILE;
        _reservedWord["for"] = TokenType::FOR;
        _reservedWord["do"] = TokenType::DO;
        _reservedWord["return"] = TokenType::RETURN;
        _reservedWord["break"] = TokenType::BREAK;
        _reservedWord["continue"] = TokenType::CONTINUE;
        _reservedWord["print"] = TokenType::PRINT;
        _reservedWord["scan"] = TokenType::SCAN;
    }
	// Note: We allow this function to return a postion which is out of bound according to the design like std::vector::end().
	std::pair<uint64_t, uint64_t> Tokenizer::nextPos() {
		if (_ptr.first >= _lines_buffer.size())
			DieAndPrint("advance after EOF");
		if (_ptr.second == _lines_buffer[_ptr.first].size() - 1)
			return std::make_pair(_ptr.first + 1, 0);
		else
			return std::make_pair(_ptr.first, _ptr.second + 1);
	}

	std::pair<uint64_t, uint64_t> Tokenizer::currentPos() {
		return _ptr;
	}

	std::pair<uint64_t, uint64_t> Tokenizer::previousPos() {
		if (_ptr.first == 0 && _ptr.second == 0)
			DieAndPrint("previous position from beginning");
		if (_ptr.second == 0)
			return std::make_pair(_ptr.first - 1, _lines_buffer[_ptr.first - 1].size() - 1);
		else
			return std::make_pair(_ptr.first, _ptr.second - 1);
	}

	char Tokenizer::nextChar() {
		if (isEOF())
            return -1;
		char result = _lines_buffer[_ptr.first][_ptr.second];
		_ptr = nextPos();
		return result;
	}

	bool Tokenizer::isEOF() {
		return _ptr.first >= _lines_buffer.size();
	}

}