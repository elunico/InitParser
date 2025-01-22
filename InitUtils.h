//
// Created by Thomas Povinelli on 1/21/25.
//

#ifndef INITUTILS_H
#define INITUTILS_H
#include <ostream>

#include "InitSection.h"

std::ostream &operator<<(std::ostream &os,
                         Init::InitSection::ResolutionType const &name);

#endif //INITUTILS_H
