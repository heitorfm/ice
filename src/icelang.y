
%include {
	#include <iostream>
	#include <stdlib.h>
	#include <ctype.h>
	#include <stdio.h>
	#include <errno.h>
	#include <limits.h>
	#include <string.h>
	#include <cstdint>
	#include <string>
	#include "Nodes.hpp"
	#include "ParserContext.hpp"
	#include "AstFactory.hpp"
	#include "DebugParser.hpp"

		static Expression* appendPipelineStep(ParserContext* context, Expression* input, FunctionCallExpr* step) {
			if (input && input->nodeType == PIPELINE_EXPR) {
	            PipelineExpr* pipeline = static_cast<PipelineExpr*>(input);
	            pipeline->addStep(step);
	            return pipeline;
			}

	        return context->newPipelineExpr(input, step);
	    }

		static void appendAssignments(StatementsList* statements, AssignmentsList* assignments) {
			if(!statements || !assignments) {
				return;
			}

			for(Assignment* assignment: *assignments) {
				statements->push_back(assignment);
			}
		}

		static StatementsList* statementsFromAssignments(ParserContext* context, AssignmentsList* assignments) {
			StatementsList* statements = context->newStatementsList();
			appendAssignments(statements, assignments);
			return statements;
		}

		struct SectionBody {
			SectionNodeList* sections = nullptr;
			StatementsList* statements = nullptr;
		};

		static SectionBody* newSectionBody(ParserContext* context) {
			SectionBody* body = context->newObject<SectionBody>();
			body->sections = context->newSectionsList();
			body->statements = context->newStatementsList();
			return body;
		}

		static void appendSection(SectionBody* body, SectionNode* section) {
			if(!body || !body->sections || !section) {
				return;
			}

			body->sections->insert(std::make_pair(section->name, section));
		}

		static void appendStatement(SectionBody* body, Node* statement) {
			if(!body || !body->statements || !statement) {
				return;
			}

			body->statements->push_back(statement);
		}

}

/********************************************************************************************/
/********************************     TYPES DEFINITIONS     *********************************/
/********************************************************************************************/

%token_type { Token* }
%default_type { Node* }

%type conf { SectionNodeList* }
%type IDENTIF { Token* }
%type value { DataValue* }
%type valueNN { DataValue* }
%type number { DataValue* }
%type string_parts { StringValue* }
%type string_stmt { DataValue* }
%type array_value { Expression* }
%type array_stmt { Expression* }
%type section_list { SectionNodeList* }
%type section { SectionNode* }
%type section_body { SectionBody* }
%type array_value_list { ExpressionList* }
%type assignment { Assignment* }
%type assignment_list { AssignmentsList* }
%type object_content { AssignmentsList* }
%type object_stmt { Expression* }
%type func_params { AssignmentsList* }
%type statement_list { StatementsList* }
%type boolean_stmt { BooleanStatement* }
%type condition_operand { Expression* }

%type mathexpr { Expression* }
%type math_primary { Expression* }

%type expr { Expression* }
%type primary_expr { Expression* }
%type reference_expr { Expression* }
%type function_call { FunctionCallExpr* }
%type call_args { ExpressionList* }
%type expr_list { ExpressionList* }

/********************************************************************************************/
/********************************         PRECEDENCE        *********************************/
/********************************************************************************************/

%left AND.
%left OR.
%nonassoc EQUALS DIFF GT GTEQ LT LTEQ XOR.
%left PIPE.
%left ADD SUB.
%left MUL DIV MOD.
%right EXP.

%nonassoc RARROW .

/********************************************************************************************/
/**********************     PARSER DIRECTIVES / LEMON SETTINGS     **************************/
/********************************************************************************************/

%extra_argument { ParserContext* context }

%parse_accept { if(context->params.debugParser) printf("The parser has completed successfully.\n"); }

%parse_failure { fprintf(stderr, "Parse failure\n"); }

%syntax_error {
    context->success = 0;

	for (int32_t i = 1, a = 0; i < YYNTOKEN; ++i) {
		a = yy_find_shift_action((YYCODETYPE)i, yypParser->yytos->stateno);
		if (a != YY_ERROR_ACTION) {
				if(context->params.debugParser) printf("possible token: %s\n", yyTokenName[i]);
				context->options.push_back(strdup(yyTokenName[i]));
		}
	}

}

%start_symbol conf


/********************************************************************************************/
/********************************           GRAMMAR         *********************************/
/********************************************************************************************/

/********************************************************/
/******************        CONF        ******************/
/********************************************************/

conf(CONF) ::= ICE_DECL(ICE) REALM_DECL(REA) section_list(SL) END_TOKEN. {
    context->iceDeclaration = ICE->content;
    context->realmDeclaration = REA->content;
    context->astSections = SL;
    CONF = SL;
}

/********************************************************/
/******************       SECTION      ******************/
/********************************************************/

	section(SEC) ::= SECTION(A) LBRACE section_body(Body) RBRACE. {
	    SEC = context->newSection(A->content, Body->sections, Body->statements);
	    if(context->params.debugParser) Ice::DebugParser::printSection(SEC);
	}

	section_body(SB) ::= . {
	    SB = newSectionBody(context);
	}

	section_body(SB) ::= section_body(SB_IN) assignment_list(Assignments). {
	    appendAssignments(SB_IN->statements, Assignments);
	    SB = SB_IN;
	}

	section_body(Body) ::= section_body(SB_IN) statement(Statement). {
	    appendStatement(SB_IN, Statement);
	    Body = SB_IN;
	}

	section_body(SB) ::= section_body(In) section(Section). {
	    appendSection(In, Section);
	    SB = In;
	}

	section_list(SL) ::= section(S). {
	    SectionNodeList* list = context->newSectionsList();
	    list->insert(std::make_pair(S->name, S));
	    SL = list;
	}

	section_list(RES) ::= section_list(SL) section(S). {
	    SL->insert(std::make_pair(S->name, S));
	    RES = SL;
	}

/********************************************************/
/******************       VALUE        ******************/
/********************************************************/

	valueNN(VAL) ::= TRUE. { VAL = context->newValue(true); }
	valueNN(VAL) ::= FALSE. { VAL = context->newValue(false); }
	valueNN(VAL) ::= NULL_TK. { VAL = context->newNullValue(); }
	valueNN(VAL) ::= string_stmt(STR). { VAL = STR; }

	number(VAL) ::= INT_LITERAL(TOK). { VAL = context->newValue(std::stoi(TOK->content)); }
	number(VAL) ::= DOUBLE1(TOK). { VAL = context->newValue(std::stod(TOK->content)); }
	number(VAL) ::= DOUBLE2(TOK). { VAL = context->newValue(std::stod(TOK->content)); }

	value(V) ::= valueNN(VNN). { V = VNN; }

/********************************************************/
/*************  STRING STMT (INTERPOLATED) **************/
/********************************************************/

	string_parts(SV) ::= STRING_LITERAL(S). {
	    SV = context->newStringValue();
	    SV->addPart(S->content, false);
	}

	string_parts(SV) ::= VAR_START VAR_NAME(V) VAR_END. {
	    SV = context->newStringValue();
	    SV->addPart(V->content, true);
	}

	string_parts(RES) ::= string_parts(SV) STRING_LITERAL(S). {
	    SV->addPart(S->content, false);
	    RES = SV;
	}

	string_parts(RES) ::= string_parts(SV) VAR_START VAR_NAME(V) VAR_END. {
	    SV->addPart(V->content, true);
	    RES = SV;
	}

	string_stmt(SS) ::= STRING_START STRING_END. {
	    SS = context->newValue(context->newStringValue());
	}

	string_stmt(SS) ::= STRING_START string_parts(SV) STRING_END. {
	    SS = context->newValue(SV);
	}

/********************************************************/
/******************      EXPRESSION    ******************/
/********************************************************/

	expr(EXPR) ::= primary_expr(P). {
        EXPR = P;
    }

	expr(EXPR) ::= mathexpr(ME). {
		EXPR = ME;
	}

    expr(EXPR) ::= expr(INPUT) PIPE function_call(STEP). {
        EXPR = appendPipelineStep(context, INPUT, STEP);
    }

	primary_expr(EXPR) ::= value(V). {
	   EXPR = context->newLiteralExpr(V);
	}

    primary_expr(EXPR) ::= array_stmt(ARR). {
        EXPR = ARR;
    }

	primary_expr(EXPR) ::= object_stmt(OBJ). {
	    EXPR = OBJ;
	}

	reference_expr(EXPR) ::= REF_START VAR_NAME(V) REF_END. {
	    EXPR = context->newReferenceExpr(V->content);
	}

	expr_list(LIST) ::= expr(EXPR). {
	    ExpressionList* list = context->newExpressionList();
	    list->push_back(EXPR);
	    LIST = list;
	}

	expr_list(LIST) ::= expr_list(LIST_IN) COMMA expr(EXPR). {
		LIST_IN->push_back(EXPR);
		LIST = LIST_IN;
	}

/********************************************************/
/******************     ASSIGNMENT     ******************/
/********************************************************/

	assignment(A) ::= IDENTIF(NAME) SET expr(EXPR). {
		A = context->newAssignment(NAME->content, EXPR);
	}

	assignment_list(AL) ::= assignment(A). {
	    AssignmentsList* list = context->newAssignmentsList();
	    list->push_back(A);
	    AL = list;
	}

	assignment_list(RES) ::= assignment_list(AS) COMMA assignment(A). {
	    AS->push_back(A);
	    RES = AS;
	    if(context->params.debugParser) Ice::DebugParser::printExpression(A->value, 0);
	}

/********************************************************/
/******************      STATEMENT     ******************/
/********************************************************/

	statement(ST) ::= if_stmt(IS). { ST = IS; }
	statement_list(STL) ::= assignment_list(ASL). { STL = statementsFromAssignments(context, ASL); }
	statement_list(STL) ::= statement(S). { StatementsList* stmtList = context->newStatementsList(); stmtList->push_back(S); STL = stmtList; }
	statement_list(RES) ::= statement_list(STL) assignment_list(ASL). { appendAssignments(STL, ASL); RES = STL; }
	statement_list(RES) ::= statement_list(STL) statement(S). { STL->push_back(S); RES = STL; }

/********************************************************/
/******************         IF         ******************/
/********************************************************/

	condition_operand(EXPR) ::= valueNN(V). { EXPR = context->newLiteralExpr(V); }
	condition_operand(EXPR) ::= mathexpr(EXPR_IN). { EXPR = EXPR_IN; }

	boolean_stmt(BST) ::= condition_operand(EXPR). { BST = context->newBooleanStatement(CMP_BOOL_PRIMITIVE, EXPR); }
	boolean_stmt(BST) ::= boolean_stmt(ST1) AND boolean_stmt(ST2). { BST = context->newBooleanStatement(CMP_AND, ST1, ST2); }
	boolean_stmt(BST) ::= boolean_stmt(ST1) OR boolean_stmt(ST2). { BST = context->newBooleanStatement(CMP_OR, ST1, ST2); }
	boolean_stmt(BST) ::= condition_operand(LEFT) EQUALS condition_operand(RIGHT). {
		BST = context->newBooleanStatement(
			CMP_EQUALS,
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, LEFT),
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, RIGHT)
		);
	}
	boolean_stmt(BST) ::= condition_operand(LEFT) DIFF condition_operand(RIGHT). {
		BST = context->newBooleanStatement(
			CMP_DIFF,
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, LEFT),
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, RIGHT)
		);
	}
	boolean_stmt(BST) ::= boolean_stmt(ST1) XOR boolean_stmt(ST2). { BST = context->newBooleanStatement(CMP_XOR, ST1, ST2); }
	boolean_stmt(BST) ::= condition_operand(LEFT) LT condition_operand(RIGHT). {
		BST = context->newBooleanStatement(
			CMP_LT,
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, LEFT),
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, RIGHT)
		);
	}
	boolean_stmt(BST) ::= condition_operand(LEFT) GT condition_operand(RIGHT). {
		BST = context->newBooleanStatement(
			CMP_GT,
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, LEFT),
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, RIGHT)
		);
	}
	boolean_stmt(BST) ::= condition_operand(LEFT) LTEQ condition_operand(RIGHT). {
		BST = context->newBooleanStatement(
			CMP_LTEQ,
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, LEFT),
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, RIGHT)
		);
	}
	boolean_stmt(BST) ::= condition_operand(LEFT) GTEQ condition_operand(RIGHT). {
		BST = context->newBooleanStatement(
			CMP_GTEQ,
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, LEFT),
			context->newBooleanStatement(CMP_BOOL_PRIMITIVE, RIGHT)
		);
	}
	if_stmt(IS) ::= IF LPAREN boolean_stmt(BST) RPAREN LBRACE statement_list(STMTS) RBRACE. { struct IFStmt* stmt = context->newIfStatement(BST); stmt->statements = STMTS; IS = stmt; }

/********************************************************/
/******************        Array       ******************/
/********************************************************/

	array_value(AV) ::= expr(EXPR). {
	    AV = EXPR;
	    if(context->params.debugParser) Ice::DebugParser::printExpression(EXPR, 0);
	}

	array_value_list(AVL) ::= . {
	    AVL = context->newExpressionList();
	}
	array_value_list(AVL) ::= array_value(V). {
	    ExpressionList* list = context->newExpressionList();
	    list->push_back(V);
	    AVL = list;
	}
	array_value_list(RES) ::= array_value_list(AVL) COMMA array_value(V). {
	    AVL->push_back(V);
	    RES = AVL;
	}
	init_array ::=. { if(context->params.debugParser) printf("\tInit Array\n"); }
	end_array ::=. { if(context->params.debugParser) printf("\tEnd Array\n"); }
	array_stmt(AS) ::= init_array LSQUARE array_value_list(AVL) RSQUARE end_array. { AS = context->newArrayExpr(AVL); }

/********************************************************/
/******************       Object       ******************/
/********************************************************/

	init_object ::=. { if(context->params.debugParser) printf("\tInit Object\n"); }
	end_object ::=. { if(context->params.debugParser) printf("\tEnd Object\n"); }
	object_content(OC) ::= . { OC = NULL; }
	object_content(OC) ::= assignment_list(AS). { OC = AS; }
	object_stmt(OS) ::= init_object LBRACE object_content(AS) RBRACE end_object. { OS = context->newObjectExpr(AS); }

/********************************************************/
/******************      CALLABLE      ******************/
/********************************************************/

    call_args(ARGS) ::= . {
        ARGS = NULL;
    }

    call_args(ARGS) ::= expr_list(LIST). {
        ARGS = LIST;
    }

	function_call(F) ::= IDENTIF(NS) RARROW IDENTIF(NAME) LPAREN call_args(ARGS) RPAREN. {
		F = context->newFunctionCallExpr(NS->content, NAME->content, ARGS);
	}

/********************************************************/
/******************    MATH EXPRESSIONS   ***************/
/********************************************************/

	math_primary(EXPR) ::= number(NUM). {
	    EXPR = context->newLiteralExpr(NUM);
	}

	math_primary(EXPR) ::= function_call(F). {
	    EXPR = F;
	}

	math_primary(EXPR) ::= reference_expr(REF). {
	    EXPR = REF;
	}

	math_primary(EXPR) ::= LPAREN mathexpr(INNER) RPAREN. {
	    EXPR = INNER;
	}

	mathexpr(EXPR) ::= math_primary(PRIMARY). {
	    EXPR = PRIMARY;
	}

	mathexpr(EXPR) ::= mathexpr(LEFT) ADD mathexpr(RIGHT). {
	    EXPR = context->newMathExpr(LEFT, MATH_ADD, RIGHT);
	}

	mathexpr(EXPR) ::= mathexpr(LEFT) SUB mathexpr(RIGHT). {
	    EXPR = context->newMathExpr(LEFT, MATH_SUB, RIGHT);
	}

	mathexpr(EXPR) ::= mathexpr(LEFT) MUL mathexpr(RIGHT). {
	    EXPR = context->newMathExpr(LEFT, MATH_MUL, RIGHT);
	}

	mathexpr(EXPR) ::= mathexpr(LEFT) DIV mathexpr(RIGHT). {
	    EXPR = context->newMathExpr(LEFT, MATH_DIV, RIGHT);
	}

	mathexpr(EXPR) ::= mathexpr(LEFT) MOD mathexpr(RIGHT). {
	    EXPR = context->newMathExpr(LEFT, MATH_MOD, RIGHT);
	}

	mathexpr(EXPR) ::= mathexpr(LEFT) EXP mathexpr(RIGHT). {
	    EXPR = context->newMathExpr(LEFT, MATH_EXP, RIGHT);
	}
