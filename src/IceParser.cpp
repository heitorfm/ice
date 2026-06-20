
#include "IceParser.hpp"
#include <memory>
#include <queue>
#include <vector>
#include <string>
#include <cstring>
#include "StrUtil.hpp"

std::unique_ptr<IceParserResult> internalParseFile(FILE* fp, const IceParserParams& params);


std::unique_ptr<IceParserResult> Ice::IceParser::parseFile(FILE* fp, const IceParserParams& params) {

	return internalParseFile(fp, params);

}

