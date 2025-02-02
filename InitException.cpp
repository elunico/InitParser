//
// Created by Thomas Povinelli on 1/22/25.
//

#include "InitException.h"

#include <utility>

namespace Init {
    InitException::InitException(std::string  w) : message(std::move(w)) {}


    char const *InitException::what() const noexcept {
        return message.c_str();
    }

    InitException::~InitException() = default;
} // Init
