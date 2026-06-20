#include "DocumentBuilder.hpp"
#include "CallableDescriptorBuilder.hpp"

namespace Ice {

namespace {

Property propertyFromExpression(Expression* expression);
Section sectionFromNode(SectionNode* sectionNode);

Property propertyFromDataValue(DataValue* value) {
	if (!value || value->isNull()) {
		return Property();
	}

	if (value->isInt()) {
		return Property(value->asInt());
	}

	if (value->isDouble()) {
		return Property(value->asDouble());
	}

	if (value->isBool()) {
		return Property(value->asBool());
	}

	if (value->isString()) {
		StringValue* stringValue = value->asString();
		return Property(stringValue ? stringValue->text() : "");
	}

	return Property();
}

Property propertyFromArray(ArrayExpr* arrayExpression) {
	Property::Array array;

	if (!arrayExpression) {
		return Property(std::move(array));
	}

	for (Expression* item: arrayExpression->items) {
		array.push_back(propertyFromExpression(item));
	}

	return Property(std::move(array));
}

Property propertyFromObject(ObjectExpr* objectExpression) {
	Property::Object object;

	if (!objectExpression) {
		return Property(std::move(object));
	}

	for (Assignment* field: objectExpression->fields) {
		if (field) {
			object[field->name] = propertyFromExpression(field->value);
		}
	}

	return Property(std::move(object));
}

Property propertyFromExpression(Expression* expression) {
	if (!expression) {
		return Property();
	}

	switch (expression->nodeType) {
		case LITERAL_EXPR: {
			LiteralExpr* literal = static_cast<LiteralExpr*>(expression);
			return propertyFromDataValue(literal->value);
		}
		case ARRAY_EXPR:
			return propertyFromArray(static_cast<ArrayExpr*>(expression));
		case OBJECT_EXPR:
			return propertyFromObject(static_cast<ObjectExpr*>(expression));
		case FUNCTION_CALL_EXPR:
			return CallableDescriptorBuilder::fromFunctionCall(
				static_cast<FunctionCallExpr*>(expression),
				propertyFromExpression
			);
		case PIPELINE_EXPR:
			return CallableDescriptorBuilder::fromPipeline(
				static_cast<PipelineExpr*>(expression),
				propertyFromExpression
			);
		case REFERENCE_EXPR:
			return Property();
		default:
			return Property();
	}
}

void copySectionProperties(Section& section, SectionNode* sectionNode) {
	if (!sectionNode || !sectionNode->statements) {
		return;
	}

	for (Node* statement: *sectionNode->statements) {
		if (!statement || statement->nodeType != ASSIGNMENT) {
			continue;
		}

		Assignment* assignment = static_cast<Assignment*>(statement);
		section.setProperty(assignment->name, propertyFromExpression(assignment->value));
	}
}

void copyChildSections(Section& section, SectionNode* sectionNode) {
	if (!sectionNode || !sectionNode->childs) {
		return;
	}

	for (const auto& childEntry: *sectionNode->childs) {
		if (childEntry.second) {
			section.addSection(sectionFromNode(childEntry.second));
		}
	}
}

Section sectionFromNode(SectionNode* sectionNode) {
	Section section(sectionNode ? sectionNode->name : "");
	copySectionProperties(section, sectionNode);
	copyChildSections(section, sectionNode);
	return section;
}

}

std::unique_ptr<IceDocument> DocumentBuilder::build(
	const std::string& iceDeclaration,
	const std::string& realmDeclaration,
	SectionNodeList* astSections
) {
	std::unique_ptr<IceDocument> document = std::make_unique<IceDocument>();
	document->setDeclarations(iceDeclaration, realmDeclaration);

	if (!astSections) {
		return document;
	}

	for (const auto& sectionEntry : *astSections) {
		if (sectionEntry.second) {
			document->addSection(sectionFromNode(sectionEntry.second));
		}
	}

	return document;
}

}
