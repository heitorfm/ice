#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

class Property {
public:
	typedef std::vector<Property> Array;
	typedef std::map<std::string, Property> Object;

	struct FunctionCall {
		std::string namespaceName;
		std::string functionName;
		Array args;
	};

	struct Pipeline {
		std::shared_ptr<Property> input;
		std::vector<FunctionCall> steps;
	};

private:
	typedef std::variant<
		std::nullptr_t,
		int,
		double,
		bool,
		std::string,
		Array,
		Object,
		FunctionCall,
		Pipeline
	> PropertyContent;

	PropertyContent content;

public:
	Property()
		: content(nullptr) {}

	explicit Property(int value)
		: content(value) {}

	explicit Property(double value)
		: content(value) {}

	explicit Property(bool value)
		: content(value) {}

	explicit Property(std::string value)
		: content(std::move(value)) {}

	explicit Property(const char* value)
		: content(std::string(value)) {}

	explicit Property(Array value)
		: content(std::move(value)) {}

	explicit Property(Object value)
		: content(std::move(value)) {}

	explicit Property(FunctionCall value)
		: content(std::move(value)) {}

	explicit Property(Pipeline value)
		: content(std::move(value)) {}

	bool isNull() const {
		return std::holds_alternative<std::nullptr_t>(content);
	}

	bool isInt() const {
		return std::holds_alternative<int>(content);
	}

	bool isDouble() const {
		return std::holds_alternative<double>(content);
	}

	bool isBool() const {
		return std::holds_alternative<bool>(content);
	}

	bool isString() const {
		return std::holds_alternative<std::string>(content);
	}

	bool isArray() const {
		return std::holds_alternative<Array>(content);
	}

	bool isObject() const {
		return std::holds_alternative<Object>(content);
	}

	bool isFunctionCall() const {
		return std::holds_alternative<FunctionCall>(content);
	}

	bool isPipeline() const {
		return std::holds_alternative<Pipeline>(content);
	}

	int asInt() const {
		return std::get<int>(content);
	}

	double asDouble() const {
		return std::get<double>(content);
	}

	bool asBool() const {
		return std::get<bool>(content);
	}

	const std::string& asString() const {
		return std::get<std::string>(content);
	}

	const Array& asArray() const {
		return std::get<Array>(content);
	}

	const Object& asObject() const {
		return std::get<Object>(content);
	}

	const FunctionCall& asFunctionCall() const {
		return std::get<FunctionCall>(content);
	}

	const Pipeline& asPipeline() const {
		return std::get<Pipeline>(content);
	}

	const Property* getProperty(const std::string& name) const {
		if (!isObject()) {
			return nullptr;
		}

		const Object& object = asObject();
		auto it = object.find(name);
		if (it == object.end()) {
			return nullptr;
		}

		return &it->second;
	}
};

#endif // PROPERTY_HPP
