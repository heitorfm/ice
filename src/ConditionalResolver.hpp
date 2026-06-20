#ifndef CONDITIONALRESOLVER_HPP
#define CONDITIONALRESOLVER_HPP

#include "AstFactory.hpp"
#include "Nodes.hpp"

namespace Ice {

class ConditionalResolver {
public:
	static bool resolveConditionals(SectionNodeList* sections, AstFactory* astFactory, bool debugParser);
};

}

#endif // CONDITIONALRESOLVER_HPP
