#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("arrays preserve order and support mixed expression values") {
	std::unique_ptr<IceParserResult> result = parseFixture("arrays/arrays.ice");
	const Section& arrays = requireSection(*result, "@arrays");

	CHECK(requireProperty(arrays, "empty").asArray().empty());
	CHECK(requireProperty(arrays, "strings").asArray()[1].asString() == "b");
	CHECK(requireProperty(arrays, "numbers").asArray()[0].asInt() == 1);
	CHECK(requireProperty(arrays, "numbers").asArray()[1].asDouble() == 2.5);
	CHECK(requireProperty(arrays, "mixed").asArray()[0].asBool());
	CHECK(requireProperty(arrays, "mixed").asArray()[1].isNull());

	const Property& objects = requireProperty(arrays, "objects");
	REQUIRE(objects.isArray());
	CHECK(requireObjectProperty(objects.asArray()[0], "name").asString() == "api");

	const Property& refs = requireProperty(arrays, "refs");
	REQUIRE(refs.isArray());
	CHECK(refs.asArray()[0].asInt() == 8080);
	CHECK(refs.asArray()[1].asInt() == 8081);
}
