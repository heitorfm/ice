#ifndef ICEPARSERPARAMS_HPP
#define ICEPARSERPARAMS_HPP

enum class MissingVariableBehavior {
	Error,
	Blank,
	VarName
};

struct IceParserParams {
	bool printConfig = false;
	bool debugParser = false;
	bool info = true;
	MissingVariableBehavior missingVariableBehavior = MissingVariableBehavior::Error;
};

#endif // ICEPARSERPARAMS_HPP
