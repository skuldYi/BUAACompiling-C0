#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
	template<>
	struct formatter<c0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case c0::ErrNoError:
				name = "No error.";
				break;
			case c0::ErrStreamError:
				name = "Stream error.";
				break;
			case c0::ErrEOF:
				name = "EOF";
				break;
			case c0::ErrInvalidInput:
				name = "The input is invalid.";
				break;
			case c0::ErrInvalidIdentifier:
				name = "Identifier is invalid";
				break;
			case c0::ErrInvalidNumberFormat:
				name = "Number format is invalid";
				break;
			case c0::ErrInvalidCharacter:
				name = "Character format is invalid";
				break;
			case c0::ErrIncompleteComment:
				name = "The comment is incomplete.";
				break;
			case c0::ErrIntegerOverflow:
				name = "The integer is too big(int64_t).";
				break;
			case c0::ErrNeedIdentifier:
				name = "Need an identifier here.";
				break;
			case c0::ErrConstantNeedValue:
				name = "The constant need a value to initialize.";
				break;
			case c0::ErrNoSemicolon:
				name = "Zai? Wei shen me bu xie fen hao.";
				break;
			case c0::ErrIncompleteExpression:
				name = "The expression is incomplete.";
				break;
			case c0::ErrNotDeclared:
				name = "The variable or constant must be declared before being used.";
				break;
			case c0::ErrAssignToConstant:
				name = "Trying to assign value to a constant.";
				break;
			case c0::ErrDuplicateDeclaration:
				name = "The variable or constant has been declared.";
				break;
			case c0::ErrNotInitialized:
				name = "The variable has not been initialized.";
				break;
			case c0::ErrInvalidAssignment:
				name = "The assignment statement is invalid.";
				break;
			case c0::ErrInvalidPrint:
				name = "The output statement is invalid.";
				break;
			case c0::ErrVoidVariable:
				name = "Variables or parameter can not be void.";
				break;
			case c0::ErrNeedTypeSpecifier:
				name = "Need a type specifier here.";
				break;
			case c0::ErrNeedStatement:
				name = "Need a statement here.";
				break;
			case c0::ErrMissingBrace:
				name = "Need an brace here.";
				break;
			case c0::ErrSyntaxError:
				name = "Encounter syntax mistake.";
				break;
			case c0::ErrInvalidFunctionCall:
				name = "Wrong argument number or type to function call.";
				break;
			case c0::ErrFunctionNotDefined:
				name = "Function must be defined before being called.";
				break;
			case c0::ErrInvalidReturnValue:
				name = "Return value has wrong type.";
				break;
			case c0::ErrNeedReturnValue:
				name = "Non-void function should return a value.";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};

	template<>
	struct formatter<c0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<c0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line: {} Column: {} Type: {} Value: {}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<c0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case c0::NULL_TOKEN:
				name = "NullToken";
				break;
			case c0::UNSIGNED_INTEGER:
				name = "UnsignedInteger";
				break;
			case c0::UNSIGNED_CHAR:
				name = "UnsignedChar";
				break;
			case c0::STRING:
				name = "String";
				break;
			case c0::IDENTIFIER:
				name = "Identifier";
				break;
            case c0::CONST:
                name = "Const";
                break;
            case c0::VOID:
                name = "Void";
                break;
            case c0::INT:
                name = "Int";
                break;
            case c0::CHAR:
                name = "Char";
                break;
            case c0::DOUBLE:
                name = "Double";
                break;
            case c0::STRUCT:
                name = "Struct";
                break;
            case c0::IF:
                name = "If";
                break;
            case c0::ELSE:
                name = "Else";
                break;
            case c0::SWITCH:
                name = "Switch";
                break;
            case c0::CASE:
                name = "Case";
                break;
            case c0::DEFAULT:
                name = "Default";
                break;
            case c0::WHILE:
                name = "While";
                break;
            case c0::FOR:
                name = "For";
                break;
            case c0::DO:
                name = "Do";
                break;
            case c0::RETURN:
                name = "Return";
                break;
            case c0::BREAK:
                name = "Break";
                break;
            case c0::CONTINUE:
                name = "Continue";
                break;
            case c0::PRINT:
                name = "Print";
                break;
            case c0::SCAN:
                name = "Scan";
                break;
            case c0::PLUS_SIGN:
                name = "PlusSign";
                break;
            case c0::MINUS_SIGN:
                name = "MinusSign";
                break;
            case c0::MULTIPLICATION_SIGN:
                name = "MultiplicationSign";
                break;
            case c0::DIVISION_SIGN:
                name = "DivisionSign";
                break;
            case c0::ASSIGN_SIGN:
                name = "AssignSign";
                break;
            case c0::LESS_SIGN:
                name = "LessSign";
                break;
            case c0::LESSEQUAL_SIGN:
                name = "LessEqualSign";
                break;
            case c0::GREATER_SIGN:
                name = "GreaterSign";
                break;
            case c0::GREATEREQUAL_SIGN:
                name = "GreaterEqualSign";
                break;
            case c0::NOTEQUAL_SIGN:
                name = "NotEqualSign";
                break;
            case c0::EQUAL_SIGN:
                name = "EqualSign";
                break;
			case c0::SEMICOLON:
				name = "Semicolon";
				break;
			case c0::COMMA:
				name = "Comma";
				break;
			case c0::LEFT_PAREN:
				name = "LeftParen";
				break;
			case c0::RIGHT_PAREN:
				name = "RightParen";
				break;
			case c0::LEFT_BRACE:
				name = "LeftBrace";
				break;
			case c0::RIGHT_BRACE:
				name = "RightBrace";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
}

namespace fmt {
	template<>
	struct formatter<c0::QuadOpr> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::QuadOpr &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case c0::ASN:
				name = "ASN";
				break;
			case c0::NEG:
				name = "NEG";
				break;
			case c0::ADD:
				name = "ADD";
				break;
			case c0::SUB:
				name = "SUB";
				break;
			case c0::MUL:
				name = "MUL";
				break;
			case c0::DIV:
				name = "DIV";
				break;
			case c0::LAB:
				name = "LAB";
				break;
			case c0::FUNC:
				name = "FUNC";
				break;
			case c0::PUSH:
				name = "PUSH";
				break;
			case c0::POP:
				name = "POP";
				break;
			case c0::CAL:
				name = "CALL";
				break;
			case c0::RET:
				name = "RET";
				break;
            case c0::EQU:
                name = "EQU";
                break;
            case c0::NE:
                name = "NE";
                break;
            case c0::LT:
                name = "LT";
                break;
            case c0::LE:
                name = "LE";
                break;
            case c0::GT:
                name = "GT";
                break;
            case c0::GE:
                name = "GE";
                break;
            case c0::GOTO:
                name = "GOTO";
                break;
            case c0::BNZ:
                name = "BNZ";
                break;
            case c0::BZ:
                name = "BZ";
                break;
            case c0::PRT:
                name = "PRT";
                break;
            case c0::SCN:
                name = "SCAN";
                break;
			}
			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<c0::Quadruple> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::Quadruple &p, FormatContext &ctx) {
			std::string name;
            return format_to(ctx.out(), "{}\t{}\t{}\t{}", p.getOperation(), p.getX(), p.getY(), p.getR());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<c0::opCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::opCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
                case c0::nop:
                    name = "nop";
                    break;
                case c0::biPush:
                    name = "bipush";
                    break;
                case c0::iPush:
                    name = "ipush";
                    break;
                case c0::pop1:
                    name = "pop1";
                    break;
                case c0::popN:
                    name = "popn";
                    break;
                case c0::loadC:
                    name = "loadc";
                    break;
                case c0::loadA:
                    name = "loada";
                    break;
                case c0::iLoad:
                    name = "iload";
                    break;
                case c0::iStore:
                    name = "istore";
                    break;
                case c0::iAdd:
                    name = "iadd";
                    break;
                case c0::iSub:
                    name = "isub";
                    break;
                case c0::iMul:
                    name = "imul";
                    break;
                case c0::iDiv:
                    name = "idiv";
                    break;
                case c0::iNeg:
                    name = "ineg";
                    break;
                case c0::iCmp:
                    name = "icmp";
                    break;
                case c0::i2c:
                    name = "i2c";
                    break;
                case c0::jmp:
                    name = "jmp";
                    break;
                case c0::je:
                    name = "je";
                    break;
                case c0::jne:
                    name = "jne";
                    break;
                case c0::jl:
                    name = "jl";
                    break;
                case c0::jge:
                    name = "jge";
                    break;
                case c0::jg:
                    name = "jg";
                    break;
                case c0::jle:
                    name = "jle";
                    break;
                case c0::call:
                    name = "call";
                    break;
                case c0::ret:
                    name = "ret";
                    break;
                case c0::iRet:
                    name = "iret";
                    break;
                case c0::iPrint:
                    name = "iprint";
                    break;
                case c0::cPrint:
                    name = "cprint";
                    break;
                case c0::sPrint:
                    name = "sprint";
                    break;
                case c0::printL:
                    name = "printl";
                    break;
                case c0::iScan:
                    name = "iscan";
                    break;
                case c0::cScan:
                    name = "cscan";
                    break;
			}
			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<c0::Instruction> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::Instruction &ins, FormatContext &ctx) {
			switch (ins.getOpr()) {
                case c0::loadA:
                    return format_to(ctx.out(), "{} {}, {}", ins.getOpr(), ins.getX(), ins.getY());

                case c0::biPush:
                case c0::iPush:
                case c0::popN:
                case c0::loadC:
                case c0::jmp:
                case c0::je:
                case c0::jne:
                case c0::jl:
                case c0::jge:
                case c0::jg:
                case c0::jle:
                case c0::call:
                    return format_to(ctx.out(), "{} {}", ins.getOpr(), ins.getX());

                default:
                    return format_to(ctx.out(), "{}", ins.getOpr());
            }
		}
	};
}

namespace fmt {
	template<>
	struct formatter<c0::funcInfo> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const c0::funcInfo &f, FormatContext &ctx) {
            return format_to(ctx.out(), "{}\t{}\t{}", f.name_index, f.params_size, f.level);
		}
	};
}