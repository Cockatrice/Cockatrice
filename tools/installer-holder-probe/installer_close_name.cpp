#include "installer_close_name.h"

#include <string>

std::wstring installerCloseSocketName(unsigned long processId, const std::wstring &userName)
{
    std::wstring name = L"CockatriceInstallerClose-";
    name += std::to_wstring(processId);
    name += L"-";
    name += userName;
    return name;
}
