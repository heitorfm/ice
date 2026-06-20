#ifndef CALLABLEDESCRIPTORBUILDER_HPP
#define CALLABLEDESCRIPTORBUILDER_HPP

#include <functional>
#include "api/Property.hpp"
#include "Nodes.hpp"

namespace Ice {

	class CallableDescriptorBuilder {
	public:
		typedef std::function<Property(Expression*)> ExpressionConverter;

		static Property fromFunctionCall(
			FunctionCallExpr* callExpression,
			const ExpressionConverter& convertExpression
		);

		static Property fromPipeline(
			PipelineExpr* pipelineExpression,
			const ExpressionConverter& convertExpression
		);
	};

}

#endif // CALLABLEDESCRIPTORBUILDER_HPP
