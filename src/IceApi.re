#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string.h>
#include <vector>
#include "../gen/icelang.h"
#include "StrUtil.hpp"
#include "Nodes.hpp"
#include "ParserContext.hpp"
#include "ConditionalResolver.hpp"
#include "ReferenceResolver.hpp"
#include "Resolver.hpp"
#include "MathResolver.hpp"
#include "DocumentBuilder.hpp"
#include "DebugParser.hpp"
#include "api/IceParserResult.hpp"

#define   YYCTYPE     int
#define   YYCURSOR    s->cur
#define   YYMARKER    s->ptr


#define MODEQ 63
#define MULEQ 65
#define ADDADD 66
#define ADDEQ 67
#define SUBSUB 68
#define SUBEQ 69
#define DIVEQ 70

#define DOT 70

#define LEX_ERROR 666


using namespace std;

using FilePtr = std::unique_ptr<FILE, decltype(&fclose)>;

/* functions to interface the lemon parser */
void *ParseAlloc(void *(*)(size_t));
int Parse(void *, int, Token* token, ParserContext* context);
void ParseFree(void *, void (*)(void*));
void ParseTrace(FILE *stream, char *zPrefix);

enum { REGULAR, STRING, VAR, REF, COMMENTS };

typedef struct Scanner {
    char *top, *cur, *ptr, *pos;
    int line;
	int cond = REGULAR;
	bool debugParser = false;
} Scanner;

bool treatError(const ParserContext& context, Scanner scanner);

static std::unique_ptr<IceParserResult> makeErrorResult(IceErrorCode code, const std::string& message, int line = 0) {
	std::unique_ptr<IceParserResult> result = std::make_unique<IceParserResult>();
	result->addError(IceError(code, message, line));
	return result;
}

static std::string syntaxErrorMessage(const ParserContext& context) {
	std::ostringstream message;
	message << "Syntax error";

	if (!context.options.empty()) {
		message << ", expecting: ";
		for (size_t i = 0; i < context.options.size(); ++i) {
			if (i > 0) {
				message << " OR ";
			}
			message << "'" << context.options[i] << "'";
		}
	}

	return message.str();
}

/*  RE2C SETUP  */
/*!re2c
    re2c:yyfill:enable = 0;
*/

int scan(Scanner* s, char *buff_end) {

	/*  RE2C MATCHERS  */
	/*!re2c
		ALPHANUMS = [a-zA-Z0-9]+;
		whitespace = [ \t\v\f]+;
		dig = [0-9];
		let = [a-zA-Z_];
		hex = [a-fA-F0-9];
		int_des = [uUlL]*;
		any = [\000-\377];
		version = dig+"."dig+"."dig+;
		OPT_SPA = " "*;
		varName = let (let|dig)*;
		identif = (varName ("." varName)*);
		tab = " "{4}|[\t]{1};
	 */

	for (;;) {
		if (s->cur >= buff_end) {
			return END_TOKEN;
		}

		s->top = s->cur;

		switch(s->cond) {
			case REGULAR:
				goto regular_state;
			case STRING:
				goto string_state;
			case VAR:
				goto var_state;
			case REF:
				goto ref_state;
			case COMMENTS:
				goto comments_state;
		}

regular_state:
/*!re2c
	"ICE-"version   { return ICE_DECL; }
	identif OPT_SPA ">=" OPT_SPA version
		  { return REALM_DECL; }
	"#"             { s->cond = COMMENTS; continue; }
	":"             { return SET; }
	'('             { return LPAREN; }
	")"             { return RPAREN; }
	"{"             { return LBRACE; }
	"}"             { return RBRACE; }
	"["             { return LSQUARE; }
	"]"             { return RSQUARE; }
	","             { return COMMA; }
	"."             { return DOT; }

	// Start of string-mode
	["]             { if (s->debugParser) printf("STRING_START, switching to STRING mode\n"); s->cond = STRING; return STRING_START; }

	("0" [xX] hex+ int_des?) | ("0" dig+ int_des?) | (dig+ int_des?)
		{ return INT_LITERAL; }
	[0-9]+ "." [0-9]{1,4}  { return DOUBLE1; }
	[0-9]+ ([+-]? [eE] [0-9]+)? { return DOUBLE2; }

	"true"          { return TRUE; }
	"false"         { return FALSE; }
	"null"          { return NULL_TK; }
	"IF"            { return IF; }
	"if"            { return IF; }
	"AND"           { return AND; }
	"and"           { return AND; }
	"OR"            { return OR; }
	"or"            { return OR; }
	"XOR"           { return XOR; }
	"xor"           { return XOR; }

	"->"            { return RARROW; }
	"|>"            { return PIPE; }
	"${"            { s->cond = REF; return REF_START; }

	identif         { return IDENTIF; }

	whitespace      { continue; }  // skip

	"+"             { return ADD; }
	"+="            { return ADDEQ; }
	"++"            { return ADDADD; }
	"-"             { return SUB; }
	"-="            { return SUBEQ; }
	"--"            { return SUBSUB; }
	"**"            { return EXP; }
	"*"             { return MUL; }
	"*="            { return MULEQ; }
	"/"             { return DIV; }
	"/="            { return DIVEQ; }
	"%"             { return MOD; }
	"%="            { return MODEQ; }

	"=="            { return EQUALS; }
	"!="            { return DIFF; }
	"<"             { return LT; }
	">"             { return GT; }
	"<="            { return LTEQ; }
	">="            { return GTEQ; }
	"^"             { return XOR; }
	"&&"            { return AND; }
	"||"            { return OR; }

	"@" identif     { return SECTION; }

	"\r\n"|"\n" {
		s->pos = s->cur;
		s->line++;
		continue;
	}

	any {
		printf("unexpected character: %c (0x%02x) at position %ld\n", *s->cur, *s->cur, s->cur - s->top);
		return LEX_ERROR;
	}
*/

string_state:
/*!re2c
	[^"$]+          { if (s->debugParser) printf("STRING_LITERAL: %.*s\n", (int)(s->cur - s->top), s->top); return STRING_LITERAL; }     // Text parts
	"${"            { s->cond = VAR; return VAR_START; }          // ${ - vai para modo variável
	"\""            { s->cond = REGULAR; return STRING_END; }  // End quote
*/

var_state:
/*!re2c
	[^}]+           { return VAR_NAME; }        // variable.name
	"}"             { s->cond = STRING; return VAR_END; }  // } - volta para string_mode
*/

ref_state:
/*!re2c
	whitespace      { continue; }
	identif         { return VAR_NAME; }
	"}"             { s->cond = REGULAR; return REF_END; }
	any             { return LEX_ERROR; }
*/

comments_state:
/*!re2c
	"\n"          { s->cond = REGULAR; s->line++; continue; }
	any           { continue; } // ignore until newline
*/

	}

}

std::unique_ptr<IceParserResult> internalParseFile(FILE *fp, const IceParserParams& params) {

    FilePtr traceFile(nullptr, fclose);
    long size;
    char* buff_end;
    size_t bytes;
    int token;
    Scanner scanner;
    void *parser;

    std::vector<char> stringBuffer;
    int strBufIni = 0;

    if (fseek(fp, 0, SEEK_END) != 0) {
        return makeErrorResult(IceErrorCode::ReadError, "Could not seek file");
    }

    size = ftell(fp);
    if (size < 0) {
        return makeErrorResult(IceErrorCode::ReadError, "Could not read file size");
    }

    rewind(fp);
    stringBuffer.resize(static_cast<size_t>(size) * 2 + 1);

	std::vector<char> inputBuffer(static_cast<size_t>(size));
    bytes = fread(inputBuffer.data(), 1, inputBuffer.size(), fp);
	if (bytes != inputBuffer.size()) {
		return makeErrorResult(IceErrorCode::ReadError, "Reading error");
	}

    /* Start scanning */
	scanner.top = inputBuffer.data();
	scanner.cur = inputBuffer.data();
	scanner.pos = inputBuffer.data();
    scanner.line = 1;
    scanner.debugParser = params.debugParser;

    buff_end = inputBuffer.data() + inputBuffer.size();

	//   ***** CREATE PARSER *****
    parser = ParseAlloc(malloc);

    // trace parser
	if (params.debugParser) {
		traceFile.reset(fopen("trace.out", "w"));
		if (!traceFile) {
			ParseFree(parser, free);
			return makeErrorResult(IceErrorCode::InternalError, "Can't open trace file");
		}

		char prefix[] = "parser >> ";
		ParseTrace(traceFile.get(), prefix);
	}

    ParserContext context;
	context.params = params;
	AstFactory astFactory;
	context.astFactory = &astFactory;

    bool isRunning = 1;

    while(isRunning) {

        token = scan(&scanner, buff_end);

        if (context.params.debugParser) {
            cout << token << endl;
        }

		if(token == ICE_DECL || token == REALM_DECL) {
			int stringLength = scanner.cur - scanner.top;
			strncpy(&stringBuffer[strBufIni], scanner.top, stringLength);
			stringBuffer[strBufIni + stringLength] = '\0';

			if (context.params.debugParser) {
				std::cout << &stringBuffer[strBufIni] << std::endl;
			}

			Token* curToken = new Token{token, &stringBuffer[strBufIni]};

			int res = Parse(parser, token, curToken, &context);
			isRunning = treatError(context, scanner);

			strBufIni += stringLength + 1;

		} else if(token == STRING_LITERAL) {

			int stringLength = scanner.cur - scanner.top;
			strncpy(&stringBuffer[strBufIni], scanner.top, stringLength);
			stringBuffer[strBufIni + stringLength] = '\0';

			Token* curToken = new Token{token, &stringBuffer[strBufIni]};

			int res = Parse(parser, token, curToken, &context);
			isRunning = treatError(context, scanner);

			strBufIni += stringLength + 1;

		} else if(token == INT_LITERAL) {

			int stringLength = scanner.cur - scanner.top;
			strncpy(&stringBuffer[strBufIni], scanner.top, stringLength);
			stringBuffer[strBufIni + stringLength] = '\0';

			Token* curToken = new Token{token, &stringBuffer[strBufIni]};

			int res = Parse(parser, token, curToken, &context);
			isRunning = treatError(context, scanner);
			strBufIni += stringLength + 1;

		} else if(token == FALSE || token == TRUE) {

			int stringLength = scanner.cur - scanner.top;
			strncpy(&stringBuffer[strBufIni], scanner.top, stringLength);
			stringBuffer[strBufIni + stringLength] = '\0';

			Token* curToken = new Token{token, &stringBuffer[strBufIni]};

			int res = Parse(parser, token, curToken, &context);
			isRunning = treatError(context, scanner);
			strBufIni += stringLength + 1;

		} else if(token == DOUBLE1 || token == DOUBLE2) {

			int stringLength = scanner.cur - scanner.top;
			strncpy(&stringBuffer[strBufIni], scanner.top, stringLength);
			stringBuffer[strBufIni + stringLength] = '\0';

			Token* curToken = new Token{token, &stringBuffer[strBufIni]};

			int res = Parse(parser, token, curToken, &context);
			isRunning = treatError(context, scanner);
			strBufIni += stringLength + 1;

		} else if(token == SECTION) {

			int stringLength = scanner.cur - scanner.top;
			strncpy(&stringBuffer[strBufIni], scanner.top, stringLength);
			stringBuffer[strBufIni + stringLength] = '\0';

			Token* curToken = new Token{token, &stringBuffer[strBufIni]};

			int res = Parse(parser, token, curToken, &context);
			isRunning = treatError(context, scanner);
			strBufIni += stringLength + 1;

		} else if(token == IDENTIF) {
			int stringLength = scanner.cur - scanner.top;
			strncpy(&stringBuffer[strBufIni], scanner.top, stringLength);
			stringBuffer[strBufIni + stringLength] = '\0';

			Token* curToken = new Token{token, &stringBuffer[strBufIni]};

			int res = Parse(parser, token, curToken, &context);
			isRunning = treatError(context, scanner);
			strBufIni += stringLength + 1;

		} else if(token == STRING_START || token == STRING_LITERAL ||
				  token == STRING_END || token == VAR_START ||
				  token == VAR_NAME || token == VAR_END) {

			int stringLength = scanner.cur - scanner.top;
			strncpy(&stringBuffer[strBufIni], scanner.top, stringLength);
			stringBuffer[strBufIni + stringLength] = '\0';

			Token* curToken = new Token{token, &stringBuffer[strBufIni]};
			int res = Parse(parser, token, curToken, &context);
			isRunning = treatError(context, scanner);
			strBufIni += stringLength + 1;

		} else if (token == LEX_ERROR) {
			context.success = 0;
			isRunning = treatError(context, scanner);
		} else if(token != END_TOKEN) {
			Parse(parser, token, nullptr, &context);
			isRunning = treatError(context, scanner);
		}

        // Execute Parse for the last time
        if(token == END_TOKEN) {
            Parse(parser, END_TOKEN, nullptr, &context);
            Parse(parser, 0, nullptr, &context);
            break;
        }
    }

    /* Deallocate parser */
    ParseFree(parser, free);

	if(!context.success || context.astSections == nullptr) {
		return makeErrorResult(IceErrorCode::SyntaxError, syntaxErrorMessage(context), scanner.line);
	}

	if (!Ice::ConditionalResolver::resolveConditionals(context.astSections, context.astFactory, context.params.debugParser)) {
		return makeErrorResult(IceErrorCode::ConditionalError, "Conditional resolution failed");
	}

	if (!Ice::ReferenceResolver::resolveReferences(context.astSections, context.astFactory, context.params.debugParser)) {
		return makeErrorResult(IceErrorCode::ReferenceError, "Reference resolution failed");
	}

	/* Resolve string interpolation  */
	if (!Ice::Resolver::resolveInterpolations(context.astSections, context.params)) {
		return makeErrorResult(IceErrorCode::InterpolationError, "Interpolation resolution failed");
	}

	if (!Ice::MathResolver::resolveMathExpressions(context.astSections, context.astFactory, context.params.debugParser)) {
		return makeErrorResult(IceErrorCode::MathError, "Math resolution failed");
	}

	std::unique_ptr<IceDocument> document = Ice::DocumentBuilder::build(
		context.iceDeclaration,
		context.realmDeclaration,
		context.astSections
	);

	if(context.params.printConfig) {
		Ice::DebugParser::printIceDocument(*document);
	}

    return std::make_unique<IceParserResult>(std::move(document));
}

bool treatError(const ParserContext& context, Scanner scanner) {

    if(!context.success) {
        cout << "=> Syntax error on line: " << scanner.line << endl;
        cout << "expecting: ";
        int idx = 0;
        for (auto opt: context.options) {
            if(idx > 0) {
                cout << " OR ";
            }
            cout << "'" << opt << "'";
            idx++;
        }
        cout << endl;
        return 0;
    }
    return 1;
}
