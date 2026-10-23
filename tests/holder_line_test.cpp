#include "holder_line.h"

#include <gtest/gtest.h>
#include <string>

namespace
{
Holder runningProcess(const std::wstring &imageName, unsigned long processId)
{
    Holder holder;
    holder.imageName = imageName;
    holder.processId = processId;
    return holder;
}

Holder service(const std::wstring &appName, const std::wstring &shortName, unsigned long processId)
{
    Holder holder;
    holder.appName = appName;
    holder.serviceShortName = shortName;
    holder.processId = processId;
    holder.isService = true;
    return holder;
}
} // namespace

TEST(HolderLine, NamesARunningProcessTheWayTaskManagerDoes)
{
    EXPECT_EQ(formatHolder(runningProcess(L"cockatrice.exe", 24340)),
              std::wstring(L"cockatrice.exe (pid 24340, running)"));
}

TEST(HolderLine, NamesAServiceByTheNameTheUserCanSearchFor)
{
    // The image of a service is often a shared host process, so the image name would
    // point at the wrong thing; both of the service's own names are kept.
    Holder holder = service(L"Sophos Anti-Virus", L"SAVService", 4412);
    holder.imageName = L"svchost.exe";
    EXPECT_EQ(formatHolder(holder), std::wstring(L"Sophos Anti-Virus (pid 4412, service SAVService)"));
}

TEST(HolderLine, FallsBackFromTheImageToTheApplicationName)
{
    // Restart Manager can name a process whose image this one is not allowed to open.
    Holder holder;
    holder.appName = L"Cockatrice";
    holder.processId = 7;
    EXPECT_EQ(formatHolder(holder), std::wstring(L"Cockatrice (pid 7, running)"));
}

TEST(HolderLine, NamesAnEmptyHolderRatherThanPrintingAnEmptyParenthesis)
{
    Holder holder;
    holder.processId = 9;
    EXPECT_EQ(formatHolder(holder), std::wstring(L"an unnamed process (pid 9, running)"));
}

TEST(HolderLine, JoinsHoldersIntoOneLine)
{
    const std::vector<Holder> holders = {
        runningProcess(L"cockatrice.exe", 1),
        runningProcess(L"oracle.exe", 2),
    };
    EXPECT_EQ(formatHolders(holders), std::wstring(L"cockatrice.exe (pid 1, running); oracle.exe (pid 2, running)"));
}

TEST(HolderLine, CountsTheHoldersItLeavesOut)
{
    std::vector<Holder> holders;
    for (unsigned long processId = 1; processId <= HOLDER_LINE_MAX + 2; ++processId) {
        holders.push_back(runningProcess(L"cockatrice.exe", processId));
    }
    const std::wstring line = formatHolders(holders);
    // The dialog has to stay readable, but a list that silently stopped at five would
    // hide that there is more to close.
    EXPECT_NE(line.find(L"pid 5, running"), std::wstring::npos);
    EXPECT_EQ(line.find(L"pid 6, running"), std::wstring::npos);
    EXPECT_NE(line.find(L"and 2 more"), std::wstring::npos);
}

TEST(HolderLine, StopsNamingHoldersBeforeTheLineOutgrowsTheDialog)
{
    // A Windows service name can be 255 characters, and the abort dialog has to fit the
    // list next to its own text and the log path in a 1024-character NSIS string.
    std::vector<Holder> holders;
    for (unsigned long processId = 1; processId <= 3; ++processId) {
        holders.push_back(service(std::wstring(255, L'S'), L"S", processId));
    }
    const std::wstring line = formatHolders(holders);
    // The first holder is still named in full; the others are counted rather than written
    // out until the line overflows.
    EXPECT_EQ(line.compare(0, 255, std::wstring(255, L'S')), 0);
    EXPECT_NE(line.find(L"(pid 1, service S)"), std::wstring::npos);
    EXPECT_EQ(line.find(L"pid 2"), std::wstring::npos);
    EXPECT_NE(line.find(L"and 2 more"), std::wstring::npos);
    EXPECT_LE(line.size(), HOLDER_LINE_CHAR_MAX + 20);
}

TEST(HolderLine, FormatsNothingAsAnEmptyLine)
{
    EXPECT_EQ(formatHolders({}), std::wstring());
}
