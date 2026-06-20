#ifndef PARSERCONTEXT_HPP
#define PARSERCONTEXT_HPP

#include <string>
#include <utility>
#include <vector>
#include "AstFactory.hpp"
#include "IceParserParams.hpp"
#include "Nodes.hpp"

struct ParserContext {
	AstFactory* astFactory = nullptr;
	std::string iceDeclaration;
	std::string realmDeclaration;
	SectionNodeList* astSections = nullptr;
	IceParserParams params;
	bool success = 1;
	std::vector<char*> options; // syntax error options

	template<typename T, typename... Args>
	T* newObject(Args&&... args) {
		return astFactory->make<T>(std::forward<Args>(args)...);
	}

	DataValue* newValue(int value) {
		return astFactory->make<DataValue>(value);
	}

	DataValue* newValue(double value) {
		return astFactory->make<DataValue>(value);
	}

	DataValue* newValue(bool value) {
		return astFactory->make<DataValue>(value);
	}

	DataValue* newValue(StringValue* value) {
		return astFactory->make<DataValue>(value);
	}

	DataValue* newNullValue() {
		return astFactory->make<DataValue>();
	}

	StringValue* newStringValue() {
		return astFactory->make<StringValue>();
	}

	LiteralExpr* newLiteralExpr(DataValue* value) {
		return astFactory->make<LiteralExpr>(value);
	}

	ArrayExpr* newArrayExpr(ExpressionList* items) {
		return astFactory->make<ArrayExpr>(items);
	}

	ObjectExpr* newObjectExpr(AssignmentsList* fields) {
		return astFactory->make<ObjectExpr>(fields);
	}

	ReferenceExpr* newReferenceExpr(const char* path) {
		return astFactory->make<ReferenceExpr>(path);
	}

	FunctionCallExpr* newFunctionCallExpr(const char* namespaceName, const char* functionName, ExpressionList* args) {
		return astFactory->make<FunctionCallExpr>(namespaceName, functionName, args);
	}

	PipelineExpr* newPipelineExpr(Expression* input, FunctionCallExpr* firstStep) {
		return astFactory->make<PipelineExpr>(input, firstStep);
	}

	MathExpr* newMathExpr(Expression* left, const MATH_OPERATION operation, Expression* right) {
		return astFactory->make<MathExpr>(left, operation, right);
	}

	Assignment* newAssignment(const char* name, Expression* value) {
		return astFactory->make<Assignment>(name, value);
	}

	BooleanStatement* newBooleanStatement(COMPARE_TYPE compareType, Expression* expression) {
		return astFactory->make<BooleanStatement>(compareType, expression);
	}

	BooleanStatement* newBooleanStatement(COMPARE_TYPE compareType, BooleanStatement* left, BooleanStatement* right) {
		return astFactory->make<BooleanStatement>(compareType, left, right);
	}

	IFStmt* newIfStatement(BooleanStatement* condition) {
		return astFactory->make<IFStmt>(condition);
	}

	SectionNode* newSection(const char* name) {
		return astFactory->make<SectionNode>(name);
	}

	SectionNode* newSection(const char* name, SectionNodeList* sections) {
		return astFactory->make<SectionNode>(name, sections);
	}

	SectionNode* newSection(const char* name, StatementsList* statements) {
		return astFactory->make<SectionNode>(name, statements);
	}

	SectionNode* newSection(const char* name, SectionNodeList* sections, StatementsList* statements) {
		return astFactory->make<SectionNode>(name, sections, statements);
	}

	SectionNodeList* newSectionsList() {
		return astFactory->make<SectionNodeList>();
	}

	StatementsList* newStatementsList() {
		return astFactory->make<StatementsList>();
	}

	AssignmentsList* newAssignmentsList() {
		return astFactory->make<AssignmentsList>();
	}

	ExpressionList* newExpressionList() {
		return astFactory->make<ExpressionList>();
	}
};

#endif // PARSERCONTEXT_HPP
