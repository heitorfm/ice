#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("conditionals flatten true branches and ignore false branches") {
	std::unique_ptr<IceParserResult> result = parseFixture("conditionals/conditionals.ice");
	const Section& build = requireSection(*result, "@build");

	CHECK(requireProperty(build, "debugSymbols").asBool());
	CHECK(build.getProperty("releaseOnly") == nullptr);
	CHECK(requireProperty(build, "linkerFlags").asArray()[0].asString() == "-pthread");
	CHECK(requireProperty(build, "adminPort").asInt() == 8081);
}

TEST_CASE("references after conditionals use the latest visible assignment") {
	std::unique_ptr<IceParserResult> result = parseFixture("conditionals/conditionals.ice");
	const Section& build = requireSection(*result, "@build");
	const Property& compile = requireProperty(build, "compile");

	REQUIRE(compile.isFunctionCall());
	REQUIRE(compile.asFunctionCall().args.size() == 1);
	CHECK(compile.asFunctionCall().args[0].asArray()[0].asString() == "-g");
}

TEST_CASE("invalid references in conditions return ConditionalError") {
	std::unique_ptr<IceParserResult> result = parseFixture("conditionals/invalid-condition-reference.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::ConditionalError);
}

TEST_CASE("non scalar standalone conditions return ConditionalError") {
	std::unique_ptr<IceParserResult> result = parseFixture("conditionals/invalid-condition-type.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::ConditionalError);
}
