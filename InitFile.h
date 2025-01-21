//
// Created by Thomas Povinelli on 1/19/25.
//

#ifndef INITFILE_H
#define INITFILE_H
#include "InitSection.h"

namespace Init {
    class InitFile {
        InitSection defaultSection{"<default>"};

        static void pop_section(std::vector<InitSection *>& secstack);

    public:
        static InitFile parse(const std::string& fileName);

        InitSection& sections() noexcept;

        InitSection const& sections() const noexcept;

        void print() const;
    };
} // Init

#endif //INITFILE_H
