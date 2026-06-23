#ifndef ICE_TEST_SUPPORT_HPP
#define ICE_TEST_SUPPORT_HPP

#include "doctest.hpp"

#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include "IceParser.hpp"
#include "api/IceParserResult.hpp"

namespace IceTests {

using FilePtr = std::unique_ptr<FILE, decltype(&fclose)>;

inline std::string fixturePath(const std::string& relativePath) {
	return "tests/" + relativePath;
}

inline std::unique_ptr<IceParserResult> parseFixture(
	const std::string& relativePath,
	const IceParserParams& params,
	bool suppressErrors = false
) {
	const std::string path = fixturePath(relativePath);
	FilePtr file(fopen(path.c_str(), "r"), fclose);
	REQUIRE(file != nullptr);

	std::ostringstream errorSink;
	std::ostringstream outputSink;
	std::streambuf* previousErrorBuffer = nullptr;
	std::streambuf* previousOutputBuffer = nullptr;
	int stderrCopy = -1;
	FilePtr nullFile(nullptr, fclose);
	if (suppressErrors) {
		previousErrorBuffer = std::cerr.rdbuf(errorSink.rdbuf());
		previousOutputBuffer = std::cout.rdbuf(outputSink.rdbuf());
		fflush(stderr);
		stderrCopy = dup(fileno(stderr));
		nullFile.reset(fopen("/dev/null", "w"));
		if (nullFile) {
			dup2(fileno(nullFile.get()), fileno(stderr));
		}
	}

	std::unique_ptr<IceParserResult> result = Ice::IceParser::parseFile(file.get(), params);

	if (suppressErrors) {
		fflush(stderr);
		if (stderrCopy >= 0) {
			dup2(stderrCopy, fileno(stderr));
			close(stderrCopy);
		}
		std::cerr.rdbuf(previousErrorBuffer);
		std::cout.rdbuf(previousOutputBuffer);
	}

	return result;
}

inline IceParserParams quietParams() {
	IceParserParams params;
	params.printConfig = false;
	params.debugParser = false;
	params.info = false;
	return params;
}

inline std::unique_ptr<IceParserResult> parseFixture(const std::string& relativePath, bool suppressErrors = false) {
	return parseFixture(relativePath, quietParams(), suppressErrors);
}

inline const Section& requireSection(const IceParserResult& result, const std::string& name) {
	REQUIRE(result.isSuccess());
	REQUIRE(result.document() != nullptr);

	const Section* section = result.document()->getSection(name);
	REQUIRE(section != nullptr);
	return *section;
}

inline const Section& requireChildSection(const Section& section, const std::string& name) {
	const Section* child = section.getSection(name);
	REQUIRE(child != nullptr);
	return *child;
}

inline const Property& requireProperty(const Section& section, const std::string& name) {
	const Property* property = section.getProperty(name);
	REQUIRE(property != nullptr);
	return *property;
}

inline const Property& requireObjectProperty(const Property& object, const std::string& name) {
	const Property* property = object.getProperty(name);
	REQUIRE(property != nullptr);
	return *property;
}

inline const IceError& requireSingleError(const IceParserResult& result) {
	REQUIRE_FALSE(result.isSuccess());
	REQUIRE(result.getErrors().size() == 1);
	return result.getErrors()[0];
}

}

#endif // ICE_TEST_SUPPORT_HPP
