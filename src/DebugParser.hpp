
#ifndef DEBUGPARSER_HPP
#define DEBUGPARSER_HPP

#include "api/IceDocument.hpp"
#include "Nodes.hpp"

namespace Ice {

	class DebugParser {
	public:
		static void printAssignment(const Token& ident, DataValue* value);

		static void printAssignment(const char* name, DataValue* value, int level);

		static void printValueInline(DataValue* value);

		static void printValue(DataValue* value);

		static void printValue(DataValue* value, int level);

		static void printValue(DataValue* value, int level, bool online);

		static void printSection(SectionNode* section);

		static void printSection(SectionNode* section, int level);

		static void printSectionFull(SectionNodeList* sections);

		static void printIFStmt(IFStmt* stm, int level);

		static void printIceDocument(IceDocument& document);

		static void printExpression(Expression* expr, int level);
		static void printAssignment(const char* name, Expression* value, int level);

	};

}

#endif // DEBUGPARSER_HPP
