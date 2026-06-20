#ifndef NODES
#define NODES

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include "StrUtil.hpp"
#include "AstFactory.hpp"
#include "IceParserParams.hpp"

struct Token {
	int type;
	char* content;
};

enum NODE_TYPE {
	ICECONFIG,
	SECTION_LIST,
	NT_SECTION,
	ASSIGNMENT,
	VALUE,
	MATH_EXPR,
	FUNCTION_CALL_EXPR,
	PIPELINE_EXPR,
	REFERENCE_EXPR,
	LITERAL_EXPR,
	ARRAY_EXPR,
	OBJECT_EXPR,
	IF_STMT,
	BOOLEAN_STMT,
	NT_IDENTIF,
};

const char* getNodeTypeName(NODE_TYPE nt);

/************************************************************************/
/************************         NODE        ***************************/
/************************************************************************/

struct Node {
	NODE_TYPE nodeType;

	Node() = default;

	explicit Node(NODE_TYPE type)
		: nodeType(type) {}

	virtual ~Node() {}
};

typedef std::vector<Node*> StatementsList;

class StringValue;

typedef std::variant<std::nullptr_t, int, double, bool, StringValue*> DataValueContent;

class DataValue: public Node {

private:
    DataValueContent content;

	explicit DataValue(DataValueContent value)
		: Node(VALUE),
		  content(value) {}

public:
	DataValue()
		: DataValue(DataValueContent(nullptr)) {}

	explicit DataValue(int value)
		: DataValue(DataValueContent(value)) {}

	explicit DataValue(double value)
		: DataValue(DataValueContent(value)) {}

	explicit DataValue(bool value)
		: DataValue(DataValueContent(value)) {}

	explicit DataValue(StringValue* value)
		: DataValue(DataValueContent(value)) {}

	bool isNull() const {
		return std::holds_alternative<std::nullptr_t>(content);
	}

	bool isString() const {
		return std::holds_alternative<StringValue*>(content);
	}

	bool isInt() const {
		return std::holds_alternative<int>(content);
	}

	bool isDouble() const {
		return std::holds_alternative<double>(content);
	}

	bool isBool() const {
		return std::holds_alternative<bool>(content);
	}

	StringValue* asString() const {
		return std::get<StringValue*>(content);
	}

	int asInt() const {
		return std::get<int>(content);
	}

	double asDouble() const {
		return std::get<double>(content);
	}

	bool asBool() const {
		return std::get<bool>(content);
	}

};

//typedef std::vector<DataValue*> DataValuesList;

/************************************************************************/
/************************      EXPRESSION     ***************************/
/************************************************************************/

struct Assignment;
typedef std::vector<Assignment*> AssignmentsList;

struct Expression : public Node {
	explicit Expression(NODE_TYPE typeArg) {
		nodeType = typeArg;
	}
};

typedef std::vector<Expression*> ExpressionList;

struct LiteralExpr : public Expression {
	DataValue* value;

	explicit LiteralExpr(DataValue* valueArg)
		: Expression(LITERAL_EXPR), value(valueArg) {}
};

struct ArrayExpr : public Expression {
	ExpressionList items;

	explicit ArrayExpr(ExpressionList* itemsArg)
		: Expression(ARRAY_EXPR) {
		if (itemsArg) {
			items = *itemsArg;
		}
	}
};

struct ObjectExpr : public Expression {
	AssignmentsList fields;

	explicit ObjectExpr(AssignmentsList* fieldsArg)
		: Expression(OBJECT_EXPR) {
		if (fieldsArg) {
			fields = *fieldsArg;
		}
	}
};

struct ReferenceExpr : public Expression {
	std::string path;

	explicit ReferenceExpr(std::string pathArg)
		: Expression(REFERENCE_EXPR), path(std::move(pathArg)) {}
};

/************************************************************************/
/************************       FUNCTION      ***************************/
/************************************************************************/

struct FunctionCallExpr : public Expression {
	std::string namespaceName;
	std::string functionName;
	ExpressionList args;

	FunctionCallExpr(std::string functionNameArg, ExpressionList* argsArg)
		: Expression(FUNCTION_CALL_EXPR), functionName(functionNameArg) {
		if (argsArg) {
			args = *argsArg;
		}
	}

	FunctionCallExpr(std::string namespaceNameArg, std::string functionNameArg, ExpressionList* argsArg)
		: Expression(FUNCTION_CALL_EXPR),
		  namespaceName(namespaceNameArg),
		  functionName(functionNameArg) {
		if (argsArg) {
			args = *argsArg;
		}
	}
};

struct PipelineExpr : public Expression {
	Expression* input;
	std::vector<FunctionCallExpr*> steps;

	PipelineExpr(Expression* inputArg, FunctionCallExpr* firstStep)
		: Expression(PIPELINE_EXPR), input(inputArg) {
		steps.push_back(firstStep);
	}

	void addStep(FunctionCallExpr* step) {
		steps.push_back(step);
	}
};

/************************************************************************/
/************************      STRING VALUE    **************************/
/************************************************************************/

struct StringPart {
	std::string content;
	bool isVariable;

	StringPart(const char* contentArg, bool isVariableArg)
		: content(contentArg),
		  isVariable(isVariableArg) {}
};

class StringValue {
public:
	std::vector<StringPart> parts;

	void addPart(const char* content, bool isVariable) {
		parts.emplace_back(content, isVariable);
	}

	bool isInterpolated() const {
		for(const StringPart& part: parts) {
			if(part.isVariable) {
				return true;
			}
		}

		return false;
	}

	std::string text() const {
		std::string result;
		for(const StringPart& part: parts) {
			result += part.content;
		}

		return result;
	}
};


/************************************************************************/
/************************        IDENTIF      ***************************/
/************************************************************************/

struct Identif: public Node {
	char* handler;

	explicit Identif(char* hand) {
		nodeType = NT_IDENTIF;
		handler = hand;
	}
};

/************************************************************************/
/************************      ASSIGNMENT     ***************************/
/************************************************************************/

struct Assignment: public Node {

	std::string name;
	Expression* value;

	Assignment(std::string nameArg, Expression* valueArg) {
		nodeType = ASSIGNMENT;
		name = nameArg;
		value = valueArg;
	}

};

/************************************************************************/
/************************       MATHEXPR      ***************************/
/************************************************************************/

enum MATH_OPERATION {
	MATH_ADD,
	MATH_SUB,
	MATH_MUL,
	MATH_DIV,
	MATH_MOD,
	MATH_EXP
};

struct MathExpr : public Expression {
	Expression* left;
	Expression* right;
	MATH_OPERATION operation;

	MathExpr(Expression* leftArg, const MATH_OPERATION operationArg, Expression* rightArg)
		: Expression(MATH_EXPR),
		  left(leftArg),
		  right(rightArg),
		  operation(operationArg) {}
};

/************************************************************************/
/************************         IF          ***************************/
/************************************************************************/

enum COMPARE_TYPE {
	CMP_EQUALS,
	CMP_GT,
	CMP_GTEQ,
	CMP_LT,
	CMP_LTEQ,
	CMP_XOR,
	CMP_DIFF,
	CMP_AND,
	CMP_OR,
	CMP_BOOL_PRIMITIVE
};

enum BOOLEAN_VALUE_TYPE {
	BOOL_EXPRESSION,
	BOOL_BINARY
};

struct BooleanStatement: public Node {
	COMPARE_TYPE compareType;
	BOOLEAN_VALUE_TYPE valueType;
	Expression* expression;
	BooleanStatement* left;
	BooleanStatement* right;

	BooleanStatement(COMPARE_TYPE ct, Expression* expressionArg)
		: Node(BOOLEAN_STMT),
		  compareType(ct),
		  valueType(BOOL_EXPRESSION),
		  expression(expressionArg),
		  left(nullptr),
		  right(nullptr) {}

	BooleanStatement(COMPARE_TYPE ct, BooleanStatement* leftArg, BooleanStatement* rightArg)
		: Node(BOOLEAN_STMT),
		  compareType(ct),
		  valueType(BOOL_BINARY),
		  expression(nullptr),
		  left(leftArg),
		  right(rightArg) {}

	const char* getCompareTypeName() const {
		switch (compareType) {
			case CMP_EQUALS: return "CMP_EQUALS";
			case CMP_GT: return "CMP_GT";
			case CMP_GTEQ: return "CMP_GTEQ";
			case CMP_LT: return "CMP_LT";
			case CMP_LTEQ: return "CMP_LTEQ";
			case CMP_XOR: return "CMP_XOR";
			case CMP_DIFF: return "CMP_DIFF";
			case CMP_AND: return "CMP_AND";
			case CMP_OR: return "CMP_OR";
			case CMP_BOOL_PRIMITIVE: return "CMP_BOOL_PRIMITIVE";
		}
		return "<NOT-FOUND>";
	}
};

struct IFStmt: public Node {
	BooleanStatement* condition;
	StatementsList* statements;
	IFStmt() {
		nodeType = IF_STMT;
		condition = nullptr;
		statements = nullptr;
	}
	IFStmt(BooleanStatement* cond) {
		nodeType = IF_STMT;
		condition = cond;
		statements = nullptr;
	}

};

/************************************************************************/
/************************       SECTION       ***************************/
/************************************************************************/

struct SectionNode: public Node {

	std::string name;
	std::string identifier;
	std::string qualifier;
    SectionNode* next;
	SectionNode* parent;
	std::map<std::string, SectionNode*>* childs;
	StatementsList* statements;

    SectionNode(std::string nameArg)
        : name(nameArg),
          next(nullptr),
          parent(nullptr),
          childs(nullptr),
          statements(nullptr) {
	    nodeType = NT_SECTION;
	}
	SectionNode(std::string nameArg, std::map<std::string, SectionNode*>* secList)
        : name(nameArg),
          next(nullptr),
          parent(nullptr),
          childs(secList),
          statements(nullptr) {
		nodeType = NT_SECTION;
		attachChildren();
	}
    SectionNode(std::string nameArg, StatementsList* stmts)
        : name(nameArg),
          next(nullptr),
          parent(nullptr),
          childs(nullptr),
          statements(stmts) {
		nodeType = NT_SECTION;
	}
	SectionNode(std::string nameArg, std::map<std::string, SectionNode*>* secList, StatementsList* stmts)
        : name(nameArg),
          next(nullptr),
          parent(nullptr),
          childs(secList),
          statements(stmts) {
		nodeType = NT_SECTION;
		attachChildren();
	}

	void attachChildren() {
		if (!childs) {
			return;
		}

		for (auto& entry: *childs) {
			if (entry.second) {
				entry.second->parent = this;
			}
		}
	}

};

typedef std::map<std::string, SectionNode*> SectionNodeList;


#endif // NODES
