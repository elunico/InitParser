//
// Created by Thomas Povinelli on 1/19/25.
//

#ifndef INITFILE_H
#define INITFILE_H
#include <iostream>

#include "InitSection.h"

namespace Init {
    class InitFile {
        InitSection defaultSection{InitSection::DEFAULT_NAME};

        static void pop_section(std::vector<InitSection *>& secstack);

    public:
        static bool is_escape_char(char c);

        static std::vector<char> const ESCAPE_CHARS;

        static InitFile parse(const std::string& fileName);

        InitSection& sections() noexcept;

        InitSection const& sections() const noexcept;

        static std::string escaped(std::string const& key);

        void print(std::ostream& os = std::cout) const;
    };
} // namespace Init

#endif // INITFILE_H
