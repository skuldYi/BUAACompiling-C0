#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
	template<>
	struct formatter<miniplc0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::ErrNoError:
				name = "No error.";
				break;
			case miniplc0::ErrStreamError:
				name = "Stream error.";
				break;
			case miniplc0::ErrEOF:
				name = "EOF";
				break;
			case miniplc0::ErrInvalidInput:
				name = "The input is invalid.";
				break;
			case miniplc0::ErrInvalidIdentifier:
				name = "Identifier is invalid";
				break;
			case miniplc0::ErrInvalidNumberFormat:
				name = "Number format is invalid";
				break;
			case miniplc0::ErrInvalidCharacter:
				name = "Character format is invalid";
				break;
			case miniplc0::ErrIncompleteComment:
				name = "The comment is incomplete.";
				break;
			case miniplc0::ErrIntegerOverflow:
				name = "The integer is too big(int64_t).";
				break;
			case miniplc0::ErrNeedIdentifier:
				name = "Need an identifier here.";
				break;
			case miniplc0::ErrConstantNeedValue:
				name = "The constant need a value to initialize.";
				break;
			case miniplc0::ErrNoSemicolon:
				name = "Zai? Wei shen me bu xie fen hao.";
				break;
			case miniplc0::ErrInvalidVariableDeclaration:
				name = "The declaration is invalid.";
				break;
			case miniplc0::ErrIncompleteExpression:
				name = "The expression is incomplete.";
				break;
			case miniplc0::ErrNotDeclared:
				name = "The variable or constant must be declared before being used.";
				break;
			case miniplc0::ErrAssignToConstant:
				name = "Trying to assign value to a constant.";
				break;
			case miniplc0::ErrDuplicateDeclaration:
				name = "The variable or constant has been declared.";
				break;
			case miniplc0::ErrNotInitialized:
				name = "The variable has not been initialized.";
				break;
			case miniplc0::ErrInvalidAssignment:
				name = "The assignment statement is invalid.";
				break;
			case miniplc0::ErrInvalidPrint:
				name = "The output statement is invalid.";
				break;
			case miniplc0::ErrVoidVariable:
				name = "Variables can not be void.";
				break;
			case miniplc0::ErrNeedTypeSpecifier:
				name = "Need an type specifier here.";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};

	template<>
	struct formatter<miniplc0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<miniplc0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line: {} Column: {} Type: {} Value: {}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<miniplc0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::NULL_TOKEN:
				name = "NullToken";
				break;
			case miniplc0::UNSIGNED_INTEGER:
				name = "UnsignedInteger";
				break;
			case miniplc0::UNSIGNED_CHAR:
				name = "UnsignedChar";
				break;
			case miniplc0::STRING:
				name = "String";
				break;
			case miniplc0::IDENTIFIER:
				name = "Identifier";
				break;
            case miniplc0::CONST:
                name = "Const";
                break;
            case miniplc0::VOID:
                name = "Void";
                break;
            case miniplc0::INT:
                name = "Int";
                break;
            case miniplc0::CHAR:
                name = "Char";
                break;
            case miniplc0::DOUBLE:
                name = "Double";
                break;
            case miniplc0::STRUCT:
                name = "Struct";
                break;
            case miniplc0::IF:
                name = "If";
                break;
            case miniplc0::ELSE:
                name = "Else";
                break;
            case miniplc0::SWITCH:
                name = "Switch";
                break;
            case miniplc0::CASE:
                name = "Case";
                break;
            case miniplc0::DEFAULT:
                name = "Default";
                break;
            case miniplc0::WHILE:
                name = "While";
                break;
            case miniplc0::FOR:
                name = "For";
                break;
            case miniplc0::DO:
                name = "Do";
                break;
            case miniplc0::RETURN:
                name = "Return";
                break;
            case miniplc0::BREAK:
                name = "Break";
                break;
            case miniplc0::CONTINUE:
                name = "Continue";
                break;
            case miniplc0::PRINT:
                name = "Print";
                break;
            case miniplc0::SCAN:
                name = "Scan";
                break;
            case miniplc0::PLUS_SIGN:
                name = "PlusSign";
                break;
            case miniplc0::MINUS_SIGN:
                name = "MinusSign";
                break;
            case miniplc0::MULTIPLICATION_SIGN:
                name = "MultiplicationSign";
                break;
            case miniplc0::DIVISION_SIGN:
                name = "DivisionSign";
                break;
            case miniplc0::ASSIGN_SIGN:
                name = "AssignSign";
                break;
            case miniplc0::LESS_SIGN:
                name = "LessSign";
                break;
            case miniplc0::LESSEQUAL_SIGN:
                name = "LessEqualSign";
                break;
            case miniplc0::GREATER_SIGN:
                name = "GreaterSign";
                break;
            case miniplc0::GREATEREQUAL_SIGN:
                name = "GreaterEqualSign";
                break;
            case miniplc0::NOTEQUAL_SIGN:
                name = "NotEqualSign";
                break;
            case miniplc0::EQUAL_SIGN:
                name = "EqualSign";
                break;
			case miniplc0::SEMICOLON:
				name = "Semicolon";
				break;
			case miniplc0::COMMA:
				name = "Comma";
				break;
			case miniplc0::LEFT_PAREN:
				name = "LeftParen";
				break;
			case miniplc0::RIGHT_PAREN:
				name = "RightParen";
				break;
			case miniplc0::LEFT_BRACE:
				name = "LeftBrace";
				break;
			case miniplc0::RIGHT_BRACE:
				name = "RightBrace";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
}

namespace fmt {
	template<>
	struct formatter<miniplc0::Operation> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Operation &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::ILL:
				name = "ILL";
				break;
			case miniplc0::ADD:
				name = "ADD";
				break;
			case miniplc0::SUB:
				name = "SUB";
				break;
			case miniplc0::MUL:
				name = "MUL";
				break;
			case miniplc0::DIV:
				name = "DIV";
				break;
			case miniplc0::WRT:
				name = "WRT";
				break;
			case miniplc0::LIT:
				name = "LIT";
				break;
			case miniplc0::LOD:
				name = "LOD";
				break;
			case miniplc0::STO:
				name = "STO";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<miniplc0::Instruction> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Instruction &p, FormatContext &ctx) {
			std::string name;
			switch (p.GetOperation())
			{
			case miniplc0::ILL:
			case miniplc0::ADD:
			case miniplc0::SUB:
			case miniplc0::MUL:
			case miniplc0::DIV:
			case miniplc0::WRT:
				return format_to(ctx.out(), "{}", p.GetOperation());
			case miniplc0::LIT:
			case miniplc0::LOD:
			case miniplc0::STO:
				return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
			}
			return format_to(ctx.out(), "ILL");
		}
	};
}