#ifndef REFERENCERESOLVER_HPP
#define REFERENCERESOLVER_HPP

#include "AstFactory.hpp"
#include "Nodes.hpp"

namespace Ice {

	class ReferenceResolver {
	public:
		static bool resolveReferences(SectionNodeList* sections, AstFactory* astFactory, bool debugParser = false);
	};

}

#endif // REFERENCERESOLVER_HPP
