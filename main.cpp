#include <fstream>
#include <iostream>

#include "InitFile.h"

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> const& v) {
    os << "[";
    for (int i = 0; i < v.size(); i++) {
        os << v[i];
        if (i != v.size() - 1) {
            os << ", ";
        }
    }
    os << "]";

    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& opt) {
    if (opt.has_value()) {
        os << "std::optional(" << opt.value() << ")";
    } else {
        os << "std::nullopt";
    }
    return os;
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    auto f = Init::InitFile::parse(argv[1]);

    auto path = f.sections().getPathToEntry("srk1");

    std::cout << path << std::endl;

    auto value = f.sections().getEntryRecursive("srk1");

    std::cout << value << std::endl;

    std::cout << f.sections().hasEntry("srk1") << std::endl;
    std::cout << f.sections().hasEntryRecursive("srk1") << std::endl;

    f.sections().updateEntryRecursive(path.value(), "srk1", "anewtestingvalue");

    auto e = f.sections().getEntryRecursive("srk1");
    std::cout << e.value() << std::endl;

    std::ofstream out{};
    out.open("new_test.init");
    f.print(out);
    out.close();


    return 0;
}
