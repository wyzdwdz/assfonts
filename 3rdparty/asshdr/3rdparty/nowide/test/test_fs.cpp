//
// Copyright (c) 2015 Artyom Beilis (Tonkikh)
// Copyright (c) 2021 Alexander Grund
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#if defined(__GNUC__) && __GNUC__ >= 7
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#include <nowide/filesystem.hpp>

#include <nowide/convert.hpp>
#include <nowide/cstdio.hpp>
#include <nowide/fstream.hpp>
#include "test.hpp"
#if defined(_MSC_VER)
#pragma warning(disable : 4714) // function marked as __forceinline not inlined
#endif
#include <nowide/filesystem/operations.hpp>

// coverity[root_function]
void test_main(int, char** argv, char**)
{
    nowide::nowide_filesystem();
    const std::string prefix = argv[0];
    const std::string utf8_name =
      prefix + "\xf0\x9d\x92\x9e-\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82-\xE3\x82\x84\xE3\x81\x82.txt";

    {
        nowide::ofstream f(utf8_name.c_str());
        TEST(f);
        f << "Test" << std::endl;
    }

    TEST(nowide::filesystem::is_regular_file(nowide::widen(utf8_name)));
    TEST(nowide::filesystem::is_regular_file(utf8_name));

    TEST(nowide::remove(utf8_name.c_str()) == 0);

    TEST(!nowide::filesystem::is_regular_file(nowide::widen(utf8_name)));
    TEST(!nowide::filesystem::is_regular_file(utf8_name));

    const nowide::filesystem::path path = utf8_name;
    {
        nowide::ofstream f(path);
        TEST(f);
        f << "Test" << std::endl;
        TEST(is_regular_file(path));
    }
    {
        nowide::ifstream f(path);
        TEST(f);
        std::string test;
        f >> test;
        TEST(test == "Test");
    }
    {
        nowide::fstream f(path);
        TEST(f);
        std::string test;
        f >> test;
        TEST(test == "Test");
    }
    nowide::filesystem::remove(path);
}
