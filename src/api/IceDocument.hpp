#ifndef ICEDOCUMENT_HPP
#define ICEDOCUMENT_HPP

#include <map>
#include <string>
#include <utility>
#include <vector>
#include "api/Section.hpp"

class IceDocument {
private:
	std::string iceDeclaration;
	std::string realmDeclaration;
	std::vector<Section> sections;
	std::map<std::string, size_t> sectionIndex;

public:
	void setDeclarations(std::string iceDecl, std::string realmDecl) {
		iceDeclaration = std::move(iceDecl);
		realmDeclaration = std::move(realmDecl);
	}

	void addSection(Section section) {
		const std::string sectionName = section.name();
		sectionIndex[sectionName] = sections.size();
		sections.push_back(std::move(section));
	}

	const std::string& getIceDeclaration() const {
		return iceDeclaration;
	}

	const std::string& getRealmDeclaration() const {
		return realmDeclaration;
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

#endif // ICEDOCUMENT_HPP
