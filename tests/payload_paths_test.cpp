#include "payload_paths.h"

#include <gtest/gtest.h>
#include <string>

namespace
{
/// The directory part of a path spelled with the same case as the install directory, which
/// is the comparison the probe makes once both sides are folded - see isSameDirectory in
/// restart_manager_holders.cpp. Kept as a plain string here so the shape of a path can be
/// tested without Windows.
bool runsFrom(const std::wstring &imagePath, const std::wstring &installDir)
{
    return parentDirectoryOf(imagePath) == installDir;
}
} // namespace

TEST(PayloadPaths, SplitsAnImagePathIntoItsDirectoryAndName)
{
    EXPECT_EQ(fileNameOf(L"D:\\Games\\Cockatrice\\cockatrice.exe"), std::wstring(L"cockatrice.exe"));
    EXPECT_EQ(parentDirectoryOf(L"D:\\Games\\Cockatrice\\cockatrice.exe"), std::wstring(L"D:\\Games\\Cockatrice"));
    EXPECT_EQ(fileNameOf(L"cockatrice.exe"), std::wstring(L"cockatrice.exe"));
    EXPECT_EQ(parentDirectoryOf(L"cockatrice.exe"), std::wstring());
    EXPECT_EQ(parentDirectoryOf(L"D:/Games/Cockatrice/cockatrice.exe"), std::wstring(L"D:/Games/Cockatrice"));
}

TEST(PayloadPaths, OnlyAnImageInTheInstallDirectoryRunsFromIt)
{
    const std::wstring installDir = L"D:\\Games\\Cockatrice";
    EXPECT_TRUE(runsFrom(L"D:\\Games\\Cockatrice\\cockatrice.exe", installDir));
    // A folder the user keeps other applications in is not the installation, and neither is
    // a sibling whose name merely begins with the same letters.
    EXPECT_FALSE(runsFrom(L"D:\\Games\\Cockatrice\\old\\cockatrice.exe", installDir));
    EXPECT_FALSE(runsFrom(L"D:\\Games\\Cockatrice-old\\cockatrice.exe", installDir));
    EXPECT_FALSE(runsFrom(L"D:\\Games\\cockatrice.exe", installDir));
}

TEST(PayloadPaths, KnowsTheProgramsThePayloadInstalls)
{
    EXPECT_TRUE(isPayloadProgram(L"cockatrice.exe"));
    EXPECT_TRUE(isPayloadProgram(L"oracle.exe"));
    EXPECT_TRUE(isPayloadProgram(L"servatrice.exe"));
    EXPECT_TRUE(isPayloadProgram(L"COCKATRICE.EXE"));
    EXPECT_TRUE(isPayloadProgram(L"Servatrice.Exe"));
}

TEST(PayloadPaths, LeavesEveryOtherProgramAlone)
{
    // A program the user keeps in the install directory, the uninstaller, a service host and
    // a name that only looks like a payload program are all not instances of the payload.
    EXPECT_FALSE(isPayloadProgram(L"uninstall.exe"));
    EXPECT_FALSE(isPayloadProgram(L"tool.exe"));
    EXPECT_FALSE(isPayloadProgram(L"svchost.exe"));
    EXPECT_FALSE(isPayloadProgram(L"cockatrice2.exe"));
    EXPECT_FALSE(isPayloadProgram(L"mycockatrice.exe"));
    EXPECT_FALSE(isPayloadProgram(L""));
}
