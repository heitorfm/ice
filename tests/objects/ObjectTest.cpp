#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("objects support empty single and nested fields") {
	std::unique_ptr<IceParserResult> result = parseFixture("objects/objects.ice");
	const Section& objects = requireSection(*result, "@objects");

	CHECK(requireProperty(objects, "empty").asObject().empty());
	CHECK(requireObjectProperty(requireProperty(objects, "single"), "name").asString() == "api");

	const Property& nested = requireProperty(objects, "nested");
	const Property& server = requireObjectProperty(nested, "server");
	CHECK(requireObjectProperty(server, "host").asString() == "127.0.0.1");
	CHECK(requireObjectProperty(server, "port").asInt() == 8081);
	CHECK(requireObjectProperty(nested, "protocols").asArray()[1].asString() == "https");
}

TEST_CASE("objects require commas between fields") {
	std::unique_ptr<IceParserResult> result = parseFixture("objects/missing-comma.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::SyntaxError);
}
