//
// Created by Thomas Povinelli on 1/19/25.
//

#include "InitSection.h"

#include <__filesystem/path.h>
#include <iostream>
#include <numeric>
#include <utility>
#include "InitEntry.h"

namespace Init {
    bool InitSection::getPathImpl(std::string const& key, std::vector<std::string>& path) const {
        if (entries.contains(key)) {
            // path.push_back(name);
            return true;
        }
        for (auto const& [name, section]: subsections) {
            if (section.getPathImpl(key, path)) {
                path.push_back(name);
                return true;
            }
        }
        return false;
    }

    InitSection::InitSection() = default;

    InitSection::InitSection(std::string name) noexcept : name(std::move(name)) {}

    void InitSection::addEntry(InitEntry const& entry) {
        entries[entry.key()] = entry;
    }

    [[nodiscard]] std::optional<std::vector<InitSection::InitSectionName> >
    InitSection::getPathToEntry(std::string const& key) const {
        std::vector<InitSectionName> path{};
        if (getPathImpl(key, path)) {
            std::ranges::reverse(path);
            return std::make_optional(path);
        }
        return std::nullopt;
    }

    [[nodiscard]] bool InitSection::hasEntry(std::string const& name) const {
        return entries.contains(name);
    }

    [[nodiscard]] bool InitSection::hasEntryRecursive(std::string const& key) const {
        if (entries.contains(key)) {
            return true;
        }
        return std::any_of(
            std::begin(subsections),
            std::end(subsections),
            [&key] (std::pair<std::string, InitSection> const& p) {
                auto const& [name, section] = p;
                return section.hasEntryRecursive(key);
            }
        );
    }

    [[nodiscard]] InitSection::ResolutionType InitSection::canResolveHelper(
        std::vector<std::string> const& path,
        int const                       n
    ) const {
        if (n >= path.size()) {
            return ResolutionType::NONE;
        }
        if (n == path.size() - 1) {
            if (name == path[n]) {
                return ResolutionType::SECTION;
            }
            for (auto const& [name, entry]: entries) {
                if (name == path[n]) {
                    return ResolutionType::ENTRY;
                }
            }
            return ResolutionType::NONE;
        }
        // the default section - parent to all subsections implicitly - is not named in a path
        // and so it is always acceptable to traverse down that section
        if ((name == path[n] && n != path.size() - 1) || isDefaultNamed()) {
            for (auto const& [name, section]: subsections) {
                if (auto r = section.canResolveHelper(path, (isDefaultNamed()) ? n : (n + 1));
                    r != ResolutionType::NONE) {
                    return r;
                }
            }
            // finally if this is the last section, check for an entry
            return canResolveHelper(path, n + 1);
        }
        return ResolutionType::NONE;
    }

    bool InitSection::isDefaultNamed() const {
        return name == DEFAULT_NAME;
    }

    [[nodiscard]] InitSection::ResolutionType InitSection::canResolve(std::string const& path) const {
        auto p = path_to_components(path);
        return canResolveHelper(p, 0);
    }


    [[nodiscard]] std::optional<std::string> InitSection::getEntry(std::string const& key) const {
        if (entries.contains(key)) {
            return std::make_optional(entries.at(key).value());
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<InitEntry> InitSection::getAllEntries() const {
        std::vector<InitEntry> s{};
        s.reserve(entries.size());
        for (auto const& [name, entry]: entries) {
            s.push_back(entry);
        }
        return s;
    }

    [[nodiscard]] std::optional<std::string> InitSection::getEntryRecursive(std::string const& key) const {
        if (entries.contains(key)) {
            return entries.at(key).value();
        }
        for (auto const& [name, subsec]: subsections) {
            if (subsec.hasEntryRecursive(key)) {
                return subsec.getEntryRecursive(key);
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<InitEntry> InitSection::getAllEntriesRecursive() const {
        std::vector<InitEntry> s{};
        s.reserve(entries.size());
        for (auto const& [name, entry]: entries) {
            s.push_back(entry);
        }
        for (auto const& [name, subsec]: subsections) {
            auto const& entries = subsec.getAllEntriesRecursive();
            s.insert(std::end(s), std::begin(entries), std::end(entries));
        }
        return s;
    }


    bool InitSection::updateEntry(std::string const& key, std::string const& value) {
        if (entries.contains(key)) {
            std::cout << key << std::endl;
            entries.at(key).value() = value;
            return true;
        }
        return false;
    }

    std::vector<std::string> InitSection::path_to_components(std::string const& path) {
        std::vector<std::string> result;
        for (int i = 0; i < path.size(); i++) {
            std::string comp{};
            while (path[i] != '/' && i < path.size()) {
                if (path[i] == '\\') {
                    auto const n = path[i + 1];
                    if (n != '/' && n != '\\') {
                        throw std::invalid_argument("Invalid esacpe char");
                    }
                    comp += path[i + 1];
                    i = i + 2;
                    continue;
                }
                comp += path[i];
                i++;
            }
            result.push_back(comp);
        }
        return result;
    }

    bool InitSection::updateEntryRecursive(std::string const& path, std::string const& key, std::string const& value) {
        std::vector<std::string> result = path_to_components(path);
        return updateEntryRecursive(result, key, value);
    }

    bool InitSection::updateEntryRecursive(
        std::vector<std::string> const& section_path,
        std::string const&              key,
        std::string const&              value
    ) {
        InitSection *target = this;
        for (int i = 0; i < section_path.size(); i++) {
            auto const& path = section_path[i];
            target           = &target->getSubsection(path);
        }
        return target->updateEntry(key, value);
    }

    [[nodiscard]] std::size_t InitSection::size() const noexcept {
        return entries.size();
    }

    [[nodiscard]] std::size_t InitSection::sizeRecursive() const noexcept {
        return entries.size() +
               std::accumulate(
                   std::begin(subsections), std::end(subsections), 0, [] (auto acc, auto s) {
                       return acc + s.second.size();
                   }
               );
    }

    [[nodiscard]] InitSection const& InitSection::getSubsection(std::string const& key) const {
        return subsections.at(key);
    }

    [[nodiscard]] InitSection& InitSection::getSubsection(std::string const& key) {
        return subsections.at(key);
    }

    void InitSection::print(std::ostream& os, int level) const {
        std::string spacing(level * 4, ' ');
        os << spacing << std::string(level, '[') << name << std::string(level, ']') << std::endl;
        for (const auto& entry: entries) {
            os << spacing << entry.first << "=" << entry.second.value() << std::endl;
        }
        for (const auto& [name, section]: subsections) {
            section.print(os, level + 1);
        }
    }
} // namespace Init
