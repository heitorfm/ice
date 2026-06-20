//
// Created by Heitor Machado on 12/09/25.
//

#ifndef ICE_RESOLVER_HPP
#define ICE_RESOLVER_HPP

#include <queue>
#include <string>
#include "IceParserParams.hpp"
#include "Nodes.hpp"

class DataValue;
struct Expression;
struct Assignment;

namespace Ice {

	enum class ResolveStatus {
		Found,
		NotFound,
		InvalidPath
	};

	struct ResolveResult {
		ResolveStatus status = ResolveStatus::NotFound;
		Assignment* assignment = nullptr;
		std::string value;
		std::string error;
	};

	struct InterpolationTarget {
		DataValue* value = nullptr;
		SectionNode* section = nullptr;
	};

	class Resolver {
	public:
		static bool resolveInterpolations(SectionNodeList* sections, const IceParserParams& params);
		static void traverseConfig(SectionNodeList* sections, std::queue<InterpolationTarget>& toResolve, bool debugParser);
		static void traverseExpression(Expression* expr, SectionNode* currentSection, std::queue<InterpolationTarget>& toResolve, bool debugParser);
		static void traverseDataValue(DataValue* dv, SectionNode* currentSection, std::queue<InterpolationTarget>& toResolve, bool debugParser);
		static ResolveResult resolvePath(SectionNodeList* sections, SectionNode* currentSection, const std::string& path);
		static ResolveResult findAssignment(StatementsList* statements, const std::string& name);
	};

} // Ice

#endif //ICE_RESOLVER_HPP
