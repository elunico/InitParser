//
// Created by Thomas Povinelli on 1/19/25.
//

#include "InitFile.h"

#include <fstream>
#include <iostream>

namespace Init {
    namespace Util {
        static auto gulp_to_char(std::ifstream& file, char stop) -> std::string {
            std::string s;
            while (!file.eof()) {
                int const c = file.get();
                if (c == stop) { break; }
                s += static_cast<char>(c);
            }
            return s;
        }


        static std::string consume_key(std::ifstream& s, int& c) {
            std::string k;
            while (c != '=' && c != EOF) {
                k.push_back(c);
                c = s.get();
                if (c == '\n' || c == ';') {
                    throw std::invalid_argument("Key ended with no corresponding value");
                }
            }
            return k;
        }

        static std::string consume_value(std::ifstream& s, int& c) {
            std::string v;
            while (c != '\n' && c != EOF && c != ';') {
                v.push_back(c);
                c = s.get();
            }
            return v;
        }

        static std::string consume_section_name(std::ifstream& s, int& c) {
            std::string n;
            while (c != ']' && c != EOF) {
                n.push_back(c);
                c = s.get();
                if (c == '\n' || c == ';') {
                    throw std::invalid_argument("section name with unterminated square brackets");
                }
            }
            return n;
        }
    }

    void InitFile::pop_section(std::vector<InitSection *>& secstack) {
        secstack.pop_back();
    }

    InitSection& InitFile::sections() noexcept {
        return defaultSection;
    }

    InitSection const& InitFile::sections() const noexcept {
        return defaultSection;
    }

    InitFile InitFile::parse(const std::string& fileName) {
        InitFile file{};

        std::ifstream s;
        s.open(fileName);

        // 0 is default section 1 is first section with actual header
        int                        subsectionLevel = 0;
        std::vector<InitSection *> secstack{};
        secstack.push_back(&file.defaultSection);


        while (s.good() && !s.eof()) {
            int c = s.get();

            // allows us to use whitespace for nesting in the init file.
            // ignore line initial whitespace
            while (c == '\t' || c == ' ') {
                c = s.get();
            }

            // skip comments
            if (c == ';') {
                // skip comments
                Util::gulp_to_char(s, '\n');
                continue;
            }

            // skip blank lines
            if (c == '\n') {
                continue;
            }

            // deal with section headers (start/end)
            if (c == '[') {
                int prox = 0;
                while (s.good() && !s.eof() && c == '[') {
                    prox += 1;
                    c = s.get();
                    if (prox > (subsectionLevel + 1)) {
                        throw std::invalid_argument("Subsection level too deeply nested. Missing parent subsection");
                    }
                }
                // after the [
                if (c == '~') {
                    // pop while prox <= subsection level
                    while (prox <= subsectionLevel) {
                        pop_section(secstack);
                        subsectionLevel--;
                    }
                    // discard closing brackets
                    Util::gulp_to_char(s, '\n');
                } else {
                    std::string name = Util::consume_section_name(s, c);
                    // discard closing brackets
                    Util::gulp_to_char(s, '\n');

                    if (prox == (subsectionLevel)) {
                        // creating a new subsection at the same level requires 1 pop and 1 push
                        pop_section(secstack);
                        secstack.back()->subsections[name] = InitSection(name);
                        secstack.push_back(&secstack.back()->subsections[name]);
                    } else if (prox < subsectionLevel) {
                        // creating a higher subsection requires popping and decr subsection level until it is one less than prox then pushing
                        while (prox <= subsectionLevel) {
                            pop_section(secstack);
                            subsectionLevel--;
                        }
                        secstack.back()->subsections[name] = InitSection(name);
                        secstack.push_back(&secstack.back()->subsections[name]);
                    } else if (prox > subsectionLevel) {
                        // creating a deeper subsection requires only pushing
                        secstack.back()->subsections[name] = InitSection(name);
                        secstack.push_back(&secstack.back()->subsections[name]);
                    }
                    subsectionLevel = prox;
                }
                // section opener or closer is read so continue
                continue;
            }


            // read key and value
            std::string k = Util::consume_key(s, c);

            c = s.get(); // discard =

            std::string v = Util::consume_value(s, c);
            if (c != '\n') {
                Util::gulp_to_char(s, '\n');
            }

            secstack.back()->addEntry(InitEntry{k, v});
        }
        // do not put back the first section which is always a pointer to the existing default section
        // while (secstack.size() > 1) {
        // pop_section(file, secstack);
        // }
        return file;
    }

    void InitFile::print() const {
        for (auto const& [name, entry]: defaultSection.entries) {
            std::cout << "0E:" << entry.key() << "=" << entry.value() << std::endl;
        }
        for (auto const& [sec_name, section]: defaultSection.subsections) {
            section.print();
        }
    }
} // Init
