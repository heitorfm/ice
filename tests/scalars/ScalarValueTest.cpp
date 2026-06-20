#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("scalar values keep their public property types") {
	std::unique_ptr<IceParserResult> result = parseFixture("scalars/scalars.ice");
	const Section& values = requireSection(*result, "@values");

	CHECK(requireProperty(values, "nullValue").isNull());
	CHECK(requireProperty(values, "trueValue").asBool());
	CHECK_FALSE(requireProperty(values, "falseValue").asBool());
	CHECK(requireProperty(values, "intValue").asInt() == 42);
	CHECK(requireProperty(values, "doubleValue").asDouble() == 3.14);
	CHECK(requireProperty(values, "stringValue").asString() == "hello");
	CHECK(requireProperty(values, "emptyString").asString() == "");
	CHECK(requireProperty(values, "path").asString() == "/tmp/app");
	CHECK(requireProperty(values, "overwrite").asString() == "new");
}
