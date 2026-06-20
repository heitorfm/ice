//
// Created by heitor on 30/10/23.
//

#ifndef STRUTIL_H
#define STRUTIL_H

#include <string>
#include <vector>

class StrUtil {

public:
    static char* stripQuotes(char *str);
	static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
};


#endif //STRUTIL_H
