#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("public document exposes declarations sections child sections and typed properties") {
	std::unique_ptr<IceParserResult> result = parseFixture("document-api/document.ice");
	const IceDocument* document = result->document();
	REQUIRE(document != nullptr);

	CHECK(document->getIceDeclaration() == "ICE-1.0.0");
	CHECK(document->getRealmDeclaration() == "sample >= 1.0.0");
	REQUIRE(document->getSections().size() == 1);

	const Section& app = requireSection(*result, "@app");
	CHECK(app.name() == "@app");
	CHECK(requireProperty(app, "name").asString() == "api");
	CHECK(requireProperty(app, "ports").asArray()[1].asInt() == 8081);
	CHECK(requireObjectProperty(requireProperty(app, "server"), "host").asString() == "127.0.0.1");

	const Section& runtime = requireChildSection(app, "@runtime");
	CHECK(requireProperty(runtime, "threads").asInt() == 4);
}
