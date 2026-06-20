#ifndef ICE_TESTS_DOCTEST_HPP
#define ICE_TESTS_DOCTEST_HPP

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace doctest {

struct TestCase {
	std::string name;
	void (*func)();
};

struct TestFailure : public std::exception {};

inline std::vector<TestCase>& registry() {
	static std::vector<TestCase> tests;
	return tests;
}

inline int& failureCount() {
	static int count = 0;
	return count;
}

struct Registrar {
	Registrar(const char* name, void (*func)()) {
		registry().push_back(TestCase{name, func});
	}
};

inline void check(bool passed, const char* expression, const char* file, int line, bool required) {
	if (passed) {
		return;
	}

	++failureCount();
	std::cerr << file << ":" << line << ": check failed: " << expression << std::endl;

	if (required) {
		throw TestFailure();
	}
}

inline int runTests() {
	for (const TestCase& test: registry()) {
		const int failuresBefore = failureCount();

		try {
			test.func();
		} catch (const TestFailure&) {
		} catch (const std::exception& error) {
			++failureCount();
			std::cerr << "Unhandled exception in '" << test.name << "': " << error.what() << std::endl;
		} catch (...) {
			++failureCount();
			std::cerr << "Unhandled unknown exception in '" << test.name << "'" << std::endl;
		}

		if (failureCount() != failuresBefore) {
			std::cerr << "[failed] " << test.name << std::endl;
		} else {
			std::cout << "[passed] " << test.name << std::endl;
		}
	}

	std::cout << registry().size() << " test case(s), " << failureCount() << " failure(s)" << std::endl;
	return failureCount() == 0 ? 0 : 1;
}

} // namespace doctest

#define DOCTEST_JOIN_IMPL(left, right) left##right
#define DOCTEST_JOIN(left, right) DOCTEST_JOIN_IMPL(left, right)

#define TEST_CASE(name) \
	static void DOCTEST_JOIN(doctest_test_, __LINE__)(); \
	static doctest::Registrar DOCTEST_JOIN(doctest_registrar_, __LINE__)(name, DOCTEST_JOIN(doctest_test_, __LINE__)); \
	static void DOCTEST_JOIN(doctest_test_, __LINE__)()

#define CHECK(expression) doctest::check(static_cast<bool>(expression), #expression, __FILE__, __LINE__, false)
#define REQUIRE(expression) doctest::check(static_cast<bool>(expression), #expression, __FILE__, __LINE__, true)
#define CHECK_FALSE(expression) CHECK(!(expression))
#define REQUIRE_FALSE(expression) REQUIRE(!(expression))

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
int main() {
	return doctest::runTests();
}
#endif

#endif // ICE_TESTS_DOCTEST_HPP
