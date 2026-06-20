//
// Created by Heitor Machado on 12/09/25.
//

#include "Nodes.hpp"
#include "Resolver.hpp"
#include "StrUtil.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace Ice {

namespace {

ResolveResult makeFound(std::string value) {
	ResolveResult result;
	result.status = ResolveStatus::Found;
	result.value = std::move(value);
	return result;
}

ResolveResult makeNotFound() {
	return ResolveResult();
}

ResolveResult makeNotFound(std::string error) {
	ResolveResult result;
	result.status = ResolveStatus::NotFound;
	result.error = std::move(error);
	return result;
}

ResolveResult makeInvalid(std::string error) {
	ResolveResult result;
	result.status = ResolveStatus::InvalidPath;
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

ResolveResult findObjectField(AssignmentsList* fields, const std::string& name) {
	if (!fields) {
		return makeNotFound();
	}

	for (Assignment* field: *fields) {
		if (field && field->name == name) {
			ResolveResult result;
			result.status = ResolveStatus::Found;
			result.assignment = field;
			return result;
		}
	}

	return makeNotFound();
}

ResolveResult dataValueToString(DataValue* value) {
	if (!value || value->isNull()) {
		return makeFound("null");
	}

	if (value->isInt()) {
		return makeFound(std::to_string(value->asInt()));
	}

	if (value->isDouble()) {
		std::ostringstream stream;
		stream << value->asDouble();
		return makeFound(stream.str());
	}

	if (value->isBool()) {
		return makeFound(value->asBool() ? "true" : "false");
	}

	if (value->isString()) {
		StringValue* stringValue = value->asString();
		if (!stringValue) {
			return makeFound("");
		}

		if (stringValue->isInterpolated()) {
			return makeInvalid("recursive or unresolved interpolation");
		}

		return makeFound(stringValue->text());
	}

	return makeInvalid("unsupported data value");
}

ResolveResult resolveExpression(Expression* expression, const std::vector<std::string>& parts, size_t index) {
	if (!expression) {
		return makeInvalid("empty expression");
	}

	if (index >= parts.size()) {
		switch (expression->nodeType) {
			case LITERAL_EXPR: {
				LiteralExpr* literal = static_cast<LiteralExpr*>(expression);
				return dataValueToString(literal->value);
			}
			case ARRAY_EXPR:
				return makeInvalid("array cannot be converted to string");
			case OBJECT_EXPR:
				return makeInvalid("object cannot be converted to string");
			case FUNCTION_CALL_EXPR:
				return makeInvalid("function call cannot be resolved as interpolation yet");
			case MATH_EXPR:
				return makeInvalid("math expression cannot be resolved as interpolation yet");
			case PIPELINE_EXPR:
				return makeInvalid("pipeline cannot be resolved as interpolation yet");
			default:
				return makeInvalid("expression cannot be converted to string");
		}
	}

	if (expression->nodeType != OBJECT_EXPR) {
		return makeInvalid("cannot access '" + joinPath(parts, index) + "' on scalar");
	}

	ObjectExpr* object = static_cast<ObjectExpr*>(expression);
	ResolveResult fieldResult = findObjectField(&object->fields, parts[index]);
	if (fieldResult.status != ResolveStatus::Found || !fieldResult.assignment) {
		return fieldResult;
	}

	return resolveExpression(fieldResult.assignment->value, parts, index + 1);
}

ResolveResult resolveFromSection(SectionNode* section, const std::vector<std::string>& parts, size_t index) {
	if (!section || index >= parts.size()) {
		return makeNotFound();
	}

	ResolveResult assignmentResult = Resolver::findAssignment(section->statements, parts[index]);
	if (assignmentResult.status != ResolveStatus::Found || !assignmentResult.assignment) {
		return assignmentResult;
	}

	return resolveExpression(assignmentResult.assignment->value, parts, index + 1);
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

void replaceWithResolvedString(DataValue* value, const std::string& resolved) {
	if (!value || !value->isString()) {
		return;
	}

	StringValue* stringValue = value->asString();
	if (!stringValue) {
		return;
	}

	stringValue->parts.clear();
	stringValue->addPart(resolved.c_str(), false);
}

ResolveResult applyMissingVariableBehavior(const std::string& path, MissingVariableBehavior behavior) {
	switch (behavior) {
		case MissingVariableBehavior::Blank:
			return makeFound("");
		case MissingVariableBehavior::VarName:
			return makeFound("${" + path + "}");
		case MissingVariableBehavior::Error:
			return makeInvalid("interpolation not found: " + path);
	}

	return makeInvalid("interpolation not found: " + path);
}

} // namespace

bool Resolver::resolveInterpolations(SectionNodeList* sections, const IceParserParams& params) {

	std::queue<InterpolationTarget> toResolve;

	traverseConfig(sections, toResolve, params.debugParser);

	bool success = true;

	while (!toResolve.empty()) {
		InterpolationTarget target = toResolve.front();
		toResolve.pop();
		bool targetSuccess = true;

		if (!target.value || !target.value->isString()) {
			continue;
		}

		StringValue* stringValue = target.value->asString();
		if (!stringValue) {
			continue;
		}

		std::string resolved;
		for (const StringPart& part: stringValue->parts) {
			if (!part.isVariable) {
				resolved += part.content;
				continue;
			}

			ResolveResult result = resolvePath(sections, target.section, part.content);
			if (result.status == ResolveStatus::NotFound) {
				result = applyMissingVariableBehavior(part.content, params.missingVariableBehavior);
			}

			if (result.status != ResolveStatus::Found) {
				success = false;
				targetSuccess = false;
				std::cerr << "Interpolation error: "
						  << (result.error.empty() ? "interpolation not found: " + part.content : result.error)
						  << std::endl;
				break;
			}

			if (params.debugParser) {
				std::cout << "resolved interpolation '" << part.content << "' => " << result.value << std::endl;
			}

			resolved += result.value;
		}

		if (targetSuccess) {
			replaceWithResolvedString(target.value, resolved);
		}
	}

	return success;
}

void Resolver::traverseConfig(SectionNodeList* sections, std::queue<InterpolationTarget>& toResolve, bool debugParser) {
	if (!sections) {
		return;
	}

    for (auto& secPair : *sections) {

		if (debugParser) {
			std::cout << "::" << secPair.first << std::endl;
		}

        SectionNode* sec = secPair.second;

        if (sec->statements) {
			if (debugParser) {
				std::cout << "- has statements: " << std::endl;
			}
            for (auto stmt : *sec->statements) {
				if (debugParser) {
					std::cout << "\t: " << getNodeTypeName(stmt->nodeType) << std::endl;
				}

	            if (stmt->nodeType == ASSIGNMENT) {

                    Assignment* a = (Assignment*)stmt;

	                    traverseExpression(a->value, sec, toResolve, debugParser);

                }

            }

        }

		if (sec && sec->childs) {
			traverseConfig(sec->childs, toResolve, debugParser);
		}

    }

}

void Resolver::traverseExpression(Expression* expr, SectionNode* currentSection, std::queue<InterpolationTarget>& toResolve, bool debugParser) {

	if(expr == nullptr) {
		return;
	}

	switch(expr->nodeType) {
		case LITERAL_EXPR: {
			LiteralExpr* literal = static_cast<LiteralExpr*>(expr);
			traverseDataValue(literal->value, currentSection, toResolve, debugParser);
		}
		break;
		case ARRAY_EXPR: {
			ArrayExpr* array = static_cast<ArrayExpr*>(expr);
			for(Expression* item: array->items) {
				traverseExpression(item, currentSection, toResolve, debugParser);
			}
		}
		break;
		case OBJECT_EXPR: {
			ObjectExpr* object = static_cast<ObjectExpr*>(expr);
			for(Assignment* field: object->fields) {
				traverseExpression(field->value, currentSection, toResolve, debugParser);
			}
		}
		break;
		case FUNCTION_CALL_EXPR: {
			FunctionCallExpr* call = static_cast<FunctionCallExpr*>(expr);
			for(Expression* arg: call->args) {
				traverseExpression(arg, currentSection, toResolve, debugParser);
			}
		}
		break;
		case PIPELINE_EXPR: {
			PipelineExpr* pipeline = static_cast<PipelineExpr*>(expr);
			traverseExpression(pipeline->input, currentSection, toResolve, debugParser);
			for(FunctionCallExpr* step: pipeline->steps) {
				traverseExpression(step, currentSection, toResolve, debugParser);
			}
		}
		break;
		default:
			break;
	}
}

void Resolver::traverseDataValue(DataValue* dv, SectionNode* currentSection, std::queue<InterpolationTarget>& toResolve, const bool debugParser) {

	if (!dv) {
		return;
	}

	if (dv->isString()) {
		StringValue* stringValue = dv->asString();

		if (debugParser) {
			std::string content = stringValue ? stringValue->text() : "";
			std::cout << "\t\t >>> " << content << std::endl;
		}

		if (stringValue && stringValue->isInterpolated()) {
			toResolve.push({dv, currentSection});
		}

    }

}

ResolveResult Resolver::findAssignment(StatementsList* statements, const std::string& name) {
	ResolveResult result;

	if (!statements) {
		return result;
	}

	for (Node* statement : *statements) {
		if (!statement || statement->nodeType != ASSIGNMENT) {
			continue;
		}

		Assignment* assignment = static_cast<Assignment*>(statement);

		if (assignment->name == name) {
			result.status = ResolveStatus::Found;
			result.assignment = assignment;
			return result;
		}
	}

	return result;

}


ResolveResult Resolver::resolvePath(SectionNodeList* sections, SectionNode* currentSection, const std::string& path) {
	std::vector<std::string> parts = StrUtil::split(path, ".");
	if (!currentSection || parts.empty()) {
		return makeInvalid("empty interpolation path");
	}

	for (SectionNode* scope = currentSection; scope; scope = scope->parent) {
		ResolveResult result = resolveFromSection(scope, parts, 0);
		if (result.status == ResolveStatus::Found || result.status == ResolveStatus::InvalidPath) {
			return result;
		}
	}

	SectionNode* globalSection = findRootSection(sections, parts[0]);
	if (!globalSection) {
		return makeNotFound("interpolation not found: " + path);
	}

	if (parts.size() == 1) {
		return makeInvalid("section cannot be converted to string: " + parts[0]);
	}

	ResolveResult globalResult = resolveFromSection(globalSection, parts, 1);
	if (globalResult.status == ResolveStatus::NotFound) {
		return makeNotFound("interpolation not found: " + path);
	}

	return globalResult;
}

} // Ice
