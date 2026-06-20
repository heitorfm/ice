#ifndef DOCUMENTBUILDER_HPP
#define DOCUMENTBUILDER_HPP

#include <memory>
#include <string>
#include "api/IceDocument.hpp"
#include "Nodes.hpp"

namespace Ice {

	class DocumentBuilder {
	public:
		static std::unique_ptr<IceDocument> build(
			const std::string& iceDeclaration,
			const std::string& realmDeclaration,
			SectionNodeList* astSections
		);
	};

}

#endif // DOCUMENTBUILDER_HPP
