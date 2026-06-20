#include "ReferenceResolver.hpp"
#include "StrUtil.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace Ice {

namespace {

enum class ReferenceStatus {
	Found,
	NotFound,
	InvalidPath
};

struct ReferenceResult {
	ReferenceStatus status = ReferenceStatus::NotFound;
	Expression* expression = nullptr;
	SectionNode* section = nullptr;
	std::string error;
};

ReferenceResult found(Expression* expression, SectionNode* section) {
	ReferenceResult result;
	result.status = ReferenceStatus::Found;
	result.expression = expression;
	result.section = section;
	return result;
}

ReferenceResult notFound() {
	return ReferenceResult();
}

ReferenceResult invalid(std::string error) {
	ReferenceResult result;
	result.status = ReferenceStatus::InvalidPath;
	result.error = std::move(error);
	return result;
}

std::string joinPath(const std::vector<std::string>& parts, size_t start) {
	std::string result;
	for (size_t i = start; i < parts.size(); ++i) {
		if (!result.empty()) {
			result += ".";
		}
		result += parts[i];
	}
	return result;
}

Assignment* findAssignment(StatementsList* statements, const std::string& name) {
	if (!statements) {
		return nullptr;
	}

	Assignment* match = nullptr;
	for (Node* statement: *statements) {
		if (!statement || statement->nodeType != ASSIGNMENT) {
			continue;
		}

		Assignment* assignment = static_cast<Assignment*>(statement);
		if (assignment->name == name) {
			match = assignment;
		}
	}

	return match;
}

Assignment* findObjectField(ObjectExpr* object, const std::string& name) {
	if (!object) {
		return nullptr;
	}

	for (Assignment* field: object->fields) {
		if (field && field->name == name) {
			return field;
		}
	}

	return nullptr;
}

ReferenceResult resolveExpressionPath(Expression* expression, SectionNode* section, const std::vector<std::string>& parts, size_t index) {
	if (!expression) {
		return invalid("empty reference target");
	}

	if (index >= parts.size()) {
		return found(expression, section);
	}

	if (expression->nodeType != OBJECT_EXPR) {
		return invalid("cannot access '" + joinPath(parts, index) + "' on scalar");
	}

	ObjectExpr* object = static_cast<ObjectExpr*>(expression);
	Assignment* field = findObjectField(object, parts[index]);
	if (!field) {
		return notFound();
	}

	return resolveExpressionPath(field->value, section, parts, index + 1);
}

ReferenceResult resolveFromSection(SectionNode* section, const std::vector<std::string>& parts, size_t index) {
	if (!section || index >= parts.size()) {
		return notFound();
	}

	Assignment* assignment = findAssignment(section->statements, parts[index]);
	if (!assignment) {
		return notFound();
	}

	return resolveExpressionPath(assignment->value, section, parts, index + 1);
}

SectionNode* findRootSection(SectionNodeList* sections, const std::string& name) {
	if (!sections) {
		return nullptr;
	}

	auto exact = sections->find(name);
	if (exact != sections->end()) {
		return exact->second;
	}

	auto withAt = sections->find("@" + name);
	if (withAt != sections->end()) {
		return withAt->second;
	}

	return nullptr;
}

ReferenceResult resolvePath(SectionNodeList* sections, SectionNode* currentSection, const std::string& path) {
	std::vector<std::string> parts = StrUtil::split(path, ".");
	if (!currentSection || parts.empty()) {
		return invalid("empty reference path");
	}

	for (SectionNode* scope = currentSection; scope; scope = scope->parent) {
		ReferenceResult result = resolveFromSection(scope, parts, 0);
		if (result.status == ReferenceStatus::Found || result.status == ReferenceStatus::InvalidPath) {
			return result;
		}
	}

	SectionNode* globalSection = findRootSection(sections, parts[0]);
	if (!globalSection) {
		return notFound();
	}

	if (parts.size() == 1) {
		return invalid("section cannot be used as reference value: " + parts[0]);
	}

	return resolveFromSection(globalSection, parts, 1);
}

DataValue* cloneDataValue(DataValue* value, AstFactory* astFactory);
Expression* cloneExpression(Expression* expression, AstFactory* astFactory);

StringValue* cloneStringValue(StringValue* stringValue, AstFactory* astFactory) {
	StringValue* clone = astFactory->make<StringValue>();
	if (!stringValue) {
		return clone;
	}

	for (const StringPart& part: stringValue->parts) {
		clone->addPart(part.content.c_str(), part.isVariable);
	}

	return clone;
}

DataValue* cloneDataValue(DataValue* value, AstFactory* astFactory) {
	if (!value || value->isNull()) {
		return astFactory->make<DataValue>();
	}

	if (value->isInt()) {
		return astFactory->make<DataValue>(value->asInt());
	}

	if (value->isDouble()) {
		return astFactory->make<DataValue>(value->asDouble());
	}

	if (value->isBool()) {
		return astFactory->make<DataValue>(value->asBool());
	}

	if (value->isString()) {
		return astFactory->make<DataValue>(cloneStringValue(value->asString(), astFactory));
	}

	return astFactory->make<DataValue>();
}

ExpressionList* cloneExpressionList(const ExpressionList& expressions, AstFactory* astFactory) {
	ExpressionList* clone = astFactory->make<ExpressionList>();
	for (Expression* expression: expressions) {
		clone->push_back(cloneExpression(expression, astFactory));
	}
	return clone;
}

AssignmentsList* cloneAssignments(const AssignmentsList& assignments, AstFactory* astFactory) {
	AssignmentsList* clone = astFactory->make<AssignmentsList>();
	for (Assignment* assignment: assignments) {
		if (assignment) {
			clone->push_back(astFactory->make<Assignment>(assignment->name, cloneExpression(assignment->value, astFactory)));
		}
	}
	return clone;
}

FunctionCallExpr* cloneFunctionCall(FunctionCallExpr* call, AstFactory* astFactory) {
	if (!call) {
		return nullptr;
	}

	return astFactory->make<FunctionCallExpr>(
		call->namespaceName,
		call->functionName,
		cloneExpressionList(call->args, astFactory)
	);
}

Expression* clonePipeline(PipelineExpr* pipeline, AstFactory* astFactory) {
	if (!pipeline || pipeline->steps.empty()) {
		return astFactory->make<LiteralExpr>(astFactory->make<DataValue>());
	}

	PipelineExpr* clone = astFactory->make<PipelineExpr>(
		cloneExpression(pipeline->input, astFactory),
		cloneFunctionCall(pipeline->steps.front(), astFactory)
	);

	for (size_t i = 1; i < pipeline->steps.size(); ++i) {
		clone->addStep(cloneFunctionCall(pipeline->steps[i], astFactory));
	}

	return clone;
}

Expression* cloneExpression(Expression* expression, AstFactory* astFactory) {
	if (!expression) {
		return astFactory->make<LiteralExpr>(astFactory->make<DataValue>());
	}

	switch (expression->nodeType) {
		case LITERAL_EXPR: {
			LiteralExpr* literal = static_cast<LiteralExpr*>(expression);
			return astFactory->make<LiteralExpr>(cloneDataValue(literal->value, astFactory));
		}
		case ARRAY_EXPR: {
			ArrayExpr* array = static_cast<ArrayExpr*>(expression);
			return astFactory->make<ArrayExpr>(cloneExpressionList(array->items, astFactory));
		}
		case OBJECT_EXPR: {
			ObjectExpr* object = static_cast<ObjectExpr*>(expression);
			return astFactory->make<ObjectExpr>(cloneAssignments(object->fields, astFactory));
		}
		case FUNCTION_CALL_EXPR:
			return cloneFunctionCall(static_cast<FunctionCallExpr*>(expression), astFactory);
		case PIPELINE_EXPR:
			return clonePipeline(static_cast<PipelineExpr*>(expression), astFactory);
		case MATH_EXPR: {
			MathExpr* math = static_cast<MathExpr*>(expression);
			return astFactory->make<MathExpr>(
				cloneExpression(math->left, astFactory),
				math->operation,
				cloneExpression(math->right, astFactory)
			);
		}
		case REFERENCE_EXPR: {
			ReferenceExpr* reference = static_cast<ReferenceExpr*>(expression);
			return astFactory->make<ReferenceExpr>(reference->path);
		}
		default:
			return astFactory->make<LiteralExpr>(astFactory->make<DataValue>());
	}
}

Expression* createNullExpression(AstFactory* astFactory) {
	return astFactory->make<LiteralExpr>(astFactory->make<DataValue>());
}

bool resolveExpression(Expression*& expression, SectionNodeList* sections, SectionNode* currentSection, AstFactory* astFactory, bool debugParser);

bool resolveExpressionList(ExpressionList& expressions, SectionNodeList* sections, SectionNode* currentSection, AstFactory* astFactory, bool debugParser) {
	bool success = true;
	for (Expression*& expression: expressions) {
		if (!resolveExpression(expression, sections, currentSection, astFactory, debugParser)) {
			success = false;
		}
	}
	return success;
}

bool resolveAssignments(AssignmentsList& assignments, SectionNodeList* sections, SectionNode* currentSection, AstFactory* astFactory, bool debugParser) {
	bool success = true;
	for (Assignment* assignment: assignments) {
		if (assignment && !resolveExpression(assignment->value, sections, currentSection, astFactory, debugParser)) {
			success = false;
		}
	}
	return success;
}

bool resolveFunctionCall(FunctionCallExpr* call, SectionNodeList* sections, SectionNode* currentSection, AstFactory* astFactory, bool debugParser) {
	if (!call) {
		return true;
	}

	return resolveExpressionList(call->args, sections, currentSection, astFactory, debugParser);
}

bool resolveExpression(Expression*& expression, SectionNodeList* sections, SectionNode* currentSection, AstFactory* astFactory, bool debugParser) {
	if (!expression) {
		return true;
	}

	switch (expression->nodeType) {
		case REFERENCE_EXPR: {
			ReferenceExpr* reference = static_cast<ReferenceExpr*>(expression);
			ReferenceResult result = resolvePath(sections, currentSection, reference->path);
			if (result.status == ReferenceStatus::NotFound) {
				if (debugParser) {
					std::cout << "reference '" << reference->path << "' not found; using null" << std::endl;
				}
				expression = createNullExpression(astFactory);
				return true;
			}

			if (result.status == ReferenceStatus::InvalidPath) {
				std::cerr << "Reference error: " << result.error << std::endl;
				return false;
			}

			if (result.expression == expression || result.expression->nodeType == REFERENCE_EXPR) {
				std::cerr << "Reference error: unresolved or recursive reference: " << reference->path << std::endl;
				return false;
			}

			if (debugParser) {
				std::cout << "resolved reference '" << reference->path << "'" << std::endl;
			}

			expression = cloneExpression(result.expression, astFactory);
			return true;
		}

		case ARRAY_EXPR: {
			ArrayExpr* array = static_cast<ArrayExpr*>(expression);
			return resolveExpressionList(array->items, sections, currentSection, astFactory, debugParser);
		}

		case OBJECT_EXPR: {
			ObjectExpr* object = static_cast<ObjectExpr*>(expression);
			return resolveAssignments(object->fields, sections, currentSection, astFactory, debugParser);
		}

		case FUNCTION_CALL_EXPR:
			return resolveFunctionCall(static_cast<FunctionCallExpr*>(expression), sections, currentSection, astFactory, debugParser);

		case PIPELINE_EXPR: {
			bool success = true;
			PipelineExpr* pipeline = static_cast<PipelineExpr*>(expression);
			if (!resolveExpression(pipeline->input, sections, currentSection, astFactory, debugParser)) {
				success = false;
			}
			for (FunctionCallExpr* step: pipeline->steps) {
				if (!resolveFunctionCall(step, sections, currentSection, astFactory, debugParser)) {
					success = false;
				}
			}
			return success;
		}

		case MATH_EXPR: {
			bool success = true;
			MathExpr* math = static_cast<MathExpr*>(expression);
			if (!resolveExpression(math->left, sections, currentSection, astFactory, debugParser)) {
				success = false;
			}
			if (!resolveExpression(math->right, sections, currentSection, astFactory, debugParser)) {
				success = false;
			}
			return success;
		}

		default:
			return true;
	}
}

bool resolveStatements(StatementsList* statements, SectionNodeList* sections, SectionNode* currentSection, AstFactory* astFactory, bool debugParser) {
	if (!statements) {
		return true;
	}

	bool success = true;
	for (Node* statement: *statements) {
		if (!statement || statement->nodeType != ASSIGNMENT) {
			continue;
		}

		Assignment* assignment = static_cast<Assignment*>(statement);
		if (!resolveExpression(assignment->value, sections, currentSection, astFactory, debugParser)) {
			success = false;
		}
	}
	return success;
}

bool resolveSections(SectionNodeList* sections, SectionNodeList* rootSections, AstFactory* astFactory, bool debugParser) {
	if (!sections) {
		return true;
	}

	bool success = true;
	for (auto& sectionEntry: *sections) {
		SectionNode* section = sectionEntry.second;
		if (!section) {
			continue;
		}

		if (!resolveStatements(section->statements, rootSections, section, astFactory, debugParser)) {
			success = false;
		}

		if (!resolveSections(section->childs, rootSections, astFactory, debugParser)) {
			success = false;
		}
	}
	return success;
}

} // namespace

bool ReferenceResolver::resolveReferences(SectionNodeList* sections, AstFactory* astFactory, bool debugParser) {
	if (!astFactory) {
		std::cerr << "Reference error: AST factory is not available" << std::endl;
		return false;
	}

	return resolveSections(sections, sections, astFactory, debugParser);
}

}
