
#include "StrUtil.hpp"
#include <string.h>
#include <cstring>


using namespace::std;

/**
 * if we have a char * that is
 * "hello"
 * it will return it without surrounding quotes
 * hello
 * @param str
 * @return
 */
char *StrUtil::stripQuotes(char *str) {

    size_t len;
    char *str2;
    len = strnlen(str, 1024);

    str2 = strdup(str);
    str2++;
    str2[len-2] = '\0';

    return str2;
}

vector<string> StrUtil::split(const string& str, const string& delimiter) {
	vector<string> tokens;

	size_t start = 0;
	size_t end = str.find(delimiter);

	while (end != string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + delimiter.length();
		end = str.find(delimiter, start);
	}

	tokens.push_back(str.substr(start));

	return tokens;
}
