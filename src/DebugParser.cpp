
#include "DebugParser.hpp"

#include <iostream>
#include <string>

using namespace std;

const char* TABSEQ = "\t\t\t\t\t\t\t\t\t";

static std::string formatStringValue(StringValue* stringValue) {
	if(!stringValue) {
		return "";
	}

	return stringValue->text();
}

void Ice::DebugParser::printAssignment(const Token& ident, DataValue* value) {
    printAssignment(ident.content.c_str(), value, 0);
}
void Ice::DebugParser::printAssignment(const char* name, DataValue* value, int level) {

    //cout << "LEVEL" << " : " << (*level)+1 << endl;

    if(value != nullptr) {
		printf("%.*s", level, TABSEQ);

		if(value->isInt()) {
			printf("[INT] %s: %d\n", name, value->asInt());
		} else if(value->isDouble()) {
			printf("[DOU] %s: %g\n", name, value->asDouble());
		} else if(value->isString()) {
			std::string content = formatStringValue(value->asString());
			printf("[STR] %s: %s\n", name, content.c_str());
		} else if(value->isBool()) {
			printf("[BOO] %s: %s\n", name, value->asBool() ? "true" : "false");
		} else if(value->isNull()) {
			printf("[NUL] %s: %s\n", name, "null");
		}
    } else {
        printf("%.*s", level, TABSEQ);
        printf("%s: %s\n", name, "<NULL DATA VALUE>");
    }

}

void Ice::DebugParser::printValueInline(DataValue* value) {
	printValue(value, 0, true);
}
void Ice::DebugParser::printValue(DataValue* value) {
    printValue(value, 0, false);
}
void Ice::DebugParser::printValue(DataValue* value, int level) {
	printValue(value, level, false);
}
void Ice::DebugParser::printValue(DataValue* value, int level, bool online) {

	const char* bl = (online) ? "" : "\n";

	if(value != nullptr) {
		if(value->isInt()) {
			printf("%.*s[INT] %d%s", level, TABSEQ, value->asInt(), bl);
		} else if(value->isDouble()) {
			printf("%.*s[DOU] %g%s", level, TABSEQ, value->asDouble(), bl);
		} else if(value->isString()) {
			std::string content = formatStringValue(value->asString());
			printf("%.*s[STR] %s%s", level, TABSEQ, content.c_str(), bl);
		} else if(value->isBool()) {
			printf("%.*s[BOO] %s%s", level, TABSEQ, value->asBool() ? "true" : "false", bl);
		} else if(value->isNull()) {
			printf("%.*s[NUL] %s%s", level, TABSEQ, "null", bl);
		}
    } else {
        printf("%s%s", "<NULL DATA VALUE>", bl);
    }

}

void Ice::DebugParser::printSection(SectionNode* section) {
    printSection(section, 0);
}
void Ice::DebugParser::printSection(SectionNode* section, int level) {
    printf("%.*sSection: %s\n", level, "\t\t\t\t\t\t\t\t", section->name.c_str());

    StatementsList * stmts = section->statements;
    if(stmts && stmts->size() > 0) {

	    for (Node* node: *stmts) {

			switch(node->nodeType) {
				case ASSIGNMENT: {
					Assignment* assi = (Assignment*) node;
					printAssignment(assi->name.c_str(), assi->value, level + 1);
			    }
				break;
					case NT_SECTION: {
						SectionNode* sec = (SectionNode*) node;
						printSection(sec, level + 1);
				}
				break;
				case IF_STMT: {
					IFStmt* stm = (IFStmt*) node;
					printIFStmt(stm, level + 1);
				}
				break;
				}

	    }

    }

    if(section->childs) {
        for (const auto& [key, child]: *section->childs) {
            printSection(child, level+1);
        }
    }
}

void Ice::DebugParser::printSectionFull(SectionNodeList* sections) {

	for (const auto& [key, sec]: *sections) {
        printSection(sec);
    }

}

void Ice::DebugParser::printAssignment(const char* name, Expression* value, int level) {
	printf("%.*s%s: ", level, TABSEQ, name);
	printExpression(value, level);
}

void Ice::DebugParser::printExpression(Expression* expr, int level) {
	if(expr == nullptr) {
		printf("%.*s<NULL EXPR>\n", level, TABSEQ);
		return;
	}

	switch(expr->nodeType) {
		case LITERAL_EXPR: {
			LiteralExpr* literal = static_cast<LiteralExpr*>(expr);
			printValue(literal->value, level);
		}
		break;
		case ARRAY_EXPR: {
			ArrayExpr* array = static_cast<ArrayExpr*>(expr);
			printf("%.*s[ARR]\n", level, TABSEQ);
			for(Expression* item: array->items) {
				printExpression(item, level + 1);
			}
		}
		break;
		case OBJECT_EXPR: {
			ObjectExpr* object = static_cast<ObjectExpr*>(expr);
			printf("%.*s[OBJ]\n", level, TABSEQ);
			for(Assignment* field: object->fields) {
				printAssignment(field->name.c_str(), field->value, level + 1);
			}
		}
		break;
		case FUNCTION_CALL_EXPR: {
			FunctionCallExpr* call = static_cast<FunctionCallExpr*>(expr);
			printf("%.*s[CALL] ", level, TABSEQ);
			if(!call->namespaceName.empty()) {
				printf("%s->", call->namespaceName.c_str());
			}
			printf("%s(", call->functionName.c_str());
			for(size_t i = 0; i < call->args.size(); ++i) {
				if(i > 0) {
					printf(", ");
				}
				printExpression(call->args[i], 0);
			}
			printf(")\n");
		}
		break;
		case PIPELINE_EXPR: {
			PipelineExpr* pipeline = static_cast<PipelineExpr*>(expr);
			printf("%.*s[PIPE]\n", level, TABSEQ);
			printExpression(pipeline->input, level + 1);
			for(FunctionCallExpr* step: pipeline->steps) {
				printExpression(step, level + 1);
			}
		}
		break;
		case REFERENCE_EXPR: {
			ReferenceExpr* reference = static_cast<ReferenceExpr*>(expr);
			printf("%.*s[REF] %s\n", level, TABSEQ, reference->path.c_str());
		}
		break;
		default:
			printf("%.*s[EXPR] %s\n", level, TABSEQ, getNodeTypeName(expr->nodeType));
			break;
	}
}

void printCondition(BooleanStatement* cond) {
	if(cond) {
		if(cond->valueType == BOOL_EXPRESSION) {
			cout << " ";
			Ice::DebugParser::printExpression(cond->expression, 0);
			return;
		}

		cout << " " << cond->getCompareTypeName() << " ";
		if(cond->left) {
			printCondition(cond->left);
		}
		if(cond->right) {
			printCondition(cond->right);
		}

	}
}

void Ice::DebugParser::printIFStmt(IFStmt* stm, int level) {

	printf("%.*sif(", level, TABSEQ);
	printCondition(stm->condition);
	printf(")\n");
}

static void printDocumentProperty(const Property& property, int level);
static void printDocumentPropertyLine(const std::string& name, const Property& property, int level);
static void printDocumentSection(const Section& section, int level);

static void printDocumentFunctionCall(const Property::FunctionCall& call, int level) {
	printf("[CALL] ");
	if (!call.namespaceName.empty()) {
		printf("%s->", call.namespaceName.c_str());
	}
	printf("%s(", call.functionName.c_str());

	if (call.args.empty()) {
		printf(")\n");
		return;
	}

	printf("\n");
	for (size_t i = 0; i < call.args.size(); ++i) {
		printDocumentPropertyLine(std::to_string(i), call.args[i], level + 1);
	}
	printf("%.*s)\n", level, TABSEQ);
}

static void printDocumentPropertyLine(const std::string& name, const Property& property, int level) {
	printf("%.*s%s: ", level, TABSEQ, name.c_str());
	printDocumentProperty(property, level);
}

static void printDocumentProperty(const Property& property, int level) {
	if (property.isInt()) {
		printf("[INT] %d\n", property.asInt());
		return;
	}

	if (property.isDouble()) {
		printf("[DOU] %g\n", property.asDouble());
		return;
	}

	if (property.isBool()) {
		printf("[BOO] %s\n", property.asBool() ? "true" : "false");
		return;
	}

	if (property.isString()) {
		printf("[STR] %s\n", property.asString().c_str());
		return;
	}

	if (property.isArray()) {
		printf("[ARR]\n");
		size_t index = 0;
		for (const Property& item: property.asArray()) {
			printDocumentPropertyLine(std::to_string(index), item, level + 1);
			++index;
		}
		return;
	}

	if (property.isObject()) {
		printf("[OBJ]\n");
		for (const auto& entry: property.asObject()) {
			printDocumentPropertyLine(entry.first, entry.second, level + 1);
		}
		return;
	}

	if (property.isFunctionCall()) {
		printDocumentFunctionCall(property.asFunctionCall(), level);
		return;
	}

	if (property.isPipeline()) {
		printf("[PIPE]\n");
		const Property::Pipeline& pipeline = property.asPipeline();
		if (pipeline.input) {
			printDocumentPropertyLine("input", *pipeline.input, level + 1);
		}
		for (size_t i = 0; i < pipeline.steps.size(); ++i) {
			printf("%.*sstep%zu: ", level + 1, TABSEQ, i);
			printDocumentFunctionCall(pipeline.steps[i], level + 1);
		}
		return;
	}

	printf("[NUL] null\n");
}

static void printDocumentSection(const Section& section, int level) {
	printf("%.*sSection: %s\n", level, TABSEQ, section.name().c_str());
	for (const auto& entry: section.getProperties()) {
		printDocumentPropertyLine(entry.first, entry.second, level + 1);
	}
	for (const Section& child: section.getSections()) {
		printDocumentSection(child, level + 1);
	}
}

void Ice::DebugParser::printIceDocument(IceDocument& document) {

	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	std::cout << "|>>> " << document.getIceDeclaration() << std::endl;
	std::cout << "|>>> " << document.getRealmDeclaration() << std::endl;
	for (const Section& section: document.getSections()) {
		printDocumentSection(section, 0);
	}
	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

}
