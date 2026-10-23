#include "holder_line.h"

#include <algorithm>

namespace
{
std::wstring displayNameOf(const Holder &holder)
{
    // A service's application name is its long name, which is what the user can
    // search for; its image name is usually a shared host process (svchost.exe) and
    // would name the wrong thing.
    if (holder.isService && !holder.appName.empty()) {
        return holder.appName;
    }
    if (!holder.imageName.empty()) {
        return holder.imageName;
    }
    if (!holder.appName.empty()) {
        return holder.appName;
    }
    return L"an unnamed process";
}
} // namespace

std::wstring formatHolder(const Holder &holder)
{
    std::wstring line = displayNameOf(holder);
    line += L" (pid ";
    line += std::to_wstring(holder.processId);
    if (holder.isService) {
        line += L", service ";
        line += holder.serviceShortName.empty() ? L"without a short name" : holder.serviceShortName;
    } else {
        line += L", running";
    }
    line += L")";
    return line;
}

std::wstring formatHolders(const std::vector<Holder> &holders)
{
    const std::size_t listed = std::min(holders.size(), HOLDER_LINE_MAX);
    std::wstring line;
    for (std::size_t index = 0; index < listed; ++index) {
        if (!line.empty()) {
            line += L"; ";
        }
        line += formatHolder(holders[index]);
    }
    if (holders.size() > listed) {
        line += L"; and ";
        line += std::to_wstring(holders.size() - listed);
        line += L" more";
    }
    return line;
}
