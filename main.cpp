#include <fstream>
#include <iostream>
#include <sstream>

#include "InitException.h"
#include "InitFile.h"
#include "InitUtils.h"

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

void old_test(Init::InitFile f) {
    auto path = f.sections().getPathToEntry("srk1");

    std::cout << path << std::endl;

    auto value = f.sections().getEntryRecursive("srk1");

    std::cout << value << std::endl;

    std::cout << f.sections().hasEntry("srk1") << std::endl;
    std::cout << f.sections().hasEntryRecursive("srk1") << std::endl;

    f.sections().updateEntryRecursive(path.value(), "anewtestingvalue");

    auto e = f.sections().getEntryRecursive("srk1");
    std::cout << e.value() << std::endl;

    std::ofstream out{};
    out.open("new_test.init");
    f.print(out);
    out.close();

    std::stringstream ss;
    f.print(ss);
    std::cout << ss.str() << std::endl;
}


int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }
    try {
        auto f = Init::InitFile::parse(argv[1]);
        f.print();
    } catch (Init::InitException const& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }

    // auto yes_entry        = f.sections().canResolve("Section1/section1-3/srk1");
    // auto yes_entry_middle = f.sections().canResolve("Section1/sk1");
    // auto yes_section      = f.sections().canResolve("Section1/section1-3");
    // auto no1              = f.sections().canResolve("Section1/section1-3/doesnotexist/srk1");
    // auto no2              = f.sections().canResolve("fails/section1-3/doesnotexist/srk1");
    // auto default_entry    = f.sections().canResolve("k1");
    // auto default_no       = f.sections().canResolve("missingkey");
    //
    // std::cout << "yes_entry: " << yes_entry << std::endl;
    // std::cout << "yes_entry_middle: " << yes_entry_middle << std::endl;
    // std::cout << "yes_section: " << yes_section << std::endl;
    // std::cout << "no1: " << no1 << std::endl;
    // std::cout << "no2: " << no2 << std::endl;
    // std::cout << "default_entry: " << default_entry << std::endl;
    // std::cout << "default_no: " << default_no << std::endl;
    //
    // f.sections().updateEntryRecursive("Section1", "sk1", "this is my value");


    return 0;
}
