
#ifndef ICE_HPP
#define ICE_HPP

#include <cstdio>
#include <memory>
#include <string>
#include "api/IceDocument.hpp"
#include "api/IceParserResult.hpp"
#include "IceParserParams.hpp"

namespace Ice {

	class IceParser {

	public:
		static std::unique_ptr<IceParserResult> parseFile(FILE* fp, const IceParserParams& params);

	};

}

#endif // ICE_HPP
