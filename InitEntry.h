//
// Created by Thomas Povinelli on 1/19/25.
//

#ifndef INITENTRY_H
#define INITENTRY_H
#include <string>

namespace Init {
    class InitEntry {
        std::string m_key;
        std::string m_value;

    public:
        InitEntry();

        InitEntry(std::string key, std::string value);

        InitEntry(const InitEntry& other) = default;

        InitEntry(InitEntry&& other) = default;

        InitEntry& operator=(const InitEntry& other) = default;

        InitEntry& operator=(InitEntry&& other) = default;

        [[nodiscard]] std::string const& key() const;

        [[nodiscard]] std::string& key();

        [[nodiscard]] std::string const& value() const;

        [[nodiscard]] std::string& value();

        [[nodiscard]] std::string toString() const;
    };
} // namespace Init


#endif // INITENTRY_H
