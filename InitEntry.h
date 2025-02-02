//
// Created by Thomas Povinelli on 1/19/25.
//

#ifndef INITENTRY_H
#define INITENTRY_H
#include <string>

namespace Init {
    class InitSection;

    class InitEntry {
        friend class InitSection;

        std::string m_key;
        std::string m_value;

        InitSection *m_parent{};

    public:
        InitEntry();

        InitEntry(std::string key, std::string value);

        InitEntry(std::pair<std::string, std::string> const& p);

        InitEntry(InitEntry const& other) = default;

        InitEntry(InitEntry&& other) = default;

        InitEntry& operator=(InitEntry const& other) = default;

        InitEntry& operator=(InitEntry&& other) = default;

        [[nodiscard]] InitSection *parent() const;

        [[nodiscard]] std::string const& key() const;

        [[nodiscard]] std::string const& value() const;

        [[nodiscard]] std::string& value();

        [[nodiscard]] std::string toString() const;
    };
} // namespace Init


#endif // INITENTRY_H
