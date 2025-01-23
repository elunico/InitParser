//
// Created by Thomas Povinelli on 1/22/25.
//

#include "InitException.h"

namespace Init {
    InitException::InitException(std::string const& w) : message(w) {}


    char const *InitException::what() const noexcept {
        return message.c_str();
    }

    InitException::~InitException() = default;
} // Init
