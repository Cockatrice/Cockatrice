#ifndef INSTALLER_CLOSE_NAME_H
#define INSTALLER_CLOSE_NAME_H

#include <string>

/**
 * @brief The local-socket name a running instance listens on for the installer's close
 *        request.
 *
 * The name carries the process id, so every instance of the payload has its own socket - a
 * second Cockatrice that was started independently, an Oracle and a Servatrice - and the
 * installer can ask each one instead of guessing which is the primary. It also carries the
 * account name, so one user's installer cannot reach another user's session.
 *
 * This is the same string the client builds with scopedLocalServerName() in
 * libcockatrice_utility/local_server_name.h: `CockatriceInstallerClose-<pid>-<user>`. It is
 * plain C++ so the probe and the client can be tested against each other off Windows.
 *
 * @param processId the id of the process that listens on the socket
 * @param userName the account name scopedLocalServerName() appends
 */
std::wstring installerCloseSocketName(unsigned long processId, const std::wstring &userName);

#endif // INSTALLER_CLOSE_NAME_H
