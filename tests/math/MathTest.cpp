#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("math resolver handles precedence grouping operators and references") {
	std::unique_ptr<IceParserResult> result = parseFixture("math/math.ice");
	const Section& math = requireSection(*result, "@math");

	CHECK(requireProperty(math, "precedence").asInt() == 14);
	CHECK(requireProperty(math, "grouped").asInt() == 20);
	CHECK(requireProperty(math, "exponent").asDouble() == 8.0);
	CHECK(requireProperty(math, "modulo").asInt() == 1);
	CHECK(requireProperty(math, "division").asDouble() == 2.5);
	CHECK(requireProperty(math, "refMath").asInt() == 8081);
	CHECK(requireProperty(math, "objectPathMath").asInt() == 9001);
	CHECK(requireProperty(math, "arrayMath").asArray()[1].asInt() == 8082);
	CHECK(requireObjectProperty(requireProperty(math, "objectMath"), "port").asInt() == 8083);
}

TEST_CASE("division by zero returns MathError") {
	std::unique_ptr<IceParserResult> result = parseFixture("math/division-by-zero.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::MathError);
}

TEST_CASE("string operands are rejected in math expressions") {
	std::unique_ptr<IceParserResult> result = parseFixture("math/string-operand.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::SyntaxError);
}
