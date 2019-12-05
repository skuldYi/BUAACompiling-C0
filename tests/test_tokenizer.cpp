#include "catch2/catch.hpp"
#include "tokenizer/tokenizer.h"
#include "fmt/core.h"

#include <sstream>
#include <vector>

// 下面是示例如何书写测试用例
TEST_CASE("Test hello world.") {
	/*
	std::string input = 
		"begin\n"
		"	var a = 1;\n"
		"	const b = 1\n"
		"	print(a+b);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);
	miniplc0::Tokenizer tkz(ss);
	std::vector<miniplc0::Token> output = {};
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}
	REQUIRE( (result.first == output) );
	*/
}

TEST_CASE("Test keywords") {
std::string input = "begin end var const print";
std::stringstream ss(input);
miniplc0::Tokenizer lexer(ss);
auto result = lexer.AllTokens();

std::vector<miniplc0::Token> expected = {
        miniplc0::Token(miniplc0::TokenType::BEGIN, std::string("begin"), {0, 0},
                        {0, 5}),
        miniplc0::Token(miniplc0::TokenType::END, std::string("end"), {0, 6},
                        {0, 9}),
        miniplc0::Token(miniplc0::TokenType::VAR, std::string("var"), {0, 10},
                        {0, 13}),
        miniplc0::Token(miniplc0::TokenType::CONST, std::string("const"), {0, 14},
                        {0, 19}),
        miniplc0::Token(miniplc0::TokenType::PRINT, std::string("print"), {0, 20},
                        {0, 25}),
};

if (result.second.has_value()) {
FAIL("Error introduced in lexing keywords");
} else {
// for (auto& x : result.first) {
//   std::cout << x.GetType() << " " << x.GetValueString() << " "
//             << x.GetStartPos().first << " " << x.GetStartPos().second <<
//             " "
//             << x.GetEndPos().first << " " << x.GetEndPos().second
//             << std::endl;
// }
REQUIRE(result.first == expected);
}
}

TEST_CASE("Test regular numbers") {
std::string input = "1 -1 999";
std::stringstream ss(input);
miniplc0::Tokenizer lexer(ss);
auto result = lexer.AllTokens();

std::vector<miniplc0::Token> expected = {
        miniplc0::Token(miniplc0::TokenType::UNSIGNED_INTEGER, (int32_t)1, {0, 0},
                        {0, 1}),
        miniplc0::Token(miniplc0::TokenType::MINUS_SIGN, '-', {0, 2}, {0, 3}),
        // minus sign is parsed as separate token
        miniplc0::Token(miniplc0::TokenType::UNSIGNED_INTEGER, (int32_t)1, {0, 3},
                        {0, 4}),
        miniplc0::Token(miniplc0::TokenType::UNSIGNED_INTEGER, (int32_t)999,
                        {0, 5}, {0, 8}),
};

// miniplc0::CompilationError expected_err()

if (result.second.has_value()) {
FAIL("Error introduced in lexing keywords");
} else {
REQUIRE(result.first == expected);
}
}

TEST_CASE("Test Identifiers") {
std::vector<std::pair<std::string, miniplc0::Token>> cases = {
        {"a", miniplc0::Token(miniplc0::TokenType::IDENTIFIER, std::string("a"),
                              {0, 0}, {0, 1})},
        {"abc123", miniplc0::Token(miniplc0::TokenType::IDENTIFIER,
                                   std::string("abc123"), {0, 0}, {0, 6})},
        {"ull", miniplc0::Token(miniplc0::TokenType::IDENTIFIER,
                                std::string("ull"), {0, 0}, {0, 3})},
};

for (auto& c : cases) {
SECTION(fmt::format("case token: {}", c.first)) {
std::stringstream in(c.first);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(res.first.value() == c.second);
}
}
}

TEST_CASE("Test Symbols") {
auto s = "+-*/()=;";
std::stringstream in(s);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();

SECTION("Plus") {
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(res.first.value() == miniplc0::Token(miniplc0::TokenType::PLUS_SIGN,
'+', {0, 0}, {0, 1}));
}

res = lexer.NextToken();
SECTION("Minus") {
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(
        res.first.value() ==
miniplc0::Token(miniplc0::TokenType::MINUS_SIGN, '-', {0, 1}, {0, 2}));
}

res = lexer.NextToken();
SECTION("Mult") {
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(res.first.value() ==
miniplc0::Token(miniplc0::TokenType::MULTIPLICATION_SIGN, '*',
{0, 2}, {0, 3}));
}

res = lexer.NextToken();
SECTION("Div") {
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(res.first.value() ==
miniplc0::Token(miniplc0::TokenType::DIVISION_SIGN, '/', {0, 3},
{0, 4}));
}

res = lexer.NextToken();
SECTION("LParen") {
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(res.first.value() ==
miniplc0::Token(miniplc0::TokenType::LEFT_BRACKET, '(', {0, 4},
{0, 5}));
}

res = lexer.NextToken();
SECTION("RParen") {
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(res.first.value() ==
miniplc0::Token(miniplc0::TokenType::RIGHT_BRACKET, ')', {0, 5},
{0, 6}));
}

res = lexer.NextToken();
SECTION("Equal") {
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(
        res.first.value() ==
miniplc0::Token(miniplc0::TokenType::EQUAL_SIGN, '=', {0, 6}, {0, 7}));
}

res = lexer.NextToken();
SECTION("Semicolon") {
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
REQUIRE(res.first.value() == miniplc0::Token(miniplc0::TokenType::SEMICOLON,
';', {0, 7}, {0, 8}));
}
}

/* ======== Errors ======== */

TEST_CASE("Test number overflow") {
SECTION("Too large numbers obviously overflow") {
std::string ins = "10000000000000000 ";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIntegerOverflow);
}
SECTION("2147483647 does not overflow") {
std::string ins = "2147483647";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE_FALSE(res.second.has_value());
REQUIRE(res.first.has_value());
// REQUIRE(res.second.value().GetCode() ==
// miniplc0::ErrorCode::ErrIntegerOverflow);
}
SECTION("2147483648 overflows") {
std::string ins = "2147483648";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIntegerOverflow);
}
}

TEST_CASE("Test invalid characters") {
SECTION("Single quotes") {
std::string ins = "'";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
SECTION("Double quotes") {
std::string ins = "\"";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
SECTION("Dollar") {
std::string ins = "$";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
SECTION("At") {
std::string ins = "@";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
SECTION("Hashtag") {
std::string ins = "#";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
SECTION("Tilde") {
std::string ins = "~";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
SECTION("Angled bracket") {
std::string ins = "<";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
SECTION("Square bracket") {
std::string ins = "<";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
SECTION("Backslash") {
std::string ins = "\\";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidInput);
}
}

TEST_CASE("EOF should be reported") {
std::string ins = "";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() == miniplc0::ErrorCode::ErrEOF);
}

TEST_CASE("EInvalidIdentifier: digit proceeded with letter") {
std::string ins = "123abc";
std::stringstream in(ins);
miniplc0::Tokenizer lexer(in);
auto res = lexer.NextToken();
REQUIRE(res.second.has_value());
REQUIRE(res.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidIdentifier);
}