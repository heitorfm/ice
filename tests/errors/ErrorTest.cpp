#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("syntax errors are reported without crashing") {
	std::unique_ptr<IceParserResult> result = parseFixture("errors/syntax-error.ice", true);
	CHECK(requireSingleError(*result).code() == IceErrorCode::SyntaxError);
}

TEST_CASE("lexical errors are reported as syntax errors without crashing") {
	std::unique_ptr<IceParserResult> result = parseFixture("errors/lex-error.ice", true);
	CHECK(requireSingleError(*result).code() == IceErrorCode::SyntaxError);
}

TEST_CASE("reference errors are reported") {
	std::unique_ptr<IceParserResult> result = parseFixture("errors/reference-error.ice", true);
	CHECK(requireSingleError(*result).code() == IceErrorCode::ReferenceError);
}

TEST_CASE("interpolation errors are reported") {
	std::unique_ptr<IceParserResult> result = parseFixture("errors/interpolation-error.ice", true);
	CHECK(requireSingleError(*result).code() == IceErrorCode::InterpolationError);
}

TEST_CASE("math errors are reported") {
	std::unique_ptr<IceParserResult> result = parseFixture("errors/math-error.ice", true);
	CHECK(requireSingleError(*result).code() == IceErrorCode::MathError);
}

TEST_CASE("conditional errors are reported") {
	std::unique_ptr<IceParserResult> result = parseFixture("errors/conditional-error.ice", true);
	CHECK(requireSingleError(*result).code() == IceErrorCode::ConditionalError);
}
