//
// Created by Thomas Povinelli on 1/19/25.
//

#include "InitEntry.h"

#include <utility>


namespace Init {
    InitEntry::InitEntry() = default;

    InitEntry::InitEntry(std::string key, std::string value) : m_key(std::move(key)), m_value(std::move(value)) {}

    [[nodiscard]] std::string const& InitEntry::key() const {
        return m_key;
    }

    [[nodiscard]] std::string& InitEntry::key() {
        return m_key;
    }

    [[nodiscard]] std::string const& InitEntry::value() const {
        return m_value;
    }

    [[nodiscard]] std::string& InitEntry::value() {
        return m_value;
    }

    std::string InitEntry::toString() const {
        return m_key + "=" + m_value;
    }
}
