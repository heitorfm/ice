#include "TestSupport.hpp"

using namespace IceTests;

TEST_CASE("pipelines become descriptors with input and ordered steps") {
	std::unique_ptr<IceParserResult> result = parseFixture("pipeline/pipeline.ice");
	const Section& build = requireSection(*result, "@build");
	const Property& clean = requireProperty(build, "clean");
	const Property& package = requireProperty(build, "package");

	REQUIRE(clean.isPipeline());
	REQUIRE(clean.asPipeline().input != nullptr);
	CHECK(clean.asPipeline().input->asString() == "dist");
	REQUIRE(clean.asPipeline().steps.size() == 2);
	CHECK(clean.asPipeline().steps[0].namespaceName == "fs");
	CHECK(clean.asPipeline().steps[0].functionName == "remove");
	CHECK(clean.asPipeline().steps[1].functionName == "mkdir");

	REQUIRE(package.isPipeline());
	CHECK(package.asPipeline().input->asString() == "dist/app");
	CHECK(package.asPipeline().steps[1].args[0].asString() == "dist/app.tar");
}

TEST_CASE("pipelines can appear inside objects and arrays") {
	std::unique_ptr<IceParserResult> result = parseFixture("pipeline/pipeline.ice");
	const Section& build = requireSection(*result, "@build");
	const Property& nestedTask = requireObjectProperty(requireProperty(build, "nested"), "task");
	const Property& firstTask = requireProperty(build, "tasks").asArray()[0];

	CHECK(nestedTask.isPipeline());
	CHECK(firstTask.isPipeline());
	CHECK(firstTask.asPipeline().steps[0].functionName == "strip");
}
