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

        constexpr static const std::string DEFAULT_NAME = "<default>";

    private:
        std::string                                  name{DEFAULT_NAME};
        std::unordered_map<std::string, InitEntry>   entries;
        std::unordered_map<std::string, InitSection> subsections;

        static std::vector<std::string> path_to_components(std::string const& path);

        bool getPathImpl(std::string const& key, std::vector<std::string>& path) const;

        [[nodiscard]] ResolutionType canResolveHelper(std::vector<std::string> const& path, int n) const;

        bool isDefaultNamed() const;

    public:
        friend class InitFile;

        using InitSectionName = std::string;

        InitSection();

        explicit InitSection(std::string name) noexcept;

        void addEntry(InitEntry const& entry);

        /// this function returns the names of nested sections required to traverse to get the `key`
        /// if the key exists in the default section an empty vector is returned
        /// if std::nullopt is returned the key does not exist in the file
        [[nodiscard]] std::optional<std::vector<InitSectionName> > getPathToEntry(std::string const& key) const;

        // [[nodiscard]] std::optional<std::vector<InitSectionName>> getPathToSubsection(std::string const& name) const;

        [[nodiscard]] ResolutionType canResolve(std::string const& path) const;

        [[nodiscard]] bool hasEntry(std::string const& name) const;

        [[nodiscard]] bool hasEntryRecursive(std::string const& key) const;

        [[nodiscard]] std::optional<std::string> getEntry(std::string const& key) const;

        [[nodiscard]] std::vector<InitEntry> getAllEntries() const;

        [[nodiscard]] std::optional<std::string> getEntryRecursive(std::string const& key) const;

        [[nodiscard]] std::vector<InitEntry> getAllEntriesRecursive() const;

        bool updateEntry(std::string const& key, std::string const& value);

        bool updateEntryRecursive(std::string const& path, std::string const& value);

        bool updateEntryRecursive(
            std::vector<std::string> const& section_path,
            std::string const&              value
        );

        [[nodiscard]] std::size_t size() const noexcept;

        [[nodiscard]] std::size_t sizeRecursive() const noexcept;

        [[nodiscard]] InitSection const& getSubsection(std::string const& key) const;

        [[nodiscard]] InitSection& getSubsection(std::string const& key);

        void print(std::ostream& os = std::cout, int level = 1) const;
    };
} // namespace Init

#endif // INITSECTION_H
