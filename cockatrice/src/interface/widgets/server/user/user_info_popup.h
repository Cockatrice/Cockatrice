#ifndef COCKATRICE_USER_INFO_POPUP_H
#define COCKATRICE_USER_INFO_POPUP_H

#include "../../interface/widgets/server/game_type_map.h"
#include "user_list_painter.h"

#include <QFrame>
#include <QListView>
#include <QMap>
#include <QPixmap>
#include <QStandardItemModel>
#include <libcockatrice/network/server/remote/user_level.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

class AbstractClient;
class QLabel;
class QPushButton;
class TabSupervisor;

// ── Roles ─────────────────────────────────────────────────────────────────────

namespace PopupRoles
{
constexpr int GameData = Qt::UserRole + 10;
}

// ── Header widget ─────────────────────────────────────────────────────────────

/**
 * @class UserInfoHeaderWidget
 * @brief Paints the enlarged banner card art + circular avatar section at the
 *        top of the UserInfoPopup.
 *
 * Layout mirrors UserListPainter but at a larger scale: the card art fills the
 * full width as a semi-transparent background, a bottom gradient ensures the
 * avatar and username text remain legible, and the status ring colour matches
 * the UserListPainter convention.
 */
class UserInfoHeaderWidget : public QWidget
{
    Q_OBJECT

    static constexpr int HeaderHeight = 130;
    static constexpr int AvatarSize = 68;
    static constexpr int AvatarPawnSize = 46;
    static constexpr int LeftPad = 14;
    static constexpr int AvatarToTextGap = 10;

public:
    explicit UserInfoHeaderWidget(QWidget *parent = nullptr);

    void setUserData(const ServerInfo_User &user,
                     bool online,
                     const QPixmap &avatar,
                     const QPixmap &cardArt,
                     const CardArtParams &params);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    ServerInfo_User m_user;
    bool m_online = false;
    QPixmap m_avatar;
    QPixmap m_cardArt;
    CardArtParams m_params;
};

// ── Main popup ────────────────────────────────────────────────────────────────

/**
 * @class UserInfoPopup
 * @brief Floating panel showing an enlarged user card, quick action buttons,
 *        and a live scrollable games list.
 *
 * Lifecycle (mirrors DeckEditorDeckDockWidget):
 *   - showForUser() — populate, position externally, call show()
 *   - mouseEnteredPopup / mouseLeftPopup — caller manages hide timer
 *   - closeRequested() — emitted by the internal close button
 *
 * The popup is a Qt::Tool frameless child so windowOpacity animations and
 * move() in screen coordinates work identically to CardInfoPictureEnlargedWidget.
 *
 * Action signals map 1-to-1 to UserContextMenu::exec*() methods so all action
 * logic stays in one place.
 */
class UserInfoPopup : public QFrame
{
    Q_OBJECT

    static constexpr int PopupWidth = 316;

public:
    explicit UserInfoPopup(TabSupervisor *tabSupervisor,
                           AbstractClient *client,
                           const QMap<QString, QPixmap> *avatarCache,
                           const QMap<QString, QPixmap> *cardArtCache,
                           const QMap<QString, CardArtParams> *cardArtParamsMap,
                           QWidget *parent);

    /**
     * Populate the popup for @p userName and kick off a game list fetch.
     * Call show() / move() externally after this.
     */
    void
    showForUser(const QString &userName, const ServerInfo_User &userInfo, bool online, bool isBuddy, bool isIgnored);
    void fetchGames();

    [[nodiscard]] QString currentUser() const
    {
        return m_currentUser;
    }

signals:
    void mouseEnteredPopup();
    void mouseLeftPopup();
    void closeRequested();

    // ── Action signals — connect to UserContextMenu::exec*() ──────────────────
    void chatRequested(const QString &userName);
    void detailsRequested(const QString &userName);
    void showGamesRequested(const QString &userName);
    void addBuddyRequested(const QString &userName);
    void removeBuddyRequested(const QString &userName);
    void addIgnoreRequested(const QString &userName);
    void removeIgnoreRequested(const QString &userName);
    void banRequested(const QString &userName);
    void warnRequested(const QString &userName);
    void banHistoryRequested(const QString &userName);
    void warnHistoryRequested(const QString &userName);
    void adminNotesRequested(const QString &userName);
    void promoteToModRequested(const QString &userName);
    void demoteFromModRequested(const QString &userName);
    void promoteToJudgeRequested(const QString &userName);
    void demoteFromJudgeRequested(const QString &userName);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *e) override;
#else
    void enterEvent(QEvent *e) override;
#endif
    void leaveEvent(QEvent *e) override;

private slots:
    void refreshGames();
    void onGamesReceived(const Response &r, const QString &forUser);

private:
    void buildUi();
    void rebuildActionButtons(const ServerInfo_User &userInfo, bool online, bool isBuddy, bool isIgnored);

    TabSupervisor *m_ts;
    AbstractClient *m_client;
    const QMap<QString, QPixmap> *m_avatarCache;
    const QMap<QString, QPixmap> *m_cardArtCache;
    const QMap<QString, CardArtParams> *m_cardArtParamsMap;

    QString m_currentUser;
    ServerInfo_User m_currentUserInfo;
    bool m_currentOnline = false;

    UserInfoHeaderWidget *m_header;
    QWidget *m_actionArea; ///< rebuilt per user
    QListView *m_gamesView;
    QStandardItemModel *m_gamesModel;
    QLabel *m_gamesStatus;
    QPushButton *m_closeBtn;
    QPushButton *m_refreshBtn;
};

#endif // COCKATRICE_USER_INFO_POPUP_H