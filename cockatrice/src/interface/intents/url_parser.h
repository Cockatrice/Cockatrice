#ifndef URL_PARSER_H
#define URL_PARSER_H

#include <QObject>
#include <libcockatrice/utility/intent.h>

class ConnectionController;
class QWidget;
class TabSupervisor;

/**
 * @brief Builds an Intent chain for a cockatrice:// URL.
 *
 * Supported URL forms:
 *   cockatrice://joingame?hostname=H&port=P&roomid=R&gameid=G
 *   cockatrice://joingame?hostname=H&port=P&roomid=R&gameid=G&spectate=1
 *
 * Credentials are intentionally NOT accepted via URL — URLs are leak-prone
 * (shell history, EDR capture, local-socket forwarding, browser history).  If
 * the target server requires authentication, the chain fails at the login step
 * and the user can complete the connection via the normal Connect dialog.
 *
 * The pure URL-validation logic lives in UrlUtils::parseJoinGameUrl
 * (libcockatrice/utility/url_utils.h) and is unit-tested there; this class
 * only handles chain construction.
 *
 * Ownership: the returned Intent (and any chained intents created as children)
 * is owned by the caller; parenting the result to a QObject will ensure
 * automatic cleanup.
 */
class UrlParser
{
public:
    /**
     * @param url            Raw URL string (e.g. "cockatrice://joingame?...").
     * @param controller     Connection controller used for connect / login intents.
     * @param supervisor     Tab supervisor used for join-room / join-game intents.
     * @param dialogParent   QWidget used as the parent for any UI dialog the
     *                       chain may show (e.g. DlgConnect when no saved
     *                       credentials match).  Typically the MainWindow.
     * @param parent         QObject parent given to every intent in the chain.
     * @return               Root intent of the chain, or nullptr on parse failure.
     */
    static Intent *parse(const QString &url,
                         ConnectionController *controller,
                         TabSupervisor *supervisor,
                         QWidget *dialogParent,
                         QObject *parent = nullptr);
};

#endif // URL_PARSER_H
