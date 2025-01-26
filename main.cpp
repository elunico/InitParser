#include <fstream>
#include <iostream>
#include <sstream>
#include <ostream>
#include <vector>

#include "InitFile.h"
#include "InitUtils.h"
#include "InitEntry.h"
#include "InitSection.h"


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

std::ostream& operator<<(std::ostream& os, Init::InitEntry const& entry) {
    os << entry.key() << "=" << entry.value();
    return os;
}

void old_test(Init::InitFile f) {
    auto path = f.sections().getPathToEntry("srk1");

    std::cout << path << std::endl;\

    auto p = f.sections().getPathToEntry("srk1");
    if (!p.has_value()) {
        throw std::runtime_error("No such entry");
    }
    auto value = f.sections().getEntryExact(*p);

    std::cout << value << std::endl;

    std::cout << f.sections().hasEntry("srk1") << std::endl;
    std::cout << (f.sections().canResolve("srk1") == Init::InitSection::ResolutionType::ENTRY) << std::endl;

    f.sections().updateEntryExact(path.value(), "anewtestingvalue");

    auto e = f.sections().getEntryExact("srk1");
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

    auto file = Init::InitFile::parse("../problem.init");

    auto const& f = file.sections().getEntryExact("Server-URL/routes/index/file");
    file.sections().updateEntryExact(
        "Server-URL/routes/index/file",
        "test.html"
    );
    auto o = file.sections().getPathToEntry("auth");
    file.sections().updateEntryExact(*o, "some-other-file.html");
    std::cout << f.value() << std::endl;

    auto& e = file.sections().getEntryExact("Server-URL/hostname");

    std::ofstream of;
    of.open("testout.init");

    file.print(of);


    return 0;
}
