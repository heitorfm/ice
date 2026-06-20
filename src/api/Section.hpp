#ifndef SECTION_HPP
#define SECTION_HPP

#include <map>
#include <string>
#include <utility>
#include <vector>
#include "api/Property.hpp"

class Section {
private:
	std::string nameValue;
	std::map<std::string, Property> properties;
	std::vector<Section> sections;
	std::map<std::string, size_t> sectionIndex;

public:
	Section() = default;

	explicit Section(std::string nameArg)
		: nameValue(std::move(nameArg)) {}

	const std::string& name() const {
		return nameValue;
	}

	void setProperty(std::string name, Property property) {
		properties[std::move(name)] = std::move(property);
	}

	const Property* getProperty(const std::string& name) const {
		auto it = properties.find(name);
		if (it == properties.end()) {
			return nullptr;
		}

		return &it->second;
	}

	const std::map<std::string, Property>& getProperties() const {
		return properties;
	}

	void addSection(Section section) {
		const std::string sectionName = section.name();
		sectionIndex[sectionName] = sections.size();
		sections.push_back(std::move(section));
	}

	const std::vector<Section>& getSections() const {
		return sections;
	}

	const Section* getSection(const std::string& name) const {
		auto it = sectionIndex.find(name);
		if (it == sectionIndex.end()) {
			return nullptr;
		}

		return &sections[it->second];
	}
};

#endif // SECTION_HPP
