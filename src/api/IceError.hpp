#ifndef ICEERROR_HPP
#define ICEERROR_HPP

#include <string>
#include <utility>

enum class IceErrorCode {
	SyntaxError,
	ConditionalError,
	ReferenceError,
	InterpolationError,
	MathError,
	ReadError,
	InternalError
};

class IceError {
private:
	IceErrorCode codeValue;
	std::string messageValue;
	int lineValue;

public:
	IceError(IceErrorCode codeArg, std::string messageArg, int lineArg = 0)
		: codeValue(codeArg),
		  messageValue(std::move(messageArg)),
		  lineValue(lineArg) {}

	IceErrorCode code() const {
		return codeValue;
	}

	const std::string& message() const {
		return messageValue;
	}

	int line() const {
		return lineValue;
	}
};

#endif // ICEERROR_HPP
