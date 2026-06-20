

#include "Nodes.hpp"
#include "Resolver.hpp"
#include <iostream>
#include <sstream>

using namespace std;


const char* getNodeTypeName(NODE_TYPE nt) {
	switch(nt) {
		case ICECONFIG: return "ICECONFIG";
		case NT_SECTION: return "NT_SECTION";
		case ASSIGNMENT: return "ASSIGNMENT";
		case VALUE: return "VALUE";
		case MATH_EXPR: return "MATH_EXPR";
		case FUNCTION_CALL_EXPR: return "FUNCTION_CALL_EXPR";
		case PIPELINE_EXPR: return "PIPELINE_EXPR";
		case REFERENCE_EXPR: return "REFERENCE_EXPR";
		case LITERAL_EXPR: return "LITERAL_EXPR";
		case ARRAY_EXPR: return "ARRAY_EXPR";
		case OBJECT_EXPR: return "OBJECT_EXPR";
		case IF_STMT: return "IF_STMT";
		case BOOLEAN_STMT: return "BOOLEAN_STMT";
		case NT_IDENTIF: return "NT_IDENTIF";
		default: return "<NOT-FOUND>";
	}
}
