#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("string interpolation converts scalar references to text") {
	std::unique_ptr<IceParserResult> result = parseFixture("interpolation/interpolation.ice");
	const Section& app = requireSection(*result, "@app");

	CHECK(requireProperty(app, "message").asString() == "service api on 8080");
	CHECK(requireProperty(app, "boolText").asString() == "true");
	CHECK(requireProperty(app, "nullText").asString() == "null");
	CHECK(requireProperty(app, "globalText").asString() == "global");
}

TEST_CASE("missing interpolation can become blank") {
	IceParserParams params = quietParams();
	params.missingVariableBehavior = MissingVariableBehavior::Blank;

	std::unique_ptr<IceParserResult> result = parseFixture("interpolation/missing.ice", params);
	const Section& app = requireSection(*result, "@app");

	CHECK(requireProperty(app, "value").asString() == "prefix--suffix");
}

TEST_CASE("missing interpolation can preserve the variable marker") {
	IceParserParams params = quietParams();
	params.missingVariableBehavior = MissingVariableBehavior::VarName;

	std::unique_ptr<IceParserResult> result = parseFixture("interpolation/missing.ice", params);
	const Section& app = requireSection(*result, "@app");

	CHECK(requireProperty(app, "value").asString() == "prefix-${notFound}-suffix");
}

TEST_CASE("missing interpolation fails by default") {
	std::unique_ptr<IceParserResult> result = parseFixture("interpolation/missing.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::InterpolationError);
}

TEST_CASE("arrays are not converted automatically by interpolation") {
	std::unique_ptr<IceParserResult> result = parseFixture("interpolation/invalid-array.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::InterpolationError);
}
