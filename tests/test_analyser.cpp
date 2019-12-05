#include "analyser/analyser.h"
#include "instruction/instruction.h"
#include "tokenizer/tokenizer.h"

#include <sstream>
#include <vector>

#include "fmt/core.h"
#include "fmts.hpp"
#include "simple_vm.hpp"
std::ostream& operator<<(std::ostream& os,
                         miniplc0::CompilationError const& t) {
    os << fmt::format("{}", t);
    return os;
}
std::ostream& operator<<(std::ostream& os, miniplc0::Instruction const& t) {
    os << fmt::format("{}", t);
    return os;
}

// template <typename T>
// struct formatter<std::vector<T>> {
//   template <typename ParseContext>
//   constexpr auto parse(ParseContext& ctx) {
//     return ctx.begin();
//   }

//   template <typename FormatContext>
//   auto format(const std::vector<T>& p, FormatContext& ctx) {
//     auto res = format_to(ctx.out(), "[");
//     for (auto& i : p) format_to(ctx.out(), "{}, ", i);
//     return format_to(ctx.out(), "]");
//   }
// };
#include "catch2/catch.hpp"

std::pair<std::vector<miniplc0::Instruction>,
        std::optional<miniplc0::CompilationError>>
analyze(std::string& input) {
    std::stringstream ss(input);
    miniplc0::Tokenizer lexer(ss);
    auto tokens = lexer.AllTokens();
    miniplc0::Analyser parser(tokens.first);
    return parser.Analyse();
}

TEST_CASE("Basic analyzing program") {
std::string input = "begin end";
auto result = analyze(input);
REQUIRE(result.first.size() == 0);
REQUIRE_FALSE(result.second.has_value());
}

TEST_CASE("Constant storing") {
std::string input =
        "begin\n"
        "  const test = 1; \n"
        "end";
auto result = analyze(input);

std::vector<miniplc0::Instruction> expected = {
        miniplc0::Instruction(miniplc0::Operation::LIT, 1),
        // miniplc0::Instruction(miniplc0::Operation::STO, 0),
};

REQUIRE(result.first == expected);

REQUIRE_FALSE(result.second.has_value());
}

TEST_CASE("No assigns to constants") {
std::string input =
        "begin\n"
        "  const test = 1; \n"
        "  test = 2;\n"
        "end";
auto result = analyze(input);

// std::vector<miniplc0::Instruction> expected = {
//     miniplc0::Instruction(miniplc0::Operation::LIT, 1),
//     miniplc0::Instruction(miniplc0::Operation::STO, 0),
// };

// spotted in tests - remember positions start at zero!
auto expected = miniplc0::CompilationError(
        2, 6, miniplc0::ErrorCode::ErrAssignToConstant);

REQUIRE(result.second.value() == expected);
}

TEST_CASE("Variable storing") {
std::string input =
        "begin\n"
        "  var test = 1; \n"
        "end";
auto result = analyze(input);

std::vector<miniplc0::Instruction> expected = {
        miniplc0::Instruction(miniplc0::Operation::LIT, 1),
        // miniplc0::Instruction(miniplc0::Operation::STO, 0),
};

REQUIRE(result.first == expected);

REQUIRE_FALSE(result.second.has_value());
}

TEST_CASE("Variables allow assignments") {
std::string input =
        "begin\n"
        "  var test = 1; \n"
        "  test = 2; \n"
        "end";
auto result = analyze(input);

std::vector<miniplc0::Instruction> expected = {
        miniplc0::Instruction(miniplc0::Operation::LIT, 1),
        // miniplc0::Instruction(miniplc0::Operation::STO, 0),
        miniplc0::Instruction(miniplc0::Operation::LIT, 2),
        miniplc0::Instruction(miniplc0::Operation::STO, 0),
};

REQUIRE(result.first == expected);

REQUIRE_FALSE(result.second.has_value());
}

TEST_CASE("Variables are stored according to declaration order") {
std::string input =
        "begin\n"
        "  var test0 = 0; \n"
        "  var test1 = 1; \n"
        "end";
auto result = analyze(input);

std::vector<miniplc0::Instruction> expected = {
        miniplc0::Instruction(miniplc0::Operation::LIT, 0),
        // miniplc0::Instruction(miniplc0::Operation::STO, 0),
        miniplc0::Instruction(miniplc0::Operation::LIT, 1),
        // miniplc0::Instruction(miniplc0::Operation::STO, 1),
};

REQUIRE(result.first == expected);

REQUIRE_FALSE(result.second.has_value());
}

TEST_CASE("Variables without initial values are initialized with 0") {
std::string input =
        "begin\n"
        "  var test; \n"
        "  var test1 = 1; \n"
        "  test = 2; \n"
        "end";
auto result = analyze(input);

std::vector<miniplc0::Instruction> expected = {
        miniplc0::Instruction(miniplc0::Operation::LIT, 0),
        // miniplc0::Instruction(miniplc0::Operation::STO, 0),
        miniplc0::Instruction(miniplc0::Operation::LIT, 1),
        // miniplc0::Instruction(miniplc0::Operation::STO, 1),
        miniplc0::Instruction(miniplc0::Operation::LIT, 2),
        miniplc0::Instruction(miniplc0::Operation::STO, 0),
};

REQUIRE(result.first == expected);

REQUIRE_FALSE(result.second.has_value());
}

TEST_CASE("Constants and variables act the same in programs") {
std::string input =
        "begin\n"
        "  const a = 1; \n"
        "  var b = 2; \n"
        "  var c; \n"
        "  c = 3;\n"
        "  print(a+b+c); \n"
        "end";
auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();

REQUIRE(vm_result == std::vector{6});
}

TEST_CASE("Constant expressions") {
std::string input =
        "begin\n"
        "  const a = 1; \n"
        "  const b = +1; \n"
        "  const c = -1; \n"
        "  print(a); \n"
        "  print(b); \n"
        "  print(c); \n"
        "end";
auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();

REQUIRE(vm_result == std::vector{1, 1, -1});
}

// ========== Correctness Tests =============

TEST_CASE("Adding random numbers") {
int32_t i = GENERATE(take(30, random(-2147483647 / 2, 2147482647 / 2)));
int32_t j = GENERATE(take(30, random(-2147483647 / 2, 2147482647 / 2)));
SECTION("Add numbers") {
std::string input = fmt::format(
        "begin\n"
        "  var i = {}; \n"
        "  var j = {}; \n"
        "  print(i+j); \n"
        "end",
        i, j);
auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();
std::vector<int32_t> expected = {i + j};

REQUIRE(vm_result == expected);
}
}

TEST_CASE("Subtracting random numbers") {
int32_t i = GENERATE(take(30, random(-2147483647 / 2, 2147482647 / 2)));
int32_t j = GENERATE(take(30, random(-2147483647 / 2, 2147482647 / 2)));
SECTION("Add numbers") {
std::string input = fmt::format(
        "begin\n"
        "  var i = {}; \n"
        "  var j = {}; \n"
        "  print(i-j); \n"
        "end",
        i, j);
auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();
std::vector<int32_t> expected = {i - j};

REQUIRE(vm_result == expected);
}
}

TEST_CASE("Multiplying random numbers") {
int32_t i = GENERATE(take(30, random(-INT16_MAX, INT16_MAX)));
int32_t j = GENERATE(take(30, random(-INT16_MAX, INT16_MAX)));
SECTION("Add numbers") {
std::string input = fmt::format(
        "begin\n"
        "  var i = {}; \n"
        "  var j = {}; \n"
        "  print(i*j); \n"
        "end",
        i, j);
auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();
std::vector<int32_t> expected = {i * j};

REQUIRE(vm_result == expected);
}
}

TEST_CASE("Dividing random numbers") {
int32_t i = GENERATE(take(30, random(-INT32_MAX, INT32_MAX)));
int32_t j = GENERATE(take(30, random(-INT32_MAX, INT32_MAX)));
SECTION("Add numbers") {
std::string input = fmt::format(
        "begin\n"
        "  var i = {}; \n"
        "  var j = {}; \n"
        "  print(i/j); \n"
        "end",
        i, j);
auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();
std::vector<int32_t> expected = {i / j};

REQUIRE(vm_result == expected);
}
}

TEST_CASE("Various expressions") {
SECTION("Deep expression") {
std::string input =
        "begin\n"
        "  const a = 1; \n"
        "  var b = 4; \n"
        "  var c = 215; \n"
        "  b = b * (a + (a * c / b));\n"
        "  print(b); \n"
        "end";
auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();

REQUIRE(vm_result == std::vector{216});
}
SECTION("Multiple expressions") {
std::string input =
        "begin\n"
        "  const a = 8; \n"
        "  var b = 16; \n"
        "  var c = 24; \n"
        "  print(a); \n"
        "  b = b * (a + a * b / c);\n"
        "  c = a - c / (a + b);\n"
        "  print(b); \n"
        "  print(c); \n"
        "end";
auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();

REQUIRE(vm_result == std::vector{8, 208, 8});
}
}

/* ======== Errors ======== */

TEST_CASE("ENoBegin: Main should has 'begin'") {
std::string input =
        "  var test; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() == miniplc0::ErrorCode::ErrNoBegin);
}

TEST_CASE("ENoEnd: Main should has 'end'") {
std::string input =
        "begin \n"
        "  var test; \n"
        "";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() == miniplc0::ErrorCode::ErrNoEnd);
}

TEST_CASE("ENoEnd: consts should appear before vars") {
std::string input =
        "begin \n"
        "  var test; \n"
        "  const test; \n"
        "";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() == miniplc0::ErrorCode::ErrNoEnd);
}

TEST_CASE("EConstantNeedValue: Constants must be initialized") {
std::string input =
        "begin \n"
        "  const test; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrConstantNeedValue);
}

TEST_CASE("EConstantNeedValue: Constants must be assigned with numbers") {
std::string input =
        "begin \n"
        "  const test = 1 * 2; \n"
        "end";
auto result = analyze(input);

// * This really is reported as "Need Semicolon". Not wrong.
REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}

TEST_CASE("EConstantNeedValue: Constants must be initialized (EOF)") {
std::string input =
        "begin \n"
        "  const test";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrConstantNeedValue);
}

TEST_CASE("ENeedIdentifier: Variable declaration need identifiers") {
SECTION("Constant declaration") {
std::string input =
        "begin \n"
        "  const = 4; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNeedIdentifier);
}
SECTION("Variable declaration") {
std::string input =
        "begin \n"
        "  var = 4; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNeedIdentifier);
}
SECTION("Variable declaration (EOL)") {
std::string input =
        "begin \n"
        "  var ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNeedIdentifier);
}
SECTION("Constant declaration (EOF)") {
std::string input =
        "begin \n"
        "  const";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNeedIdentifier);
}
SECTION("Variable declaration (EOF)") {
std::string input =
        "begin \n"
        "  var";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNeedIdentifier);
}
}

TEST_CASE("ENotDeclared: Variable cannot be used without declaration") {
SECTION("As LValue") {
std::string input =
        "begin\n"
        "  test = 1; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNotDeclared);
}

SECTION("As RValue") {
std::string input =
        "begin\n"
        "  var test1 = test; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNotDeclared);
}
}

TEST_CASE("ENotInitialized: Uninitialized variable cannot be used") {
std::string input =
        "begin\n"
        "  var test; \n"
        "  var test1 = test; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNotInitialized);
}

TEST_CASE("EAssignToConstant: Const cannot be assigned") {
std::string input =
        "begin\n"
        "  const test = 1; \n"
        "  test = 1; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrAssignToConstant);
}

TEST_CASE("EDuplicateDeclaration: Variables are declared only once") {
SECTION("Crashing consts with consts") {
std::string input =
        "begin\n"
        "  const test = 1; \n"
        "  const test = 1; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrDuplicateDeclaration);
}
SECTION("Crashing consts with vars") {
std::string input =
        "begin\n"
        "  const test = 1; \n"
        "  var test = 1; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrDuplicateDeclaration);
}
SECTION("Crashing vars with vars") {
std::string input =
        "begin\n"
        "  var test = 1; \n"
        "  var test = 1; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrDuplicateDeclaration);
}
}

TEST_CASE("ENeedSemicolon: Semicolons are needed at every statement") {
SECTION("Semicolon in const declaration") {
std::string input =
        "begin\n"
        "  const test = 1 \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}

SECTION("Semicolon in var declaration") {
std::string input =
        "begin\n"
        "  var test = 1 \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}

SECTION("Semicolon in expression") {
std::string input =
        "begin\n"
        "  var test; \n"
        "  test = 1 \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}

SECTION("Semicolon in print statement") {
std::string input =
        "begin\n"
        "  print(1) \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}
SECTION("Semicolon in const declaration (EOF)") {
std::string input =
        "begin\n"
        "  const test = 1";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}

SECTION("Semicolon in var declaration (EOF)") {
std::string input =
        "begin\n"
        "  var test = 1";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}

SECTION("Semicolon in expression (EOF)") {
std::string input =
        "begin\n"
        "  var test; \n"
        "  test = 1";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}

SECTION("Semicolon in print statement (EOF)") {
std::string input =
        "begin\n"
        "  print(1)";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrNoSemicolon);
}
}

TEST_CASE("EIncompleteExpression: When parameters don't match") {
SECTION("In assignment") {
std::string input =
        "begin\n"
        "  var test = ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("In assignment (EOF)") {
std::string input =
        "begin\n"
        "  var test = ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("In constant number literals - no number") {
std::string input =
        "begin\n"
        "  const j = + ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("In constant number literals - no number (EOF)") {
std::string input =
        "begin\n"
        "  const j = + ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("In constant number literals - no sign or number") {
std::string input =
        "begin\n"
        "  const j = ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("In constant number literals - no sign or number (EOF)") {
std::string input =
        "begin\n"
        "  const j =";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("After a additive operator") {
std::string input =
        "begin\n"
        "  var test = 1 + ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("After a multiplicative operator") {
std::string input =
        "begin\n"
        "  var test = 1 * ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("After a additive operator (EOF)") {
std::string input =
        "begin\n"
        "  var test = 1 +";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("After a multiplicative operator (EOF)") {
std::string input =
        "begin\n"
        "  var test = 1 *";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("With parentheses") {
std::string input =
        "begin\n"
        "  var test = (1 + 1; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("With parentheses (EOF)") {
std::string input =
        "begin\n"
        "  var test = (1 + 1";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
SECTION("One more test (EOF)") {
std::string input =
        "begin\n"
        "  var test = (1 +";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrIncompleteExpression);
}
}
TEST_CASE("EInvalidPrint: When parameters don't match") {
SECTION("Left parenthesis") {
std::string input =
        "begin\n"
        "  print 1 ); \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidPrint);
}
SECTION("Right parenthesis") {
std::string input =
        "begin\n"
        "  print( 1 ; \n"
        "end";
auto result = analyze(input);

REQUIRE(result.second.has_value());
REQUIRE(result.second.value().GetCode() ==
miniplc0::ErrorCode::ErrInvalidPrint);
}
}

// ========= Official tests ==========

TEST_CASE("Official tests - Assign") {
std::string input =
        "begin\n"
        "var a = 1;\n"
        "var b;\n"
        "var c;\n"
        "var d;\n"
        "var e;\n"
        "b = a;\n"
        "e = b;\n"
        "d = e;\n"
        "c = a;\n"
        "print(c);\n"
        "end \n";

auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();
std::vector<int32_t> expected = {1};

REQUIRE(vm_result == expected);
}

TEST_CASE("Official tests - Declaration") {
std::string input =
        "begin\n"
        "const abc = 123;\n"
        "var ABC = 456;\n"
        "print(abc);\n"
        "print(ABC);\n"
        "end";

auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();
std::vector<int32_t> expected = {123, 456};

REQUIRE(vm_result == expected);
}

TEST_CASE("Official tests - Init") {
std::string input =
        "begin\n"
        "var a = 0;\n"
        "var b = 1;\n"
        "var c = a + b;\n"
        "a = b;\n"
        "c = c;\n"
        "c = a + b;\n"
        "a = b;\n"
        "b = c;\n"
        "print(a);\n"
        "print(b);\n"
        "print(c);\n"
        "end";

auto result = analyze(input);

REQUIRE_FALSE(result.second.has_value());

auto vm = miniplc0::VM(result.first);
CAPTURE(result.first);

auto vm_result = vm.Run();
std::vector<int32_t> expected = {1, 2, 2};

REQUIRE(vm_result == expected);
}

TEST_CASE("Official tests - Misssing Begin/End") {
std::string input =
        "var a = 1;\n"
        "print(a);\n";

auto result = analyze(input);

REQUIRE(result.second.has_value());
}

TEST_CASE("Official tests - Misssing Semicolon") {
std::string input =
        "begin\n"
        "const A = 1;\n"
        "var B = A;\n"
        "print(A)\n"
        "end ";

auto result = analyze(input);

REQUIRE(result.second.has_value());
}

TEST_CASE("Official tests - Redeclaration") {
std::string input =
        "begin\n"
        "const A = 1;\n"
        "var A;\n"
        "end";

auto result = analyze(input);

REQUIRE(result.second.has_value());
}

TEST_CASE("Official tests - Uninit") {
std::string input =
        "begin\n"
        "var a;\n"
        "print(a);\n"
        "end";

auto result = analyze(input);

REQUIRE(result.second.has_value());
}

TEST_CASE("Official tests - Var/Const") {
std::string input =
        "begin\n"
        "var a;\n"
        "const b = 1;\n"
        "end";

auto result = analyze(input);

REQUIRE(result.second.has_value());
}