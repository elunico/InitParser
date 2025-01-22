//
// Created by Thomas Povinelli on 1/22/25.
//

#ifndef INIT_EXCEPTION_H
#define INIT_EXCEPTION_H
#include <exception>
#include <string>

namespace Init {
    class InitException : public std::exception {
    public:
        std::string message;

        explicit InitException(std::string const& w);

        virtual char const *what() const noexcept override ;

        virtual ~InitException() override;
    };

    class InvalidSubsection : public InitException {
        using InitException::InitException;
    };

    class ParseException : public InitException {
        using InitException::InitException;
    };

    class KeySyntaxError : public ParseException {
        using ParseException::ParseException;
    };

    class SectionSyntaxError : public ParseException {
        using ParseException::ParseException;
    };
} // Init

#endif //INIT_EXCEPTION_H
