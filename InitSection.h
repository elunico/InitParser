//
// Created by Thomas Povinelli on 1/19/25.
//

#ifndef INITSECTION_H
#define INITSECTION_H
#include <iostream>
#include <unordered_map>
#include <vector>

#include "InitEntry.h"

namespace Init {
    class InitSection {
    public:
        enum class ResolutionType { NONE, ENTRY, SECTION };

        constexpr static std::string DEFAULT_NAME = "<default>";

    private:
        std::string                                  name{DEFAULT_NAME};
        std::unordered_map<std::string, InitEntry>   entries;
        std::unordered_map<std::string, InitSection> subsections;

        static std::vector<std::string> path_to_components(std::string const& path);

        bool getPathImpl(std::string const& key, std::vector<std::string>& path) const;

        [[nodiscard]] std::pair<ResolutionType, void *> canResolveHelper(
            std::vector<std::string> const& path,
            int                             n
        );

        template <class It>
        [[nodiscard]] std::pair<ResolutionType, void *> canResolveHelper(
            It start,
            It end
        ) {
            if (start >= end) {
                return std::make_pair(ResolutionType::NONE, nullptr);
            }
            if (start == (end - 1)) {
                if (name == *start) {
                    return std::make_pair(ResolutionType::SECTION, this);
                }
                for (auto& [name, entry]: entries) {
                    if (name == *start) {
                        return std::make_pair(ResolutionType::ENTRY, &entry);
                    }
                }
                return std::make_pair(ResolutionType::NONE, nullptr);
            }
            // the default section - parent to all subsections implicitly - is not named in a path
            // and so it is always acceptable to traverse down that section
            if ((name == *start && start != end - 1) || isDefaultNamed()) {
                for (auto& [name, section]: subsections) {
                    if (auto res = section.canResolveHelper(isDefaultNamed() ? start : start + 1, end);
                        res.first != ResolutionType::NONE) {
                        return res;
                    }
                }
                // finally if this is the last section, check for an entry
                return canResolveHelper(start + 1, end);
            }
            return std::make_pair(ResolutionType::NONE, nullptr);
        }

        [[nodiscard]] bool isDefaultNamed() const;

    public:
        friend class InitFile;

        using InitSectionName = std::string;

        InitSection();

        explicit InitSection(std::string name) noexcept;

        void createEntry(std::string const& key, std::string const& value);

        void addEntry(InitEntry const& entry);

        void addEntry(InitEntry&& entry);

        /// this function returns the names of nested sections required to traverse to get the `key`
        /// if the key exists in the default section an empty vector is returned
        /// if std::nullopt is returned the key does not exist in the file
        [[nodiscard]] std::optional<std::vector<InitSectionName> > getPathToEntry(std::string const& key) const;

        // [[nodiscard]] std::optional<std::vector<InitSectionName>> getPathToSubsection(std::string const& name) const;

        [[nodiscard]] ResolutionType canResolve(std::string const& path) const;

        [[nodiscard]] ResolutionType canResolve(std::vector<std::string> const& path) const;

        [[nodiscard]] InitEntry const& getEntryExact(std::string const& path) const;

        InitEntry& getEntryExact(std::string const& path);

        [[nodiscard]] InitEntry const& getEntryExact(std::vector<std::string> const& path) const;

        InitEntry& getEntryExact(std::vector<std::string> const& path);

        [[nodiscard]] InitSection const& getSectionExact(std::string const& path) const;

        InitSection& getSectionExact(std::string const& path);

        [[nodiscard]] InitSection const& getSectionExact(std::vector<std::string> const& path) const;

        InitSection& getSectionExact(std::vector<std::string> const& path);

        InitSection& createSubsection(std::string const& name);

        bool removeSubsection(std::string const& name);

        bool removeEntry(std::string const& key);

        [[nodiscard]] bool hasEntry(std::string const& name) const;

        [[nodiscard]] bool hasEntryExact(std::string const& path) const;

        [[nodiscard]] bool hasEntryExact(std::vector<std::string> const& path) const;

        [[nodiscard]] std::optional<std::string> getEntry(std::string const& key) const;

        [[nodiscard]] std::vector<InitEntry> getAllEntries() const;

        [[nodiscard]] std::vector<InitEntry> getAllEntriesRecursive() const;

        bool updateEntry(std::string const& key, std::string const& value);

        bool updateEntryExact(std::string const& path, std::string const& value);

        bool updateEntryExact(
            std::vector<std::string> const& section_path,
            std::string const&              value
        );

        [[nodiscard]] std::size_t size() const noexcept;

        [[nodiscard]] std::size_t sizeRecursive() const noexcept;

        [[nodiscard]] InitSection const& getSubsection(std::string const& key) const;

        [[nodiscard]] InitSection& getSubsection(std::string const& key);

        static void print_with_escapes(std::ostream& os, std::string const& s);

        template <typename Callable> requires std::is_invocable_v<Callable, InitEntry&>
        void breadth_first_visit(Callable l) {
            for (auto& [name, entry]: entries) {
                l(entry);
            }
            std::deque<InitSection> sections{};
            sections.insert(sections.end(), subsections.begin(), subsections.end());
            while (!sections.empty()) {
                auto& s = sections.front();
                sections.pop_front();
                for (auto& [name, entry]: s.entries) {
                    l(entry);
                }
                sections.insert(sections.end(), s.subsections.begin(), s.subsections.end());
            }
        }

        void print(std::ostream& os = std::cout, int level = 1) const;
    };
} // namespace Init

#endif // INITSECTION_H
