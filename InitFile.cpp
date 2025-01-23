//
// Created by Thomas Povinelli on 1/19/25.
//

#include "InitFile.h"
#include "InitException.h"

#include <fstream>
#include <iostream>

namespace Init {
    namespace Util {
        static int gulp_whitespace(std::ifstream& file) {
            int c{};
            while (!file.eof() && (c = file.get()) != EOF && (c == ' ' || c == '\t')) {}
            return c;
        }

        static void gulp_close_brackets(std::ifstream& file) {
            int c{};
            while (!file.eof() && (c = file.get()) != EOF && c == ']') {}
            file.unget();
        }

        static void gulp_to_char(std::ifstream& file, char stop) {
            int c;
            while (!file.eof() && (c = file.get()) != EOF && c != stop);
        }

        static int consume_escape(std::ifstream& file) {
            int const d = file.get();
            if (!InitFile::is_escape_char(d)) {
                throw ParseException("Invalid escape character");
            }
            return d;
        }


        static std::string consume_key(std::ifstream& s, int& c) {
            std::string k;
            while (c != '=' && c != EOF) {
                k.push_back(c);
                c = s.get();
                if (c == '\\') {
                    c = consume_escape(s);
                    // add the escaped char to the data
                    k.push_back(c);
                    // retrieve the next one
                    c = s.get();
                    // continue so it is pushed back
                    continue;
                }

                if (c == '\n' || c == ';') {
                    throw KeySyntaxError("Key ended with no corresponding value");
                }
            }
            return k;
        }

        static std::string consume_value(std::ifstream& s, int& c) {
            std::string v;
            while (c != '\n' && c != EOF && c != ';') {
                v.push_back(c);
                c = s.get();
                if (c == '\\') {
                    c = consume_escape(s);
                }
            }
            return v;
        }

        static std::string consume_section_name(std::ifstream& s, int& c) {
            std::string n;
            while (c != ']' && c != EOF) {
                n.push_back(c);
                c = s.get();
                if (c == '\n' || c == ';') {
                    throw SectionSyntaxError("section name with unterminated square brackets");
                }
            }
            return n;
        }
    } // namespace Util

    std::vector<char> const InitFile::ESCAPE_CHARS{{'=', ';', '\\'}};

    bool InitFile::is_escape_char(char c) {
        for (auto const& e: InitFile::ESCAPE_CHARS) {
            if (c == e) {
                return true;
            }
        }
        return false;
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

    std::string InitFile::escaped(std::string const& key) {
        std::string result{};
        for (int i = 0; i < key.size(); i++) {
            if (is_escape_char(key[i])) {
                result.push_back('\\');
            }
            result.push_back(key[i]);
        }
        return result;
    }

    InitFile InitFile::parse(std::string const& fileName) {
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
                // determine the depth of the subsection that is about to be read
                int prox = 0;
                while (s.good() && !s.eof() && c == '[') {
                    prox += 1;
                    c = s.get();
                    if (prox > (subsectionLevel + 1)) {
                        throw InvalidSubsection("Subsection level too deeply nested. Missing parent subsection");
                    }
                }
                // after the [
                if (c == '~') {
                    // ~ simply ends the section concordant with the subsection level indicated by the brackets
                    // pop while prox <= subsection level
                    while (prox <= subsectionLevel) {
                        pop_section(secstack);
                        subsectionLevel--;
                    }
                    // discard closing brackets
                    Util::gulp_to_char(s, '\n');
                } else {
                    // read the section name
                    std::string name = Util::consume_section_name(s, c);
                    // discard closing brackets
                    Util::gulp_close_brackets(s);

                    int d = Util::gulp_whitespace(s);

                    // if the line doesn't end or have a comment there is extra text
                    if (d != ';' && d != '\n') {
                        throw SectionSyntaxError("Extraneous text after section name. Keys must be on a new line");
                    }

                    // if after whitespace is a newline, we are on the first key in the section
                    // without the d == ';' check we eat the first key in each section
                    if (d == ';') {
                        Util::gulp_to_char(s, '\n');
                    }

                    /*
                     CASE 1: new section is the same level as the current section meaning that that current section is
                     closed and the new section is opened: this requires 1 pop if prox == subsectionLevel then pushing
                     the new section

                     CASE 2: new section is a higher level (closer to 0, aka default section) than the current
                     section closing the current section and all sections which are lower than the impending
                     new section. This requires several pops until the levels work out. Then it requires 1 push
                     to open the new section

                     CASE 3: new section is a lower level than the current section opening a new subsection of the
                     current section; creating a deeper subsection requires only pushing with no popping if prox > subsectionLevel
                     @brief: pop 1 time for equal section, many times for higher section, none for lower section
                     */
                    while (prox <= subsectionLevel) {
                        pop_section(secstack);
                        subsectionLevel--;
                    }

                    // push the new section after appropriate closing of existing sections
                    secstack.back()->subsections[name] = InitSection(name);
                    secstack.push_back(&secstack.back()->subsections[name]);
                    subsectionLevel = prox;
                }
                // section opener or closer is read so continue
                continue;
            }

            // if the current character is not whitespace, comments, or section headers, it must be a key value pair
            // read key
            std::string k = Util::consume_key(s, c);

            // discard =
            c = s.get();

            // read the value
            std::string v = Util::consume_value(s, c);

            // eat the rest of the line in case there's a comment
            if (c != '\n') {
                Util::gulp_to_char(s, '\n');
            }

            // add the key value pair to the current section
            secstack.back()->addEntry(InitEntry{k, v});
        }

        return file;
    }

    void InitFile::print(std::ostream& os) const {
        for (auto const& [name, entry]: defaultSection.entries) {
            os << escaped(entry.key()) << "=" << escaped(entry.value()) << std::endl;
        }
        for (auto const& [sec_name, section]: defaultSection.subsections) {
            section.print(os);
        }
    }
} // namespace Init
