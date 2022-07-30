/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include "../../../NativeApp/include/CommandLineParser.hpp"

#include "gtest/gtest.h"

using namespace Diligent;

namespace
{

TEST(Tools_CommandLineParser, Parse)
{
    {
        CommandLineParser NullParser{0, nullptr};
    }

    enum TEST_ENUM
    {
        TEST_ENUM_INVALID,
        TEST_ENUM1,
        TEST_ENUM2,
        TEST_ENUM3
    };
    const std::vector<std::pair<const char*, TEST_ENUM>> EnumVals =
        {
            {"ENUM1", TEST_ENUM1},
            {"ENUM2", TEST_ENUM2},
            {"ENUM3", TEST_ENUM3} //
        };

    std::vector<const char*> Args =
        {
            "--bool1",
            "true",
            "--bool2",
            "1",
            "--bool3",
            "0",
            "--bool4",
            "false",
            "--bool5=true",
            "-b",
            "1",
            "--help",
            "-h",

            "--int1",
            "10",
            "--int2",
            "-20",
            "--int3=-30",
            "-i",
            "-40",

            "--uint1",
            "100",
            "--uint2=300",
            "-u",
            "400",

            "--float1",
            "-4.5",
            "--float2=-6.5e+2",
            "-f",
            "-7.5e+3",

            "--double1",
            "10.25",
            "--double2=-7.5e+2",
            "-d",
            "-8.25e+3",

            "--str1",
            "abc",
            "--str2=--def",
            "-s",
            "xyz",

            "--enum1",
            "ENUM1",
            "--enum2=EnUm2",
            "-e",
            "enum3",

            "OtherArg",
            "--OtherArg2",
            "OtherVal",
            "--OtherArg3=OtherVal3",
            "-o",
            "OtherVal",
            "--NoValue1=",
            "--NoValue2",
            "--NoValue3",
            "-n",
            "--NoValue4" //
        };
    CommandLineParser ArgsParser{Args};

    {
        //  "--bool1",
        //  "true",
        //  "--bool2",
        //  "1",
        //  "--bool3",
        //  "0",
        //  "--bool4",
        //  "false",
        //  "--bool5=true",
        //  "-b",
        //  "1",
        //  "--help",
        //  "-h",

        bool b = false;
        EXPECT_TRUE(ArgsParser.Parse("bool1", b, false));
        EXPECT_TRUE(b);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size()));

        b = false;
        EXPECT_TRUE(ArgsParser.Parse("bool1", b));
        EXPECT_TRUE(b);
        ArgsParser.ArgC();
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size() - 2));

        b = false;
        EXPECT_TRUE(ArgsParser.Parse("bool2", b));
        EXPECT_TRUE(b);

        b = true;
        EXPECT_TRUE(ArgsParser.Parse("bool3", b));
        EXPECT_FALSE(b);

        b = true;
        EXPECT_TRUE(ArgsParser.Parse("bool4", b));
        EXPECT_FALSE(b);

        b = false;
        EXPECT_TRUE(ArgsParser.Parse("bool5", b));
        EXPECT_TRUE(b);

        b = false;
        EXPECT_TRUE(ArgsParser.Parse(nullptr, 'b', b));
        EXPECT_TRUE(b);

        b = false;
        EXPECT_TRUE(ArgsParser.Parse("help", b));
        EXPECT_TRUE(b);

        b = false;
        EXPECT_TRUE(ArgsParser.Parse(nullptr, 'h', b));
        EXPECT_TRUE(b);

        b = false;
        EXPECT_FALSE(ArgsParser.Parse("boolX", 'x', b));
        EXPECT_FALSE(b);

        b = true;
        EXPECT_FALSE(ArgsParser.Parse("boolY", 'y', b));
        EXPECT_TRUE(b);

        Args.erase(Args.begin(), Args.begin() + 13);
        EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
    }

    {
        //  "--int1",
        //  "10",
        //  "--int2",
        //  "-20",
        //  "--int3=-30",
        //  "-i",
        //  "-40",

        int i = 0;
        EXPECT_TRUE(ArgsParser.Parse("int1", i, false));
        EXPECT_EQ(i, 10);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size()));

        i = 0;
        EXPECT_TRUE(ArgsParser.Parse("int1", i));
        EXPECT_EQ(i, 10);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size() - 2));

        i = 0;
        EXPECT_TRUE(ArgsParser.Parse("int2", i));
        EXPECT_EQ(i, -20);

        i = 0;
        EXPECT_TRUE(ArgsParser.Parse("int3", i));
        EXPECT_EQ(i, -30);

        i = 0;
        EXPECT_TRUE(ArgsParser.Parse(nullptr, 'i', i));
        EXPECT_EQ(i, -40);

        i = 123;
        EXPECT_FALSE(ArgsParser.Parse("intX", 'x', i));
        EXPECT_EQ(i, 123);

        Args.erase(Args.begin(), Args.begin() + 7);
        EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
    }

    {
        //  "--uint1",
        //  "100",
        //  "--uint2=300",
        //  "-u",
        //  "400",

        unsigned int u = 0;
        EXPECT_TRUE(ArgsParser.Parse("uint1", u, false));
        EXPECT_EQ(u, 100u);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size()));

        u = 0;
        EXPECT_TRUE(ArgsParser.Parse("uint1", u));
        EXPECT_EQ(u, 100u);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size() - 2));

        u = 0;
        EXPECT_TRUE(ArgsParser.Parse("uint2", u));
        EXPECT_EQ(u, 300u);

        u = 0;
        EXPECT_TRUE(ArgsParser.Parse(nullptr, 'u', u));
        EXPECT_EQ(u, 400u);

        u = 123;
        EXPECT_FALSE(ArgsParser.Parse("uintX", 'x', u));
        EXPECT_EQ(u, 123u);

        Args.erase(Args.begin(), Args.begin() + 5);
        EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
    }

    {
        //  "--float1",
        //  "-4.5",
        //  "--float2=-6.5e+2",
        //  "-f",
        //  "-7.5e+3",

        float f = 0;
        EXPECT_TRUE(ArgsParser.Parse("float1", f, false));
        EXPECT_EQ(f, -4.5f);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size()));

        f = 0;
        EXPECT_TRUE(ArgsParser.Parse("float1", f));
        EXPECT_EQ(f, -4.5f);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size() - 2));

        f = 0;
        EXPECT_TRUE(ArgsParser.Parse("float2", f));
        EXPECT_EQ(f, -6.5e+2f);

        f = 0;
        EXPECT_TRUE(ArgsParser.Parse(nullptr, 'f', f));
        EXPECT_EQ(f, -7.5e+3f);

        f = 123;
        EXPECT_FALSE(ArgsParser.Parse("floatX", 'x', f));
        EXPECT_EQ(f, 123.f);

        Args.erase(Args.begin(), Args.begin() + 5);
        EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
    }

    {
        //   "--double1",
        //   "10.25",
        //   "--double2=-7.5e+2",
        //   "-d",
        //   "-8.25e+3",

        double d = 0;
        EXPECT_TRUE(ArgsParser.Parse("double1", d, false));
        EXPECT_EQ(d, 10.25);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size()));

        d = 0;
        EXPECT_TRUE(ArgsParser.Parse("double1", d));
        EXPECT_EQ(d, 10.25);

        d = 0;
        EXPECT_TRUE(ArgsParser.Parse("double2", d));
        EXPECT_EQ(d, -7.5e+2);

        d = 0;
        EXPECT_TRUE(ArgsParser.Parse(nullptr, 'd', d));
        EXPECT_EQ(d, -8.25e+3);

        d = 123;
        EXPECT_FALSE(ArgsParser.Parse("doublex", 'x', d));
        EXPECT_EQ(d, 123.0);

        Args.erase(Args.begin(), Args.begin() + 5);
        EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
    }

    {
        //  "--str1",
        //  "abc",
        //  "--str2=--def",
        //  "-s",
        //  "xyz",

        std::string s;
        EXPECT_TRUE(ArgsParser.Parse("str1", s, false));
        EXPECT_EQ(s, "abc");
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size()));

        s = "";
        EXPECT_TRUE(ArgsParser.Parse("str1", s));
        EXPECT_EQ(s, "abc");
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size() - 2));

        s = "";
        EXPECT_TRUE(ArgsParser.Parse("str2", s));
        EXPECT_EQ(s, "--def");

        s = "";
        EXPECT_TRUE(ArgsParser.Parse(nullptr, 's', s));
        EXPECT_EQ(s, "xyz");

        s = "mnk";
        EXPECT_FALSE(ArgsParser.Parse("strx", 'x', s));
        EXPECT_EQ(s, "mnk");

        Args.erase(Args.begin(), Args.begin() + 5);
        EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
    }

    {
        //  "--enum1",
        //  "ENUM1",
        //  "--enum2=EnUm2",
        //  "-e",
        //  "enum3" //

        TEST_ENUM e = TEST_ENUM_INVALID;
        EXPECT_TRUE(ArgsParser.ParseEnum("enum1", 0, EnumVals, e, /*CaseSensitive = */ false, /*RemoveArgument = */ false));
        EXPECT_EQ(e, TEST_ENUM1);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size()));

        e = TEST_ENUM_INVALID;
        EXPECT_TRUE(ArgsParser.ParseEnum("enum1", 0, EnumVals, e, /*CaseSensitive = */ true));
        EXPECT_EQ(e, TEST_ENUM1);
        EXPECT_EQ(ArgsParser.ArgC(), static_cast<int>(Args.size() - 2));

        e = TEST_ENUM_INVALID;
        EXPECT_TRUE(ArgsParser.ParseEnum("enum2", 0, EnumVals, e, /*CaseSensitive = */ false));
        EXPECT_EQ(e, TEST_ENUM2);

        e = TEST_ENUM_INVALID;
        EXPECT_FALSE(ArgsParser.ParseEnum("enum2", 0, EnumVals, e, /*CaseSensitive = */ true));
        EXPECT_EQ(e, TEST_ENUM_INVALID);

        e = TEST_ENUM_INVALID;
        EXPECT_TRUE(ArgsParser.ParseEnum(nullptr, 'e', EnumVals, e, /*CaseSensitive = */ false));
        EXPECT_EQ(e, TEST_ENUM3);

        e = TEST_ENUM_INVALID;
        EXPECT_FALSE(ArgsParser.ParseEnum("enumx", 'x', EnumVals, e));
        EXPECT_EQ(e, TEST_ENUM_INVALID);

        Args.erase(Args.begin(), Args.begin() + 5);
        EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
    }

    {
        //  "--NoValue1=",
        //  "--NoValue2",
        //  "--NoValue3",
        //  "-n",
        //  "--NoValue4"
        std::string str;
        EXPECT_TRUE(ArgsParser.Parse("NoValue1", str));
        EXPECT_EQ(str, "");

        int i = 0;
        EXPECT_FALSE(ArgsParser.Parse("NoValue2", i));
        bool b = false;
        EXPECT_TRUE(ArgsParser.Parse("NoValue2", b));
        EXPECT_TRUE(b);

        float f = 0;
        EXPECT_FALSE(ArgsParser.Parse("NoValue3", f));
        b = false;
        EXPECT_TRUE(ArgsParser.Parse("NoValue3", b));
        EXPECT_TRUE(b);

        unsigned int u = 0;
        EXPECT_FALSE(ArgsParser.Parse(nullptr, 'n', u));
        b = false;
        EXPECT_TRUE(ArgsParser.Parse(nullptr, 'n', b));
        EXPECT_TRUE(b);

        double d = 0;
        EXPECT_FALSE(ArgsParser.Parse("NoValue4", d));
        b = false;
        EXPECT_TRUE(ArgsParser.Parse("NoValue4", b));
        EXPECT_TRUE(b);

        str = "pqr";
        EXPECT_FALSE(ArgsParser.Parse("NoValue4", str));
        EXPECT_EQ(str, "pqr");

        TEST_ENUM e = TEST_ENUM1;
        EXPECT_FALSE(ArgsParser.ParseEnum("NoValue4", 0, EnumVals, e));
        EXPECT_EQ(e, TEST_ENUM1);
    }
}

TEST(Tools_CommandLineParser, DuplicateArguments)
{
    std::vector<const char*> Args =
        {
            "--arg1",
            "abc",
            "--arg2",
            "123",
            "--arg1",
            "0",
            "-a",
            "true",
            "--arg3=456",
            "--arg1=def" //
        };
    CommandLineParser ArgsParser{Args};

    std::string str;
    EXPECT_TRUE(ArgsParser.Parse("arg1", 'a', str));
    EXPECT_EQ(str, "def");
    Args.erase(Args.begin() + 9);
    Args.erase(Args.begin() + 4, Args.begin() + 8);
    Args.erase(Args.begin(), Args.begin() + 2);
    EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
}


TEST(Tools_CommandLineParser, InvalidShortName)
{
    std::vector<const char*> Args =
        {
            "-arg1",
            "abc",
            "--arg2",
            "def",
            "-arg2",
            "123" //
        };
    CommandLineParser ArgsParser{Args};

    std::string str = "xyz";
    EXPECT_FALSE(ArgsParser.Parse("arg1", 'a', str));
    EXPECT_EQ(str, "xyz");

    EXPECT_TRUE(ArgsParser.Parse("arg2", 'a', str));
    EXPECT_EQ(str, "def");

    Args.erase(Args.begin() + 2, Args.begin() + 4);
    EXPECT_EQ(Args, std::vector<const char*>(ArgsParser.ArgV(), ArgsParser.ArgV() + ArgsParser.ArgC()));
}

} // namespace
