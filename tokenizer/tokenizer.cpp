#include "tokenizer/tokenizer.h"

#include <cctype>
#include <sstream>

namespace miniplc0 {
    TokenType idType (const std::string& s) {
        if (s == "begin")
            return TokenType::BEGIN;
        if (s == "end")
            return TokenType::END;
        if (s == "var")
            return TokenType::VAR;
        if (s == "const")
            return TokenType::CONST;
        if (s == "print")
            return TokenType::PRINT;

        return TokenType::IDENTIFIER;
    }

    std::optional<int32_t> strToInt(const std::string& s) {
        int32_t i;
        try {
            i = std::stoi(s);
        } catch (std::out_of_range& e) {
            return {};
        }
        return i;
    }

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
		if (!_initialized)
			readAll();
		if (_rdr.bad())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
		if (isEOF())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
		auto p = nextToken();
		if (p.second.has_value())
			return std::make_pair(p.first, p.second);
		auto err = checkToken(p.first.value());
		if (err.has_value())
			return std::make_pair(p.first, err.value());
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
		
		while (true) {
			// 读一个字符，请注意auto推导得出的类型是std::optional<char>
			auto current_char = nextChar();

			switch (current_state) {

			case INITIAL_STATE: {
				// if (isEOF()) return {};
				if (!current_char.has_value())
					// 返回一个空的token，和编译错误ErrEOF：遇到了文件尾
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));

				auto ch = current_char.value();
				auto invalid = false;

				// 使用了自己封装的判断字符类型的函数，定义于 tokenizer/utils.hpp
				// see https://en.cppreference.com/w/cpp/string/byte/isblank
				if (miniplc0::isspace(ch))
					current_state = DFAState::INITIAL_STATE; // stay init
				else if (!miniplc0::isprint(ch)) // control codes and backspace
					invalid = true;
				else if (miniplc0::isdigit(ch))
					current_state = DFAState::UNSIGNED_INTEGER_STATE;
				else if (miniplc0::isalpha(ch))
					current_state = DFAState::IDENTIFIER_STATE;
				else {
					switch (ch) {
					case '=':
						current_state = DFAState::EQUAL_SIGN_STATE;
						break;
					case '-':
						current_state = DFAState::MINUS_SIGN_STATE;
						break;
					case '+':
						current_state = DFAState::PLUS_SIGN_STATE;
						break;
					case '*':
						current_state = DFAState::MULTIPLICATION_SIGN_STATE;
						break;
					case '/':
						current_state = DFAState::DIVISION_SIGN_STATE;
						break;
					case ';':
						current_state = DFAState::SEMICOLON_STATE;
						break;
					case '(':
						current_state = DFAState::LEFTBRACKET_STATE;
						break;
					case ')':
						current_state = DFAState::RIGHTBRACKET_STATE;
						break;
					// 不接受的字符导致的不合法的状态
					default:
						invalid = true;
						break;
					}
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE)
					pos = previousPos(); // 记录该字符的的位置为token的开始位置
				// 读到了不合法的字符
				if (invalid) {
					unreadLast();
					// 返回编译错误：非法的输入
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}

                // ignore white spaces
				if (current_state != DFAState::INITIAL_STATE) {
                    ss.str(std::string());
                    ss << ch;
                }
				break;
			}

			case UNSIGNED_INTEGER_STATE: {
				// will not get EOF
				auto ch = current_char.value();
				if (miniplc0::isdigit(ch)) {
				// 如果读到的字符是数字，则存储读到的字符
					ss << ch;
				} else if (miniplc0::isalpha(ch)) {
				// 如果读到的是字母，返回编译错误 invalid identifier
					return std::make_pair(
					        std::optional<Token>(),
					        std::make_optional<CompilationError>(pos, ErrInvalidIdentifier));
				} else {
				// 如果读到的字符不是上述情况之一，则回退读到的字符，将字符串解析为整数
					unreadLast();
					auto integer = strToInt(ss.str());
					if (integer.has_value())
                        return std::make_pair(
                                std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, integer.value(), pos, currentPos()),
                                std::optional<CompilationError>());
					else
                        return std::make_pair(
                                std::optional<Token>(),
                                std::make_optional<CompilationError>(pos, ErrIntegerOverflow));
				}
				break;
			}
			case IDENTIFIER_STATE: {
			    // will not get EOF
                auto ch = current_char.value();
                if (miniplc0::isdigit(ch) || miniplc0::isalpha(ch)) {
                    // 如果读到的是字符或字母，则存储读到的字符
                    ss << ch;
                } else {
                    // 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串
                    //     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
                    unreadLast();
                    return std::make_pair(
                            std::make_optional<Token>(idType(ss.str()), ss.str(), pos, currentPos()),
                            std::optional<CompilationError>());
                }
                break;
			}

			case PLUS_SIGN_STATE: {
				// 请思考这里为什么要回退，在其他地方会不会需要
				unreadLast(); // Yes, we unread last char even if it's an EOF.
				return std::make_pair(std::make_optional<Token>(TokenType::PLUS_SIGN, '+', pos, currentPos()), std::optional<CompilationError>());
			}

			case MINUS_SIGN_STATE: {
				// 请填空：回退，并返回减号token
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::MINUS_SIGN, '-', pos, currentPos()), std::optional<CompilationError>());
            }

			case MULTIPLICATION_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::MULTIPLICATION_SIGN, '*', pos, currentPos()), std::optional<CompilationError>());
            }

			case DIVISION_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN, '/', pos, currentPos()), std::optional<CompilationError>());
            }

			case EQUAL_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN, '=', pos, currentPos()), std::optional<CompilationError>());
            }

			case SEMICOLON_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::SEMICOLON, ';', pos, currentPos()), std::optional<CompilationError>());
            }

			case LEFTBRACKET_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACKET, '(', pos, currentPos()), std::optional<CompilationError>());
            }

			case RIGHTBRACKET_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACKET, ')', pos, currentPos()), std::optional<CompilationError>());
            }

			default:
				DieAndPrint("unhandled state.");
				break;
			}
		}
		// 预料之外的状态，如果执行到了这里，说明程序异常
		return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
	}

	std::optional<CompilationError> Tokenizer::checkToken(const Token& t) {
		switch (t.GetType()) {
			case IDENTIFIER: {
				auto val = t.GetValueString();
				if (miniplc0::isdigit(val[0]))
					return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIdentifier);
				break;
			}
		default:
			break;
		}
		return {};
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

	std::optional<char> Tokenizer::nextChar() {
		if (isEOF())
			return {}; // EOF
		auto result = _lines_buffer[_ptr.first][_ptr.second];
		_ptr = nextPos();
		return result;
	}

	bool Tokenizer::isEOF() {
		return _ptr.first >= _lines_buffer.size();
	}

	// Note: Is it evil to unread a buffer?
	void Tokenizer::unreadLast() {
		_ptr = previousPos();
	}


}