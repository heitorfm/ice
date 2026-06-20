#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("typed references resolve local parent global and nested object paths") {
	std::unique_ptr<IceParserResult> result = parseFixture("references/references.ice");
	const Section& config = requireSection(*result, "@config");
	const Section& child = requireChildSection(config, "@child");

	CHECK(requireProperty(child, "sameScope").asString() == "from-child");
	CHECK(requireProperty(child, "parentScope").asString() == "from-parent");
	CHECK(requireProperty(child, "globalScope").asString() == "from-global");
	CHECK(requireProperty(child, "host").asString() == "127.0.0.1");
	CHECK(requireProperty(child, "nestedPort").asInt() == 8080);
	CHECK(requireProperty(child, "missingValue").isNull());
}

TEST_CASE("typed references reject navigation through scalar values") {
	std::unique_ptr<IceParserResult> result = parseFixture("references/invalid-scalar-path.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::ReferenceError);
}

TEST_CASE("typed references reject using a section as a scalar value") {
	std::unique_ptr<IceParserResult> result = parseFixture("references/section-as-value.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::ReferenceError);
}
