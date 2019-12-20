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

//			switch (p.getOperation())
//			{
//			case c0::LAB:
//			case c0::PUSH:
//			case c0::CAL:
//			case c0::RET:
//			case c0::GOTO:
//			case c0::BNZ:
//			case c0::BZ:
//				return format_to(ctx.out(), "{} {}", p.getOperation(), p.getX())
//
//			case c0::ADD:
//			case c0::SUB:
//			case c0::MUL:
//			case c0::DIV:
//			case c0::WRT:
//				return format_to(ctx.out(), "{}", p.GetOperation());
//			case c0::LIT:
//			case c0::LOD:
//			case c0::STO:
//				return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
//			}
//			return format_to(ctx.out(), "ILL");
		}
	};
}