#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("parser accepts declarations comments sections and child sections") {
	std::unique_ptr<IceParserResult> result = parseFixture("parser-basic/valid-document.ice");
	const Section& app = requireSection(*result, "@app");
	const Section& runtime = requireChildSection(app, "@runtime");
	const Section& empty = requireSection(*result, "@empty");

	CHECK(result->document()->getIceDeclaration() == "ICE-1.0.0");
	CHECK(result->document()->getRealmDeclaration() == "sample >= 1.0.0");
	CHECK(requireProperty(app, "name").asString() == "demo");
	CHECK(requireProperty(runtime, "threads").asInt() == 4);
	CHECK(empty.getProperties().empty());
}

TEST_CASE("parser rejects documents without sections") {
	std::unique_ptr<IceParserResult> result = parseFixture("parser-basic/no-section.ice", true);
	const IceError& error = requireSingleError(*result);

	CHECK(error.code() == IceErrorCode::SyntaxError);
}
