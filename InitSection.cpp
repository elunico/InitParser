//
// Created by Thomas Povinelli on 1/19/25.
//

#include "InitSection.h"

#include <__filesystem/path.h>
#include <iostream>
#include <numeric>
#include <concepts>
#include <algorithm>
#include <utility>
#include "InitEntry.h"
#include "InitException.h"
#include "InitFile.h"

namespace Init {
    namespace Private {
        template <class L, class K, class V, class R>
        concept map_operator = requires (L op, std::pair<K, V> pair, R result) {
            { op(result, pair) } -> std::same_as<R>;
        };

        template <typename R, typename K, typename V, map_operator<K, V, R> Lambda>
        R accumulate(std::unordered_map<K, V> const& vec, R init, Lambda operation) {
            return std::accumulate(std::begin(vec), std::end(vec), init, operation);
        }
    }

    bool InitSection::getPathImpl(std::string const& key, std::vector<std::string>& path) const {
        if (entries.contains(key)) {
            // include key in path so it can be used in canResolve & updateEntryRecursive
            path.push_back(key);
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

    void InitSection::createEntry(std::string const& key, std::string const& value) {
        addEntry(InitEntry(key, value));
    }

    void InitSection::addEntry(InitEntry const& entry) {
        entries[entry.key()]          = entry;
        entries[entry.key()].m_parent = this;
    }

    void InitSection::addEntry(InitEntry&& entry) {
        entry.m_parent       = this;
        entries[entry.key()] = std::move(entry);
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

    bool InitSection::hasEntryExact(std::string const& path) const {
        return canResolve(path) == ResolutionType::ENTRY;
    }

    bool InitSection::hasEntryExact(std::vector<std::string> const& path) const {
        return canResolve(path) == ResolutionType::ENTRY;
    }

    bool InitSection::isDefaultNamed() const {
        return name == DEFAULT_NAME;
    }

    [[nodiscard]] InitSection::ResolutionType InitSection::canResolve(std::string const& path) const {
        auto p = path_to_components(path);
        // method is const
        return canResolve(p);
    }

    [[nodiscard]] InitSection::ResolutionType InitSection::canResolve(std::vector<std::string> const& path) const {
        return const_cast<InitSection *>(this)->canResolveHelper(std::begin(path), std::end(path)).first;
    }

    [[nodiscard]] InitEntry& InitSection::getEntryExact(std::string const& path) {
        auto p = path_to_components(path);
        return getEntryExact(p);
    }

    InitEntry const& InitSection::getEntryExact(std::vector<std::string> const& path) const {
        return const_cast<InitSection *>(this)->getEntryExact(path);
    }

    InitEntry& InitSection::getEntryExact(std::vector<std::string> const& path) {
        switch (auto [kind, ptr] = canResolveHelper(std::begin(path), std::end(path)); kind) {
            case ResolutionType::NONE:
                throw MissingEntry("InitSection::getEntryExact: no such entry");
            case ResolutionType::SECTION:
                throw InitException("InitSection::getEntryExact: can't get section ");
            case ResolutionType::ENTRY:
                return *static_cast<InitEntry *>(ptr);
            default:
                throw std::runtime_error("InitSection::getEntryExact: unknown branch");
        }
    }

    [[nodiscard]] InitSection& InitSection::getSectionExact(std::string const& path) {
        auto p = path_to_components(path);
        return getSectionExact(p);
    }

    InitSection const& InitSection::getSectionExact(std::vector<std::string> const& path) const {
        return const_cast<InitSection *>(this)->getSectionExact(path);
    }

    InitSection& InitSection::getSectionExact(std::vector<std::string> const& path) {
        switch (auto [kind, ptr] = canResolveHelper(std::begin(path), std::end(path)); kind) {
            case ResolutionType::NONE:
                throw MissingEntry("InitSection::getSectionExact: no such section");
            case ResolutionType::SECTION:
                return *static_cast<InitSection *>(ptr);
            case ResolutionType::ENTRY:
                throw InitException("InitSection::getSectionExact: can't get entry ");
            default:
                throw std::runtime_error("InitSection::getSectionExact: unknown branch");
        }
    }

    InitSection& InitSection::createSubsection(std::string const& name) {
        subsections[name] = InitSection{name};
        return subsections[name];
    }

    bool InitSection::removeSubsection(std::string const& name) {
        if (subsections.contains(name)) {
            subsections.erase(name);
            return true;
        }
        return false;
    }

    bool InitSection::removeEntry(std::string const& key) {
        if (entries.contains(key)) {
            entries.erase(key);
            return true;
        }
        return false;
    }

    [[nodiscard]] InitEntry const& InitSection::getEntryExact(std::string const& path) const {
        return const_cast<InitSection *>(this)->getEntryExact(path);
    }

    [[nodiscard]] InitSection const& InitSection::getSectionExact(std::string const& path) const {
        return const_cast<InitSection *>(this)->getSectionExact(path);
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
                        throw std::invalid_argument("Invalid escape char");
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

    bool InitSection::updateEntryExact(std::string const& path, std::string const& value) {
        return updateEntryExact(path_to_components(path), value);
    }

    bool InitSection::updateEntryExact(
        std::vector<std::string> const& section_path,
        std::string const&              value
    ) {
        auto target = this;
        // navigate through all subsections of the past, except the last component
        // which is the name of the entry to update
        for (int i = 0; i < section_path.size() - 1; i++) {
            auto const& path = section_path[i];
            target           = &target->getSubsection(path);
        }
        return target->updateEntry(section_path.back(), value);
    }

    [[nodiscard]] std::size_t InitSection::size() const noexcept {
        return entries.size();
    }

    [[nodiscard]] std::size_t InitSection::sizeRecursive() const noexcept {
        using Private::accumulate;
        return entries.size() + accumulate(
                   subsections, 0uz, [] (auto acc, auto const& s) { return acc + s.second.sizeRecursive(); }
               );
    }

    [[nodiscard]] InitSection const& InitSection::getSubsection(std::string const& key) const {
        return subsections.at(key);
    }

    [[nodiscard]] InitSection& InitSection::getSubsection(std::string const& key) {
        return subsections.at(key);
    }

    void InitSection::print_with_escapes(std::ostream &os, std::string const& s) {
        for (auto const& c: s) {
            if (InitFile::is_escape_char(c)) {
                os << '\\';
            }
            os << c;
        }
    }

    void InitSection::print(std::ostream& os, int level) const {
        std::string spacing(level * 4, ' ');
        os << spacing << std::string(level, '[') << name << std::string(level, ']') << std::endl;
        for (auto const& [name, entry]: entries) {
            os << spacing;
            print_with_escapes(os, entry.key());
            os << "=";
            print_with_escapes(os, entry.value());
            os << std::endl;
        }
        for (auto const& [name, section]: subsections) {
            section.print(os, level + 1);
        }
    }
} // namespace Init
