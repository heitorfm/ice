#include "MathResolver.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <utility>

namespace Ice {

namespace {

struct NumberValue {
	bool isDouble = false;
	int intValue = 0;
	double doubleValue = 0.0;

	static NumberValue fromInt(int value) {
		NumberValue number;
		number.intValue = value;
		number.doubleValue = value;
		return number;
	}

	static NumberValue fromDouble(double value) {
		NumberValue number;
		number.isDouble = true;
		number.doubleValue = value;
		return number;
	}

	double asDouble() const {
		return isDouble ? doubleValue : intValue;
	}
};

struct MathResult {
	bool success = false;
	NumberValue number;
	std::string error;

	static MathResult ok(NumberValue value) {
		MathResult result;
		result.success = true;
		result.number = value;
		return result;
	}

	static MathResult fail(std::string message) {
		MathResult result;
		result.error = std::move(message);
		return result;
	}
};

bool isZero(NumberValue value) {
	return value.isDouble ? value.doubleValue == 0.0 : value.intValue == 0;
}

const char* operationName(MATH_OPERATION operation) {
	switch (operation) {
		case MATH_ADD: return "+";
		case MATH_SUB: return "-";
		case MATH_MUL: return "*";
		case MATH_DIV: return "/";
		case MATH_MOD: return "%";
		case MATH_EXP: return "^";
	}

	return "?";
}

MathResult evaluateExpression(Expression* expression);

MathResult evaluateLiteral(LiteralExpr* literal) {
	if (!literal || !literal->value) {
		return MathResult::fail("math operand is empty");
	}

	if (literal->value->isInt()) {
		return MathResult::ok(NumberValue::fromInt(literal->value->asInt()));
	}

	if (literal->value->isDouble()) {
		return MathResult::ok(NumberValue::fromDouble(literal->value->asDouble()));
	}

	return MathResult::fail("math operand must be int or double");
}

MathResult evaluateOperation(NumberValue left, MATH_OPERATION operation, NumberValue right) {
	const bool resultIsDouble = left.isDouble || right.isDouble;

	switch (operation) {
		case MATH_ADD:
			if (resultIsDouble) {
				return MathResult::ok(NumberValue::fromDouble(left.asDouble() + right.asDouble()));
			}
			return MathResult::ok(NumberValue::fromInt(left.intValue + right.intValue));

		case MATH_SUB:
			if (resultIsDouble) {
				return MathResult::ok(NumberValue::fromDouble(left.asDouble() - right.asDouble()));
			}
			return MathResult::ok(NumberValue::fromInt(left.intValue - right.intValue));

		case MATH_MUL:
			if (resultIsDouble) {
				return MathResult::ok(NumberValue::fromDouble(left.asDouble() * right.asDouble()));
			}
			return MathResult::ok(NumberValue::fromInt(left.intValue * right.intValue));

		case MATH_DIV:
			if (isZero(right)) {
				return MathResult::fail("division by zero");
			}
			return MathResult::ok(NumberValue::fromDouble(left.asDouble() / right.asDouble()));

		case MATH_MOD:
			if (left.isDouble || right.isDouble) {
				return MathResult::fail("modulo requires integer operands");
			}
			if (right.intValue == 0) {
				return MathResult::fail("modulo by zero");
			}
			return MathResult::ok(NumberValue::fromInt(left.intValue % right.intValue));

		case MATH_EXP:
			return MathResult::ok(NumberValue::fromDouble(std::pow(left.asDouble(), right.asDouble())));
	}

	return MathResult::fail("unknown math operation");
}

MathResult evaluateMath(MathExpr* mathExpression) {
	if (!mathExpression) {
		return MathResult::fail("math expression is empty");
	}

	MathResult left = evaluateExpression(mathExpression->left);
	if (!left.success) {
		return left;
	}

	MathResult right = evaluateExpression(mathExpression->right);
	if (!right.success) {
		return right;
	}

	return evaluateOperation(left.number, mathExpression->operation, right.number);
}

MathResult evaluateExpression(Expression* expression) {
	if (!expression) {
		return MathResult::fail("math expression is empty");
	}

	switch (expression->nodeType) {
		case LITERAL_EXPR:
			return evaluateLiteral(static_cast<LiteralExpr*>(expression));
		case MATH_EXPR:
			return evaluateMath(static_cast<MathExpr*>(expression));
		case FUNCTION_CALL_EXPR:
			return MathResult::fail("function call cannot be used as math operand yet");
		case ARRAY_EXPR:
			return MathResult::fail("array cannot be used as math operand");
		case OBJECT_EXPR:
			return MathResult::fail("object cannot be used as math operand");
		case PIPELINE_EXPR:
			return MathResult::fail("pipeline cannot be used as math operand");
		case REFERENCE_EXPR:
			return MathResult::fail("reference cannot be used as math operand before reference resolution");
		default:
			return MathResult::fail("expression cannot be used as math operand");
	}
}

Expression* createNumberExpression(AstFactory* astFactory, NumberValue number) {
	if (number.isDouble) {
		return astFactory->make<LiteralExpr>(astFactory->make<DataValue>(number.doubleValue));
	}

	return astFactory->make<LiteralExpr>(astFactory->make<DataValue>(number.intValue));
}

bool resolveExpression(Expression*& expression, AstFactory* astFactory, bool debugParser);

bool resolveExpressionList(ExpressionList& expressions, AstFactory* astFactory, bool debugParser) {
	bool success = true;

	for (Expression*& expression: expressions) {
		if (!resolveExpression(expression, astFactory, debugParser)) {
			success = false;
		}
	}

	return success;
}

bool resolveAssignment(Assignment* assignment, AstFactory* astFactory, bool debugParser) {
	if (!assignment) {
		return true;
	}

	return resolveExpression(assignment->value, astFactory, debugParser);
}

bool resolveAssignments(AssignmentsList& assignments, AstFactory* astFactory, bool debugParser) {
	bool success = true;

	for (Assignment* assignment: assignments) {
		if (!resolveAssignment(assignment, astFactory, debugParser)) {
			success = false;
		}
	}

	return success;
}

bool resolveExpression(Expression*& expression, AstFactory* astFactory, bool debugParser) {
	if (!expression) {
		return true;
	}

	switch (expression->nodeType) {
		case MATH_EXPR: {
			MathExpr* mathExpression = static_cast<MathExpr*>(expression);
			MathResult result = evaluateMath(mathExpression);
			if (!result.success) {
				std::cerr << "Math error: " << result.error << std::endl;
				return false;
			}

			if (debugParser) {
				std::cout << "resolved math expression using '" << operationName(mathExpression->operation) << "'" << std::endl;
			}

			expression = createNumberExpression(astFactory, result.number);
			return true;
		}

		case ARRAY_EXPR: {
			ArrayExpr* array = static_cast<ArrayExpr*>(expression);
			return resolveExpressionList(array->items, astFactory, debugParser);
		}

		case OBJECT_EXPR: {
			ObjectExpr* object = static_cast<ObjectExpr*>(expression);
			return resolveAssignments(object->fields, astFactory, debugParser);
		}

		case FUNCTION_CALL_EXPR: {
			FunctionCallExpr* call = static_cast<FunctionCallExpr*>(expression);
			return resolveExpressionList(call->args, astFactory, debugParser);
		}

		case PIPELINE_EXPR: {
			bool success = true;
			PipelineExpr* pipeline = static_cast<PipelineExpr*>(expression);
			if (!resolveExpression(pipeline->input, astFactory, debugParser)) {
				success = false;
			}
			for (FunctionCallExpr* step: pipeline->steps) {
				Expression* stepExpression = step;
				if (!resolveExpression(stepExpression, astFactory, debugParser)) {
					success = false;
				}
			}
			return success;
		}

		default:
			return true;
	}
}

bool resolveStatements(StatementsList* statements, AstFactory* astFactory, bool debugParser) {
	if (!statements) {
		return true;
	}

	bool success = true;

	for (Node* statement: *statements) {
		if (!statement || statement->nodeType != ASSIGNMENT) {
			continue;
		}

		Assignment* assignment = static_cast<Assignment*>(statement);
		if (!resolveAssignment(assignment, astFactory, debugParser)) {
			success = false;
		}
	}

	return success;
}

bool resolveSections(SectionNodeList* sections, AstFactory* astFactory, bool debugParser) {
	if (!sections) {
		return true;
	}

	bool success = true;

	for (auto& sectionEntry: *sections) {
		SectionNode* section = sectionEntry.second;
		if (!section) {
			continue;
		}

		if (!resolveStatements(section->statements, astFactory, debugParser)) {
			success = false;
		}

		if (!resolveSections(section->childs, astFactory, debugParser)) {
			success = false;
		}
	}

	return success;
}

} // namespace

bool MathResolver::resolveMathExpressions(SectionNodeList* sections, AstFactory* astFactory, bool debugParser) {
	if (!astFactory) {
		std::cerr << "Math error: AST factory is not available" << std::endl;
		return false;
	}

	return resolveSections(sections, astFactory, debugParser);
}

}
