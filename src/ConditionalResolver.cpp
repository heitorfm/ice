#include "ConditionalResolver.hpp"
#include "StrUtil.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace Ice {

namespace {

enum class ScalarType {
	Null,
	Int,
	Double,
	Bool,
	String
};

struct ScalarValue {
	ScalarType type = ScalarType::Null;
	int intValue = 0;
	double doubleValue = 0.0;
	bool boolValue = false;
	std::string stringValue;

	static ScalarValue nullValue() {
		return ScalarValue();
	}

	static ScalarValue fromInt(int value) {
		ScalarValue scalar;
		scalar.type = ScalarType::Int;
		scalar.intValue = value;
		scalar.doubleValue = value;
		return scalar;
	}

	static ScalarValue fromDouble(double value) {
		ScalarValue scalar;
		scalar.type = ScalarType::Double;
		scalar.doubleValue = value;
		return scalar;
	}

	static ScalarValue fromBool(bool value) {
		ScalarValue scalar;
		scalar.type = ScalarType::Bool;
		scalar.boolValue = value;
		return scalar;
	}

	static ScalarValue fromString(std::string value) {
		ScalarValue scalar;
		scalar.type = ScalarType::String;
		scalar.stringValue = std::move(value);
		return scalar;
	}

	bool isNumber() const {
		return type == ScalarType::Int || type == ScalarType::Double;
	}

	double asDouble() const {
		return type == ScalarType::Double ? doubleValue : intValue;
	}
};

struct EvaluationResult {
	bool success = false;
	ScalarValue scalar;
	std::string error;

	static EvaluationResult ok(ScalarValue value) {
		EvaluationResult result;
		result.success = true;
		result.scalar = std::move(value);
		return result;
	}

	static EvaluationResult fail(std::string message) {
		EvaluationResult result;
		result.error = std::move(message);
		return result;
	}
};

struct ConditionResult {
	bool success = false;
	bool value = false;
	std::string error;

	static ConditionResult ok(bool value) {
		ConditionResult result;
		result.success = true;
		result.value = value;
		return result;
	}

	static ConditionResult fail(std::string message) {
		ConditionResult result;
		result.error = std::move(message);
		return result;
	}
};

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

Expression* resolveExpressionPath(Expression* expression, const std::vector<std::string>& parts, size_t index, std::string& error) {
	if (!expression) {
		error = "empty conditional reference target";
		return nullptr;
	}

	if (index >= parts.size()) {
		return expression;
	}

	if (expression->nodeType != OBJECT_EXPR) {
		error = "cannot access '" + joinPath(parts, index) + "' on scalar in conditional";
		return nullptr;
	}

	Assignment* field = findObjectField(static_cast<ObjectExpr*>(expression), parts[index]);
	if (!field) {
		error = "conditional reference not found: " + joinPath(parts, 0);
		return nullptr;
	}

	return resolveExpressionPath(field->value, parts, index + 1, error);
}

Expression* resolveFromSection(SectionNode* section, const std::vector<std::string>& parts, size_t index, std::string& error) {
	if (!section || index >= parts.size()) {
		return nullptr;
	}

	Assignment* assignment = findAssignment(section->statements, parts[index]);
	if (!assignment) {
		return nullptr;
	}

	return resolveExpressionPath(assignment->value, parts, index + 1, error);
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

Expression* resolveReferenceExpression(SectionNodeList* sections, SectionNode* currentSection, const std::string& path, std::string& error) {
	std::vector<std::string> parts = StrUtil::split(path, ".");
	if (!currentSection || parts.empty()) {
		error = "empty conditional reference path";
		return nullptr;
	}

	for (SectionNode* scope = currentSection; scope; scope = scope->parent) {
		Expression* expression = resolveFromSection(scope, parts, 0, error);
		if (expression || !error.empty()) {
			return expression;
		}
	}

	SectionNode* globalSection = findRootSection(sections, parts[0]);
	if (!globalSection) {
		error = "conditional reference not found: " + path;
		return nullptr;
	}

	if (parts.size() == 1) {
		error = "section cannot be used as conditional value: " + parts[0];
		return nullptr;
	}

	return resolveFromSection(globalSection, parts, 1, error);
}

EvaluationResult evaluateExpression(Expression* expression, SectionNodeList* sections, SectionNode* currentSection, int depth);

EvaluationResult evaluateDataValue(DataValue* value) {
	if (!value || value->isNull()) {
		return EvaluationResult::ok(ScalarValue::nullValue());
	}

	if (value->isInt()) {
		return EvaluationResult::ok(ScalarValue::fromInt(value->asInt()));
	}

	if (value->isDouble()) {
		return EvaluationResult::ok(ScalarValue::fromDouble(value->asDouble()));
	}

	if (value->isBool()) {
		return EvaluationResult::ok(ScalarValue::fromBool(value->asBool()));
	}

	if (value->isString()) {
		StringValue* stringValue = value->asString();
		if (stringValue && stringValue->isInterpolated()) {
			return EvaluationResult::fail("interpolated strings cannot be used in IF conditions yet");
		}
		return EvaluationResult::ok(ScalarValue::fromString(stringValue ? stringValue->text() : ""));
	}

	return EvaluationResult::fail("unsupported conditional value");
}

bool isZero(const ScalarValue& scalar) {
	if (scalar.type == ScalarType::Double) {
		return scalar.doubleValue == 0.0;
	}

	return scalar.intValue == 0;
}

EvaluationResult evaluateMath(MathExpr* mathExpression, SectionNodeList* sections, SectionNode* currentSection, int depth) {
	if (!mathExpression) {
		return EvaluationResult::fail("empty math expression in conditional");
	}

	EvaluationResult left = evaluateExpression(mathExpression->left, sections, currentSection, depth + 1);
	if (!left.success) {
		return left;
	}

	EvaluationResult right = evaluateExpression(mathExpression->right, sections, currentSection, depth + 1);
	if (!right.success) {
		return right;
	}

	if (!left.scalar.isNumber() || !right.scalar.isNumber()) {
		return EvaluationResult::fail("math operands in IF conditions must be numeric");
	}

	const bool resultIsDouble = left.scalar.type == ScalarType::Double || right.scalar.type == ScalarType::Double;

	switch (mathExpression->operation) {
		case MATH_ADD:
			if (resultIsDouble) {
				return EvaluationResult::ok(ScalarValue::fromDouble(left.scalar.asDouble() + right.scalar.asDouble()));
			}
			return EvaluationResult::ok(ScalarValue::fromInt(left.scalar.intValue + right.scalar.intValue));
		case MATH_SUB:
			if (resultIsDouble) {
				return EvaluationResult::ok(ScalarValue::fromDouble(left.scalar.asDouble() - right.scalar.asDouble()));
			}
			return EvaluationResult::ok(ScalarValue::fromInt(left.scalar.intValue - right.scalar.intValue));
		case MATH_MUL:
			if (resultIsDouble) {
				return EvaluationResult::ok(ScalarValue::fromDouble(left.scalar.asDouble() * right.scalar.asDouble()));
			}
			return EvaluationResult::ok(ScalarValue::fromInt(left.scalar.intValue * right.scalar.intValue));
		case MATH_DIV:
			if (isZero(right.scalar)) {
				return EvaluationResult::fail("division by zero in IF condition");
			}
			return EvaluationResult::ok(ScalarValue::fromDouble(left.scalar.asDouble() / right.scalar.asDouble()));
		case MATH_MOD:
			if (left.scalar.type != ScalarType::Int || right.scalar.type != ScalarType::Int) {
				return EvaluationResult::fail("modulo in IF conditions requires integer operands");
			}
			if (right.scalar.intValue == 0) {
				return EvaluationResult::fail("modulo by zero in IF condition");
			}
			return EvaluationResult::ok(ScalarValue::fromInt(left.scalar.intValue % right.scalar.intValue));
		case MATH_EXP:
			return EvaluationResult::ok(ScalarValue::fromDouble(std::pow(left.scalar.asDouble(), right.scalar.asDouble())));
	}

	return EvaluationResult::fail("unknown math operation in IF condition");
}

EvaluationResult evaluateExpression(Expression* expression, SectionNodeList* sections, SectionNode* currentSection, int depth) {
	if (!expression) {
		return EvaluationResult::fail("empty expression in IF condition");
	}

	if (depth > 32) {
		return EvaluationResult::fail("recursive conditional reference");
	}

	switch (expression->nodeType) {
		case LITERAL_EXPR: {
			LiteralExpr* literal = static_cast<LiteralExpr*>(expression);
			return evaluateDataValue(literal->value);
		}
		case REFERENCE_EXPR: {
			ReferenceExpr* reference = static_cast<ReferenceExpr*>(expression);
			std::string error;
			Expression* resolved = resolveReferenceExpression(sections, currentSection, reference->path, error);
			if (!resolved) {
				return EvaluationResult::fail(error);
			}
			return evaluateExpression(resolved, sections, currentSection, depth + 1);
		}
		case MATH_EXPR:
			return evaluateMath(static_cast<MathExpr*>(expression), sections, currentSection, depth + 1);
		default:
			return EvaluationResult::fail("IF conditions only support scalar values and numeric math");
	}
}

bool areEqual(const ScalarValue& left, const ScalarValue& right) {
	if (left.isNumber() && right.isNumber()) {
		return left.asDouble() == right.asDouble();
	}

	if (left.type != right.type) {
		return false;
	}

	switch (left.type) {
		case ScalarType::Null: return true;
		case ScalarType::Int: return left.intValue == right.intValue;
		case ScalarType::Double: return left.doubleValue == right.doubleValue;
		case ScalarType::Bool: return left.boolValue == right.boolValue;
		case ScalarType::String: return left.stringValue == right.stringValue;
	}

	return false;
}

ConditionResult compareScalars(COMPARE_TYPE compareType, const ScalarValue& left, const ScalarValue& right) {
	switch (compareType) {
		case CMP_EQUALS:
			return ConditionResult::ok(areEqual(left, right));
		case CMP_DIFF:
			return ConditionResult::ok(!areEqual(left, right));
		case CMP_GT:
		case CMP_GTEQ:
		case CMP_LT:
		case CMP_LTEQ:
			if (!left.isNumber() || !right.isNumber()) {
				return ConditionResult::fail("ordered IF comparisons require numeric operands");
			}
			if (compareType == CMP_GT) {
				return ConditionResult::ok(left.asDouble() > right.asDouble());
			}
			if (compareType == CMP_GTEQ) {
				return ConditionResult::ok(left.asDouble() >= right.asDouble());
			}
			if (compareType == CMP_LT) {
				return ConditionResult::ok(left.asDouble() < right.asDouble());
			}
			return ConditionResult::ok(left.asDouble() <= right.asDouble());
		default:
			return ConditionResult::fail("invalid scalar comparison in IF condition");
	}
}

ConditionResult evaluateCondition(BooleanStatement* condition, SectionNodeList* sections, SectionNode* currentSection);

EvaluationResult evaluateConditionOperand(BooleanStatement* condition, SectionNodeList* sections, SectionNode* currentSection) {
	if (!condition || condition->valueType != BOOL_EXPRESSION) {
		return EvaluationResult::fail("invalid IF condition operand");
	}

	return evaluateExpression(condition->expression, sections, currentSection, 0);
}

ConditionResult evaluateBinaryCondition(BooleanStatement* condition, SectionNodeList* sections, SectionNode* currentSection) {
	if (!condition) {
		return ConditionResult::fail("empty IF condition");
	}

	switch (condition->compareType) {
		case CMP_AND: {
			ConditionResult left = evaluateCondition(condition->left, sections, currentSection);
			if (!left.success || !left.value) {
				return left;
			}
			return evaluateCondition(condition->right, sections, currentSection);
		}
		case CMP_OR: {
			ConditionResult left = evaluateCondition(condition->left, sections, currentSection);
			if (!left.success || left.value) {
				return left;
			}
			return evaluateCondition(condition->right, sections, currentSection);
		}
		case CMP_XOR: {
			ConditionResult left = evaluateCondition(condition->left, sections, currentSection);
			if (!left.success) {
				return left;
			}
			ConditionResult right = evaluateCondition(condition->right, sections, currentSection);
			if (!right.success) {
				return right;
			}
			return ConditionResult::ok(left.value != right.value);
		}
		case CMP_EQUALS:
		case CMP_DIFF:
		case CMP_GT:
		case CMP_GTEQ:
		case CMP_LT:
		case CMP_LTEQ: {
			EvaluationResult left = evaluateConditionOperand(condition->left, sections, currentSection);
			if (!left.success) {
				return ConditionResult::fail(left.error);
			}
			EvaluationResult right = evaluateConditionOperand(condition->right, sections, currentSection);
			if (!right.success) {
				return ConditionResult::fail(right.error);
			}
			return compareScalars(condition->compareType, left.scalar, right.scalar);
		}
		default:
			return ConditionResult::fail("invalid IF condition operator");
	}
}

ConditionResult evaluateCondition(BooleanStatement* condition, SectionNodeList* sections, SectionNode* currentSection) {
	if (!condition) {
		return ConditionResult::fail("empty IF condition");
	}

	if (condition->valueType == BOOL_EXPRESSION) {
		EvaluationResult operand = evaluateExpression(condition->expression, sections, currentSection, 0);
		if (!operand.success) {
			return ConditionResult::fail(operand.error);
		}
		if (operand.scalar.type != ScalarType::Bool) {
			return ConditionResult::fail("standalone IF condition must resolve to bool");
		}
		return ConditionResult::ok(operand.scalar.boolValue);
	}

	return evaluateBinaryCondition(condition, sections, currentSection);
}

bool resolveStatements(StatementsList* statements, SectionNodeList* rootSections, SectionNode* currentSection, bool debugParser) {
	if (!statements) {
		return true;
	}

	bool success = true;
	StatementsList resolvedStatements;

	for (Node* statement: *statements) {
		if (!statement) {
			continue;
		}

		if (statement->nodeType != IF_STMT) {
			resolvedStatements.push_back(statement);
			continue;
		}

		IFStmt* ifStatement = static_cast<IFStmt*>(statement);
		ConditionResult condition = evaluateCondition(ifStatement->condition, rootSections, currentSection);
		if (!condition.success) {
			std::cerr << "Conditional error: " << condition.error << std::endl;
			success = false;
			continue;
		}

		if (debugParser) {
			std::cout << "resolved IF condition as " << (condition.value ? "true" : "false") << std::endl;
		}

		if (!condition.value || !ifStatement->statements) {
			continue;
		}

		if (!resolveStatements(ifStatement->statements, rootSections, currentSection, debugParser)) {
			success = false;
		}

		for (Node* innerStatement: *ifStatement->statements) {
			if (innerStatement) {
				resolvedStatements.push_back(innerStatement);
			}
		}
	}

	*statements = resolvedStatements;
	return success;
}

bool resolveSections(SectionNodeList* sections, SectionNodeList* rootSections, bool debugParser) {
	if (!sections) {
		return true;
	}

	bool success = true;
	for (auto& sectionEntry: *sections) {
		SectionNode* section = sectionEntry.second;
		if (!section) {
			continue;
		}

		if (!resolveStatements(section->statements, rootSections, section, debugParser)) {
			success = false;
		}

		if (!resolveSections(section->childs, rootSections, debugParser)) {
			success = false;
		}
	}

	return success;
}

} // namespace

bool ConditionalResolver::resolveConditionals(SectionNodeList* sections, AstFactory* astFactory, bool debugParser) {
	if (!astFactory) {
		std::cerr << "Conditional error: AST factory is not available" << std::endl;
		return false;
	}

	return resolveSections(sections, sections, debugParser);
}

}
