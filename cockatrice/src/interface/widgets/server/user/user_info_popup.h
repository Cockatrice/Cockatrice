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

// ── Popup theme ───────────────────────────────────────────────────────────────

/**
 * Palette-derived colors for the popup chrome. Both color schemes read from
 * the active QPalette so custom palettes are respected; @c dark only tunes the
 * blend strengths.
 */
struct PopupTheme
{
    bool dark = false;
    QColor bg;
    QColor border;
    QColor text;
    QColor subText;
    QColor buttonBg;
    QColor buttonBorder;
    QColor buttonHover;
    QColor buttonPressed;
    QColor buttonDisabled;
    QColor closeBg;
    QColor closeHover;
    QColor gamesRow;
    QColor gamesSelected;
    QColor gamesSeparator;
    QColor statusText;

    static PopupTheme fromPalette(const QPalette &palette, bool dark);
};

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

    void setUserData(const ServerInfo_User &_user,
                     bool _online,
                     const QPixmap &_avatar,
                     const QPixmap &_cardArt,
                     const CardArtParams &_params);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    ServerInfo_User user;
    bool online = false;
    QPixmap avatar;
    QPixmap cardArt;
    CardArtParams params;
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
    explicit UserInfoPopup(TabSupervisor *_ts,
                           AbstractClient *_client,
                           const QMap<QString, QPixmap> *_avatarCache,
                           const QMap<QString, QPixmap> *_cardArtCache,
                           const QMap<QString, CardArtParams> *_cardArtParamsMap,
                           QWidget *parent);

    /**
     * Populate the popup for @p userName and kick off a game list fetch.
     * Call show() / move() externally after this.
     */
    void
    showForUser(const QString &userName, const ServerInfo_User &userInfo, bool online, bool isBuddy, bool isIgnored);
    void fetchGames();

    [[nodiscard]] QString getCurrentUser() const
    {
        return currentUser;
    }

    /** Called when buddy/ignore status changes externally while popup is open. */
    void updateActionButtons(const ServerInfo_User &userInfo, bool online, bool isBuddy, bool isIgnored);

    /** Re-pulls the avatar/card art for the currently shown user (e.g. after it loads). */
    void refreshHeader();

signals:
    void mouseEnteredPopup();
    void mouseLeftPopup();
    void closeRequested();

    /** Emitted when the user requests joining or spectating a game in the list. */
    void joinGameRequested(int gameId, int roomId, bool asSpectator);

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
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;

private slots:
    void refreshGames();
    void onGamesReceived(const Response &r, const QString &forUser);
    void onGamesContextMenu(const QPoint &pos);

private:
    void buildUi();
    void applyTheme();
    void rebuildActionButtons(const ServerInfo_User &userInfo, bool online, bool isBuddy, bool isIgnored);

    TabSupervisor *ts;
    AbstractClient *client;
    const QMap<QString, QPixmap> *avatarCache;
    const QMap<QString, QPixmap> *cardArtCache;
    const QMap<QString, CardArtParams> *cardArtParamsMap;

    PopupTheme theme;

    QString currentUser;
    ServerInfo_User currentUserInfo;
    bool currentOnline = false;

    UserInfoHeaderWidget *header;
    QWidget *actionArea; ///< rebuilt per user
    QLabel *gamesLabel;
    QFrame *separator;
    QListView *gamesView;
    QStandardItemModel *gamesModel;
    QLabel *gamesStatus;
    QPushButton *closeBtn;
    QPushButton *refreshBtn;
};

#endif // COCKATRICE_USER_INFO_POPUP_H