//
// Created by Thomas Povinelli on 1/21/25.
//

#include "InitUtils.h"

std::ostream& operator<<(
    std::ostream&                            os,
    Init::InitSection::ResolutionType const& name
) {
    switch (name) {
        case Init::InitSection::ResolutionType::ENTRY:
            os << "ENTRY";
            break;
        case Init::InitSection::ResolutionType::SECTION:
            os << "SECTION";
            break;
        case Init::InitSection::ResolutionType::NONE:
            os << "NONE";
            break;
        default:
            os << "UNKNOWN" << " " << (int) name;
            break;
    }
    return os;
}
