//
// Created by Thomas Povinelli on 1/19/25.
//

#include "InitEntry.h"

#include <utility>


namespace Init {
    InitEntry::InitEntry() = default;

    InitEntry::InitEntry(std::string  key, std::string  value) : m_key(std::move(key)), m_value(std::move(value)) {}

    InitEntry::InitEntry(std::pair<std::string, std::string> const& p) : m_key(p.first),
                                                                         m_value(p.second) {}

    InitSection *InitEntry::parent() const {
        return m_parent;
    }

    [[nodiscard]] std::string const& InitEntry::key() const {
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
} // namespace Init
