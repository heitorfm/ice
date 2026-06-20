#ifndef ICEPARSERRESULT_HPP
#define ICEPARSERRESULT_HPP

#include <memory>
#include <utility>
#include <vector>
#include "api/IceDocument.hpp"
#include "api/IceError.hpp"

class IceParserResult {
private:
	std::unique_ptr<IceDocument> documentValue;
	std::vector<IceError> errors;

public:
	IceParserResult() = default;

	explicit IceParserResult(std::unique_ptr<IceDocument> documentArg)
		: documentValue(std::move(documentArg)) {}

	IceParserResult(const IceParserResult&) = delete;
	IceParserResult& operator=(const IceParserResult&) = delete;

	IceParserResult(IceParserResult&&) = default;
	IceParserResult& operator=(IceParserResult&&) = default;

	void setDocument(std::unique_ptr<IceDocument> documentArg) {
		documentValue = std::move(documentArg);
	}

	IceDocument* document() {
		return documentValue.get();
	}

	const IceDocument* document() const {
		return documentValue.get();
	}

	std::unique_ptr<IceDocument> releaseDocument() {
		return std::move(documentValue);
	}

	void addError(IceError error) {
		errors.push_back(std::move(error));
	}

	bool hasErrors() const {
		return !errors.empty();
	}

	bool isSuccess() const {
		return documentValue != nullptr && errors.empty();
	}

	const std::vector<IceError>& getErrors() const {
		return errors;
	}
};

#endif // ICEPARSERRESULT_HPP
