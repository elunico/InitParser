//
// Created by Thomas Povinelli on 1/19/25.
//

#include "InitSection.h"

#include <__filesystem/path.h>
#include <iostream>
#include <numeric>
#include <concepts>
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
        return std::ranges::any_of(
            subsections,
            [&key] (std::pair<std::string, InitSection> const& p) {
                auto const& [name, section] = p;
                return section.hasEntryRecursive(key);
            }
        );
    }

    [[nodiscard]] std::pair<InitSection::ResolutionType, void *> InitSection::canResolveHelper(
        std::vector<std::string> const& path,
        int const                       n
    ) {
        if (n >= path.size()) {
            return std::make_pair(ResolutionType::NONE, nullptr);
        }
        if (n == path.size() - 1) {
            if (name == path[n]) {
                return std::make_pair(ResolutionType::SECTION, this);
            }
            for (auto& [name, entry]: entries) {
                if (name == path[n]) {
                    return std::make_pair(ResolutionType::ENTRY, &entry);
                }
            }
            return std::make_pair(ResolutionType::NONE, nullptr);
        }
        // the default section - parent to all subsections implicitly - is not named in a path
        // and so it is always acceptable to traverse down that section
        if ((name == path[n] && n != path.size() - 1) || isDefaultNamed()) {
            for (auto & [name, section]: subsections) {
                if (auto res = section.canResolveHelper(path, (isDefaultNamed()) ? n : (n + 1));
                    res.first != ResolutionType::NONE) {
                    return res;
                }
            }
            // finally if this is the last section, check for an entry
            return canResolveHelper(path, n + 1);
        }
        return std::make_pair(ResolutionType::NONE, nullptr);
    }

    bool InitSection::isDefaultNamed() const {
        return name == DEFAULT_NAME;
    }

    [[nodiscard]] InitSection::ResolutionType InitSection::canResolve(std::string const& path) const {
        auto p = path_to_components(path);
        // method is const
        return const_cast<InitSection*>(this)->canResolveHelper(p, 0).first;
    }

    [[nodiscard]] InitEntry& InitSection::getEntryExact(std::string const& path) {
        auto p = path_to_components(path);
        switch (auto [kind, ptr] = canResolveHelper(p, 0); kind) {
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
        switch (auto [kind, ptr] = canResolveHelper(p, 0); kind) {
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

    bool InitSection::updateEntryRecursive(std::string const& path, std::string const& value) {
        return updateEntryRecursive(path_to_components(path), value);
    }

    bool InitSection::updateEntryRecursive(
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
                   subsections, 0uz, [] (auto acc, auto s) { return acc + s.second.sizeRecursive(); }
               );
        // std::accumulate(
        // std::begin(subsections), std::end(subsections), 0, [] (auto acc, auto s) {
        // return acc + s.second.size();
        // }
        // );
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
        for (auto const& entry: entries) {
            os << spacing << InitFile::escaped(entry.first) << "=" << InitFile::escaped(entry.second.value()) <<
                    std::endl;
        }
        for (auto const& [name, section]: subsections) {
            section.print(os, level + 1);
        }
    }
} // namespace Init
