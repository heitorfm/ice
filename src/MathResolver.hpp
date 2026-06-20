#ifndef MATHRESOLVER_HPP
#define MATHRESOLVER_HPP

#include <string>
#include "AstFactory.hpp"
#include "Nodes.hpp"

namespace Ice {

	class MathResolver {
	public:
		static bool resolveMathExpressions(SectionNodeList* sections, AstFactory* astFactory, bool debugParser = false);
	};

}

#endif // MATHRESOLVER_HPP
