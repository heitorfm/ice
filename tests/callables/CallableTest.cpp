#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("function calls become public descriptors with resolved arguments") {
	std::unique_ptr<IceParserResult> result = parseFixture("callables/callables.ice");
	const Section& build = requireSection(*result, "@build");
	const Property& compile = requireProperty(build, "compile");
	const Property& emptyCall = requireProperty(build, "emptyCall");

	REQUIRE(compile.isFunctionCall());
	CHECK(compile.asFunctionCall().namespaceName == "build");
	CHECK(compile.asFunctionCall().functionName == "compile");
	REQUIRE(compile.asFunctionCall().args.size() == 3);
	CHECK(compile.asFunctionCall().args[0].asString() == "src/main.cpp");
	CHECK(compile.asFunctionCall().args[1].asArray()[0].asString() == "-O2");
	CHECK(requireObjectProperty(compile.asFunctionCall().args[2], "port").asInt() == 8081);

	REQUIRE(emptyCall.isFunctionCall());
	CHECK(emptyCall.asFunctionCall().args.empty());
}

TEST_CASE("function calls require namespace arrow syntax") {
	std::unique_ptr<IceParserResult> result = parseFixture("callables/invalid-call.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::SyntaxError);
}
