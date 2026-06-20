#include "CallableDescriptorBuilder.hpp"

#include <memory>
#include <utility>

namespace Ice {

namespace {

Property::FunctionCall buildFunctionCall(
	FunctionCallExpr* callExpression,
	const CallableDescriptorBuilder::ExpressionConverter& convertExpression
) {
	Property::FunctionCall functionCall;

	if (!callExpression) {
		return functionCall;
	}

	functionCall.namespaceName = callExpression->namespaceName;
	functionCall.functionName = callExpression->functionName;

	for (Expression* arg: callExpression->args) {
		functionCall.args.push_back(convertExpression(arg));
	}

	return functionCall;
}

}

Property CallableDescriptorBuilder::fromFunctionCall(
	FunctionCallExpr* callExpression,
	const ExpressionConverter& convertExpression
) {
	return Property(buildFunctionCall(callExpression, convertExpression));
}

Property CallableDescriptorBuilder::fromPipeline(
	PipelineExpr* pipelineExpression,
	const ExpressionConverter& convertExpression
) {
	Property::Pipeline pipeline;

	if (!pipelineExpression) {
		return Property(std::move(pipeline));
	}

	pipeline.input = std::make_shared<Property>(convertExpression(pipelineExpression->input));

	for (FunctionCallExpr* step: pipelineExpression->steps) {
		pipeline.steps.push_back(buildFunctionCall(step, convertExpression));
	}

	return Property(std::move(pipeline));
}

}
