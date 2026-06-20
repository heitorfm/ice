#ifndef ASTFACTORY_HPP
#define ASTFACTORY_HPP

#include <memory>
#include <utility>
#include <vector>

/**
 * Keeps parser semantic values as raw pointers while centralizing ownership.
 */
class AstFactory {
private:
	struct Allocation {
		virtual ~Allocation() {}
	};

	template<typename T>
	struct TypedAllocation : public Allocation {
		std::unique_ptr<T> object;

		explicit TypedAllocation(std::unique_ptr<T> objectArg)
			: object(std::move(objectArg)) {}
	};

	std::vector<std::unique_ptr<Allocation>> allocations;

public:
	AstFactory() = default;
	AstFactory(const AstFactory&) = delete;
	AstFactory& operator=(const AstFactory&) = delete;

	template<typename T, typename... Args>
	T* make(Args&&... args) {
		std::unique_ptr<T> object = std::make_unique<T>(std::forward<Args>(args)...);
		T* rawObject = object.get();
		allocations.emplace_back(std::make_unique<TypedAllocation<T>>(std::move(object)));
		return rawObject;
	}

	void clean() {
		allocations.clear();
	}
};

#endif // ASTFACTORY_HPP
