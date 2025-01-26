cmake_minimum_required(VERSION 3.30)
project(initparserxx)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "-fsanitize=undefined -fsanitize=address ${CMAKE_CXX_FLAGS}")

add_library(InitParserCPP STATIC InitEntry.cpp
                InitEntry.h
                InitSection.cpp
                InitSection.h
                InitFile.cpp
        InitFile.h
        InitUtils.cpp
        InitUtils.h
        InitException.cpp
        InitException.h)

add_executable(initparserxx main.cpp
        InitEntry.cpp
        InitEntry.h
        InitSection.cpp
        InitSection.h
        InitFile.cpp
        InitFile.h
        InitUtils.cpp
        InitUtils.h
        InitException.cpp
        InitException.h)
