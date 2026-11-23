#ifndef COCKATRICE_INSTALLER_CLOSE_PROTOCOL_H
#define COCKATRICE_INSTALLER_CLOSE_PROTOCOL_H

/**
 * @brief The one line the installer's holder probe writes to a running instance.
 *
 * Both ends include this header: the probe writes the line
 * (tools/installer-holder-probe/installer_close_request.cpp) and the client's
 * InstallerCloseServer reads it. Keeping the bytes in one place means the two ends cannot
 * drift apart, and the test pins the exact line so a change to it is a deliberate protocol
 * change rather than a typo in one copy.
 *
 * Plain C++ on purpose: the probe links no Qt and no part of this project, so this header
 * must stay free of both.
 */
inline constexpr char INSTALLER_CLOSE_COMMAND[] = "COCKATRICE_INSTALLER_CLOSE\n";

#endif // COCKATRICE_INSTALLER_CLOSE_PROTOCOL_H
