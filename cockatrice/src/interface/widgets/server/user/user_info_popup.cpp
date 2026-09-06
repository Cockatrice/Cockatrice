#include "user_info_popup.h"

#include "../../cards/art_crop_attribution.h"
#include "../../interface/pixel_map_generator.h"
#include "../../interface/theme_manager.h"
#include "../../interface/widgets/tabs/tab_supervisor.h"
#include "user_list_painter.h"

#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/response_get_games_of_user.pb.h>
#include <libcockatrice/protocol/pending_command.h>

/// Qt stylesheets accept #aarrggbb, which is QColor::name(QColor::HexArgb).
static QString colorStr(const QColor &color)
{
    return color.name(QColor::HexArgb);
}

PopupTheme PopupTheme::fromPalette(const QPalette &palette, bool dark)
{
    PopupTheme t;
    t.dark = dark;
    const QColor window = palette.color(QPalette::Window);
    const QColor base = palette.color(QPalette::Base);
    const QColor mid = palette.color(QPalette::Mid);
    const QColor text = palette.color(QPalette::Text);
    const QColor disabledText = palette.color(QPalette::Disabled, QPalette::Text);
    const QColor highlight = palette.color(QPalette::Highlight);

    t.bg = window;
    t.border = mid;
    t.text = text;
    t.subText = disabledText;
    t.statusText = disabledText;
    t.buttonBg = base;
    t.buttonBorder = mid;
    t.buttonHover = UserListPainter::blend(base, highlight, dark ? 0.30 : 0.12);
    t.buttonPressed = UserListPainter::blend(base, highlight, dark ? 0.50 : 0.25);
    t.buttonDisabled = disabledText;
    t.closeBg = UserListPainter::blend(base, window, 0.5);
    t.closeHover = dark ? QColor(200, 50, 50) : UserListPainter::blend(QColor(200, 50, 50), base, 0.45);
    t.gamesRow = base;
    t.gamesSelected = UserListPainter::blend(base, highlight, dark ? 0.45 : 0.30);
    t.gamesSeparator = mid;
    t.gamesSeparator.setAlpha(90);
    return t;
}

// ── Compact game row delegate ─────────────────────────────────────────────────

class PopupGameDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        return QSize(0, 38);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        const QVariant var = index.data(PopupRoles::GameData);
        if (!var.isValid()) {
            QStyledItemDelegate::paint(p, option, index);
            return;
        }

        p->save();
        p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        const QRect rect = option.rect;
        const ServerInfo_Game game = var.value<ServerInfo_Game>();
        const bool selected = option.state & QStyle::State_Selected;
        const bool dark = themeManager && themeManager->isDarkModeActive();
        // The widget palette can be stale after a runtime theme change, so the
        // rows are styled from the application palette (always current).
        const QPalette pal = qApp->palette();
        const QColor base = pal.color(QPalette::Base);
        const QColor highlight = pal.color(QPalette::Highlight);

        p->fillRect(rect, selected ? UserListPainter::blend(base, highlight, dark ? 0.45 : 0.30) : base);

        // State colour dot
        const QColor dot = game.started()                                ? QColor(239, 68, 68)
                           : (game.player_count() >= game.max_players()) ? QColor(249, 115, 22)
                           : game.with_password()                        ? QColor(59, 130, 246)
                                                                         : QColor(34, 197, 94);
        p->setPen(Qt::NoPen);
        p->setBrush(dot);
        p->drawEllipse(QRectF(rect.left() + 9, rect.top() + (rect.height() - 8) / 2.0, 8, 8));

        // Game title (bold, elided)
        QFont tf = option.font;
        tf.setBold(true);
        p->setFont(tf);
        p->setPen(pal.color(QPalette::Text));
        const int textX = rect.left() + 26;
        const int countW = 52;
        const int titleW = rect.width() - textX - countW - 6;
        p->drawText(QRect(textX, rect.top(), titleW, rect.height()), Qt::AlignVCenter | Qt::AlignLeft,
                    QFontMetrics(tf).elidedText(QString::fromStdString(game.description()), Qt::ElideRight, titleW));

        // Player count
        const bool full = game.player_count() >= game.max_players();
        p->setFont(option.font);
        p->setPen(full ? QColor(249, 115, 22) : pal.color(QPalette::Disabled, QPalette::Text));
        p->drawText(QRect(rect.right() - countW - 4, rect.top(), countW, rect.height()),
                    Qt::AlignVCenter | Qt::AlignRight,
                    QStringLiteral("%1/%2").arg(game.player_count()).arg(game.max_players()));

        // Row separator
        QColor separator = pal.color(QPalette::Mid);
        separator.setAlpha(90);
        p->setPen(separator);
        p->drawLine(rect.bottomLeft(), rect.bottomRight());

        p->restore();
    }
};

// ── UserInfoHeaderWidget ──────────────────────────────────────────────────────

UserInfoHeaderWidget::UserInfoHeaderWidget(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(HeaderHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void UserInfoHeaderWidget::setUserData(const ServerInfo_User &_user,
                                       bool _online,
                                       const QPixmap &_avatar,
                                       const QPixmap &_cardArt,
                                       const CardArtParams &_params)
{
    user = _user;
    online = _online;
    avatar = _avatar;
    cardArt = _cardArt;
    params = _params;

    attribution.clear();
    if (user.has_card_art_params()) {
        const ExactCard card =
            CardDatabaseManager::query()->getCard({QString::fromStdString(user.card_art_params().card_name()),
                                                   QString::fromStdString(user.card_art_params().card_provider_id())});
        if (card) {
            attribution = buildArtAttribution(card);
        }
    }
    update();
}

void UserInfoHeaderWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    const QRect rect = this->rect();
    const UserLevelFlags level(user.user_level());
    const QString userName = QString::fromStdString(user.name());
    const QString privLevel = QString::fromStdString(user.privlevel());

    const bool dark = themeManager && themeManager->isDarkModeActive();
    const UserListPainter::Style style = UserListPainter::resolveStyle(qApp->palette(), dark);

    // Palette surface
    {
        QLinearGradient bg(0, 0, rect.width(), 0);
        bg.setColorAt(0, style.cardStart);
        bg.setColorAt(1, style.cardEnd);
        p.fillRect(rect, bg);
    }

    // ── Card art background ───────────────────────────────────────────────────
    if (!cardArt.isNull()) {
        // Same DPR normalization as UserListPainter::drawCardArt: the cache
        // carries screen scaled pixmaps on HiDPI displays, the math below is
        // in raw pixels.
        QPixmap art = cardArt;
        art.setDevicePixelRatio(1.0);

        const int w = rect.width();
        const int h = rect.height();
        const int mL = qRound(w * params.marginPctL);
        const int mR = qRound(w * params.marginPctR);
        const int dW = w - mL - mR;

        const double base = qMax(double(dW) / art.width(), double(h) / art.height());
        const double scale = base * params.zoom;
        const int sW = qRound(art.width() * scale);
        const int sH = qRound(art.height() * scale);

        const QPixmap scaled = art.scaled(sW, sH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        // Clamp against stored zoom < 1, which can push srcX negative and silently
        // underfill the strip with transparent padding
        const int safeSrcX = qBound(0, (sW - dW) / 2, qMax(0, sW - dW));
        const int safeSrcY = qBound(0, qRound((sH - h) * params.verticalOffset), qMax(0, sH - h));

        QImage img =
            scaled.copy(safeSrcX, safeSrcY, dW, h).toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
        {
            QPainter mask(&img);
            mask.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            QLinearGradient g(0, 0, img.width(), 0);
            g.setColorAt(0.00, Qt::transparent);
            g.setColorAt(0.18, Qt::white);
            g.setColorAt(0.82, Qt::white);
            g.setColorAt(1.00, Qt::transparent);
            mask.fillRect(img.rect(), g);
        }
        p.setOpacity(0.48);
        p.drawImage(mL, 0, img);
        p.setOpacity(1.0);
    }

    // Bottom gradient overlay so avatar and text are always legible. The scrim
    // is the palette's Window color so it reads naturally in either scheme.
    {
        const QColor scrim = qApp->palette().color(QPalette::Window);
        QLinearGradient ov(0, 0, 0, rect.height());
        ov.setColorAt(0.0, QColor(scrim.red(), scrim.green(), scrim.blue(), 0));
        ov.setColorAt(0.55, QColor(scrim.red(), scrim.green(), scrim.blue(), 110));
        ov.setColorAt(1.0, QColor(scrim.red(), scrim.green(), scrim.blue(), 230));
        p.fillRect(rect, ov);
    }

    // ── Avatar ────────────────────────────────────────────────────────────────
    const QColor accent = [&]() -> QColor {
        if (level.testFlag(ServerInfo_User::IsAdmin)) {
            return QColor(245, 158, 11);
        }
        if (level.testFlag(ServerInfo_User::IsModerator)) {
            return QColor(59, 130, 246);
        }
        if (level.testFlag(ServerInfo_User::IsJudge)) {
            return QColor(168, 85, 247);
        }
        return QColor(100, 116, 139);
    }();

    const int ax = LeftPad;
    const int ay = rect.height() - AvatarSize - 10;
    const QRect ar(ax, ay, AvatarSize, AvatarSize);

    QPainterPath clip;
    clip.addEllipse(ar);
    p.save();
    p.setClipPath(clip);

    if (!avatar.isNull()) {
        p.drawPixmap(ar, avatar.scaled(ar.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(UserListPainter::blend(accent, style.base, dark ? 0.45 : 0.72));
        p.drawEllipse(ar);
        const QPixmap pawn =
            UserLevelPixmapGenerator::generatePixmap(AvatarPawnSize, level, user.pawn_colors(), false, privLevel);
        p.drawPixmap(ar.center().x() - AvatarPawnSize / 2, ar.center().y() - AvatarPawnSize / 2, pawn);
    }
    p.restore();

    // Status ring
    p.setPen(QPen(online ? QColor(34, 197, 94) : style.ringOffline, 2.5));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(ar).adjusted(-1.25, -1.25, 1.25, 1.25));

    // ── Username + badge ──────────────────────────────────────────────────────
    const int tx = ax + AvatarSize + AvatarToTextGap;
    const int tw = rect.width() - tx - 8;

    QFont nf = font();
    nf.setBold(true);
    nf.setPointSizeF(nf.pointSizeF() * 1.12);
    p.setFont(nf);
    p.setPen(online ? style.textOnline : style.textOffline);
    p.drawText(QRect(tx, ay, tw, AvatarSize / 2 + 4), Qt::AlignBottom | Qt::AlignLeft,
               QFontMetrics(nf).elidedText(userName, Qt::ElideRight, tw));

    // Level / priv badge
    struct
    {
        QString text;
        QColor color;
    } badge;
    if (level.testFlag(ServerInfo_User::IsAdmin)) {
        badge = {"ADMIN", QColor(245, 158, 11)};
    } else if (level.testFlag(ServerInfo_User::IsModerator)) {
        badge = {"MOD", QColor(59, 130, 246)};
    } else if (level.testFlag(ServerInfo_User::IsJudge)) {
        badge = {"JUDGE", QColor(168, 85, 247)};
    } else if (privLevel == "VIP") {
        badge = {"VIP", QColor(20, 184, 166)};
    } else if (privLevel == "DONATOR") {
        badge = {"DONATOR", QColor(249, 115, 22)};
    }

    if (!badge.text.isEmpty()) {
        QFont bf = font();
        bf.setPointSizeF(bf.pointSizeF() * 0.70);
        bf.setBold(true);
        p.setFont(bf);
        const QFontMetrics bfm(bf);
        const int bw = bfm.horizontalAdvance(badge.text) + 10;
        const QRect br(tx, ay + AvatarSize / 2 + 6, bw, 15);
        p.setPen(Qt::NoPen);
        p.setBrush(UserListPainter::blend(badge.color, style.base, dark ? 0.55 : 0.78));
        p.drawRoundedRect(br, 3, 3);
        p.setPen(dark ? UserListPainter::blend(badge.color, Qt::white, 0.5)
                      : UserListPainter::blend(badge.color, Qt::black, 0.35));
        p.drawText(br, Qt::AlignCenter, badge.text);
    }

    // The painter font at this point depends on whether a badge was drawn
    // (badge font vs username font), so pin an explicit font for the pill.
    p.setFont(font());

    // Only show the attribution when there is actually art on screen: on a
    // cache miss cardArt is null and the pill would float over the plain
    // header with no art behind it.
    if (!cardArt.isNull()) {
        paintArtAttribution(p, rect, attribution, Qt::AlignRight | Qt::AlignBottom, 0.8);
    }
}

// ── UserInfoPopup ─────────────────────────────────────────────────────────────

UserInfoPopup::UserInfoPopup(TabSupervisor *_ts,
                             AbstractClient *_client,
                             const QMap<QString, QPixmap> *_avatarCache,
                             const QMap<QString, QPixmap> *_cardArtCache,
                             const QMap<QString, CardArtParams> *_cardArtParamsMap,
                             QWidget *parent)
    : QFrame(parent, Qt::Tool | Qt::FramelessWindowHint), ts(_ts), client(_client), avatarCache(_avatarCache),
      cardArtCache(_cardArtCache), cardArtParamsMap(_cardArtParamsMap)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedWidth(PopupWidth);
    setFrameShape(QFrame::NoFrame);
    buildUi();

    // Restyle the popup chrome when the theme or its color scheme changes.
    if (themeManager) {
        connect(themeManager, &ThemeManager::themeChanged, this, &UserInfoPopup::applyTheme);
    }
}

void UserInfoPopup::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header
    header = new UserInfoHeaderWidget(this);
    root->addWidget(header);

    // Action area, rebuilt per user
    actionArea = new QWidget(this);
    root->addWidget(actionArea);

    // Thin separator
    separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    root->addWidget(separator);

    // Games header row
    auto *gh = new QHBoxLayout;
    gh->setContentsMargins(10, 4, 8, 2);
    gamesLabel = new QLabel(tr("Games"), this);
    gh->addWidget(gamesLabel);
    gh->addStretch();
    refreshBtn = new QPushButton(QStringLiteral("↻"), this);
    refreshBtn->setFixedSize(20, 20);
    refreshBtn->setFlat(true);
    connect(refreshBtn, &QPushButton::clicked, this, &UserInfoPopup::refreshGames);
    gh->addWidget(refreshBtn);
    root->addLayout(gh);

    // Status label
    gamesStatus = new QLabel(this);
    gamesStatus->setAlignment(Qt::AlignCenter);
    root->addWidget(gamesStatus);

    // Games list
    gamesModel = new QStandardItemModel(this);
    gamesView = new QListView(this);
    gamesView->setModel(gamesModel);
    gamesView->setItemDelegate(new PopupGameDelegate(gamesView));
    gamesView->setFrameShape(QFrame::NoFrame);
    gamesView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gamesView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    gamesView->setMaximumHeight(220);
    gamesView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(gamesView, &QListView::customContextMenuRequested, this, &UserInfoPopup::onGamesContextMenu);

    root->addWidget(gamesView);

    // Close button, positioned absolutely in the top right corner
    closeBtn = new QPushButton(QStringLiteral("✕"), this);
    closeBtn->setFixedSize(22, 22);
    closeBtn->setFlat(true);
    connect(closeBtn, &QPushButton::clicked, this, &UserInfoPopup::closeRequested);

    applyTheme();
}

void UserInfoPopup::applyTheme()
{
    const bool dark = themeManager && themeManager->isDarkModeActive();
    theme = PopupTheme::fromPalette(qApp->palette(), dark);

    setStyleSheet(QStringLiteral("UserInfoPopup {"
                                 "  background:%1;"
                                 "  border:1px solid %2;"
                                 "  border-radius:8px;"
                                 "}")
                      .arg(colorStr(theme.bg), colorStr(theme.border)));

    actionArea->setStyleSheet(QStringLiteral("background:%1;").arg(colorStr(theme.bg)));

    separator->setStyleSheet(QStringLiteral("color:%1; margin: 0 8px;").arg(colorStr(theme.border)));

    gamesLabel->setStyleSheet(QStringLiteral("color:%1; font-size:11px; font-weight:bold; background:transparent;")
                                  .arg(colorStr(theme.subText)));

    refreshBtn->setStyleSheet(QStringLiteral("QPushButton{color:%1;border:none;font-size:14px;background:transparent;}"
                                             "QPushButton:hover{color:%2;}")
                                  .arg(colorStr(theme.subText), colorStr(theme.text)));

    gamesStatus->setStyleSheet(QStringLiteral("color:%1; font-size:11px; padding:10px; background:transparent;")
                                   .arg(colorStr(theme.statusText)));

    gamesView->setStyleSheet(QStringLiteral("QListView{background:%1;border:none;}"
                                            "QListView::item:selected{background:%2;}")
                                 .arg(colorStr(theme.gamesRow), colorStr(theme.gamesSelected)));

    closeBtn->setStyleSheet(
        QStringLiteral("QPushButton{background:%1;color:%2;"
                       "border:none;border-radius:11px;font-size:10px;}"
                       "QPushButton:hover{color:%3;background:%4;}")
            .arg(colorStr(theme.closeBg), colorStr(theme.subText), colorStr(theme.text), colorStr(theme.closeHover)));

    header->update();
}

// ── Action button factory ─────────────────────────────────────────────────────

static QPushButton *makeBtn(const QString &label, const QString &tip, QWidget *p, const PopupTheme &t)
{
    auto *b = new QPushButton(label, p);
    b->setToolTip(tip);
    b->setFixedHeight(26);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    b->setStyleSheet(QStringLiteral("QPushButton{"
                                    "  background:%1;color:%2;border:1px solid %3;"
                                    "  border-radius:4px;font-size:11px;padding:0 4px;"
                                    "}"
                                    "QPushButton:hover{background:%4;color:%5;}"
                                    "QPushButton:pressed{background:%6;}"
                                    "QPushButton:disabled{color:%7;border-color:%3;}")
                         .arg(colorStr(t.buttonBg), colorStr(t.text), colorStr(t.buttonBorder), colorStr(t.buttonHover),
                              colorStr(t.text), colorStr(t.buttonPressed), colorStr(t.buttonDisabled)));
    return b;
}

void UserInfoPopup::rebuildActionButtons(const ServerInfo_User &userInfo, bool online, bool isBuddy, bool isIgnored)
{
    // Clear previous contents
    delete actionArea->layout();
    const auto old = actionArea->findChildren<QPushButton *>(QString{}, Qt::FindDirectChildrenOnly);
    for (auto *w : old) {
        w->deleteLater();
    }

    const QString name = QString::fromStdString(userInfo.name());
    const auto ownLevel = UserLevelFlags(ts->getUserInfo()->user_level());
    const bool isSelf = (name == QString::fromStdString(ts->getUserInfo()->name()));
    const bool isMod = ownLevel.testFlag(ServerInfo_User::IsModerator);
    const bool isAdmin = ownLevel.testFlag(ServerInfo_User::IsAdmin);
    const auto their = UserLevelFlags(userInfo.user_level());
    const bool isReg = their.testFlag(ServerInfo_User::IsRegistered);

    auto *grid = new QGridLayout(actionArea);
    grid->setContentsMargins(8, 6, 8, 6);
    grid->setSpacing(4);

    int row = 0, col = 0;
    const int cols = 3;
    auto add = [&](QPushButton *btn) {
        grid->addWidget(btn, row, col);
        if (++col == cols) {
            col = 0;
            ++row;
        }
    };

    // ── Always visible ────────────────────────────────────────────────────────
    auto *chat = makeBtn(tr("Chat"), tr("Open private chat"), actionArea, theme);
    chat->setEnabled(!isSelf && online);
    connect(chat, &QPushButton::clicked, this, [this, name] { emit chatRequested(name); });
    add(chat);

    auto *prof = makeBtn(tr("Profile"), tr("View user profile"), actionArea, theme);
    connect(prof, &QPushButton::clicked, this, [this, name] { emit detailsRequested(name); });
    add(prof);

    auto *games = makeBtn(tr("Games"), tr("Show this user's games"), actionArea, theme);
    games->setEnabled(!isSelf && online);
    connect(games, &QPushButton::clicked, this, [this, name] { emit showGamesRequested(name); });
    add(games);

    // ── Invite (only while the inviter has a joinable game for this user) ────
    if (!isSelf && online && gameInviteAvailable && gameInviteAvailable(name)) {
        auto *invite = makeBtn(tr("Invite"), tr("Invite to your game"), actionArea, theme);
        connect(invite, &QPushButton::clicked, this, [this, name] { emit inviteRequested(name); });
        add(invite);
    }

    // ── Buddy / ignore (registered users only) ────────────────────────────────
    if (!isSelf && isReg) {
        if (isBuddy) {
            auto *b = makeBtn(tr("− Buddy"), tr("Remove from buddy list"), actionArea, theme);
            connect(b, &QPushButton::clicked, this, [this, name] { emit removeBuddyRequested(name); });
            add(b);
        } else {
            auto *b = makeBtn(tr("+ Buddy"), tr("Add to buddy list"), actionArea, theme);
            connect(b, &QPushButton::clicked, this, [this, name] { emit addBuddyRequested(name); });
            add(b);
        }
        if (isIgnored) {
            auto *b = makeBtn(tr("− Ignore"), tr("Remove from ignore list"), actionArea, theme);
            connect(b, &QPushButton::clicked, this, [this, name] { emit removeIgnoreRequested(name); });
            add(b);
        } else {
            auto *b = makeBtn(tr("+ Ignore"), tr("Add to ignore list"), actionArea, theme);
            connect(b, &QPushButton::clicked, this, [this, name] { emit addIgnoreRequested(name); });
            add(b);
        }
    }

    // ── Moderator actions ─────────────────────────────────────────────────────
    if (!isSelf && (isMod || isAdmin)) {
        if (col != 0) {
            ++row;
            col = 0;
        } // start mod section on a fresh row

        auto *ban = makeBtn(tr("Ban"), tr("Ban from server"), actionArea, theme);
        auto *warn = makeBtn(tr("Warn"), tr("Warn user"), actionArea, theme);
        auto *bLog = makeBtn(tr("Ban log"), tr("View ban history"), actionArea, theme);
        auto *wLog = makeBtn(tr("Warn log"), tr("View warning history"), actionArea, theme);
        connect(ban, &QPushButton::clicked, this, [this, name] { emit banRequested(name); });
        connect(warn, &QPushButton::clicked, this, [this, name] { emit warnRequested(name); });
        connect(bLog, &QPushButton::clicked, this, [this, name] { emit banHistoryRequested(name); });
        connect(wLog, &QPushButton::clicked, this, [this, name] { emit warnHistoryRequested(name); });
        add(ban);
        add(warn);
        add(bLog);
        add(wLog);
    }

    // ── Admin actions ─────────────────────────────────────────────────────────
    if (!isSelf && isAdmin) {
        auto *notes = makeBtn(tr("Notes"), tr("View admin notes"), actionArea, theme);
        connect(notes, &QPushButton::clicked, this, [this, name] { emit adminNotesRequested(name); });
        add(notes);

        if (their.testFlag(ServerInfo_User::IsModerator)) {
            auto *b = makeBtn(tr("− Mod"), tr("Demote from moderator"), actionArea, theme);
            connect(b, &QPushButton::clicked, this, [this, name] { emit demoteFromModRequested(name); });
            add(b);
        } else if (isReg) {
            auto *b = makeBtn(tr("+ Mod"), tr("Promote to moderator"), actionArea, theme);
            connect(b, &QPushButton::clicked, this, [this, name] { emit promoteToModRequested(name); });
            add(b);
        }
        if (their.testFlag(ServerInfo_User::IsJudge)) {
            auto *b = makeBtn(tr("− Judge"), tr("Demote from judge"), actionArea, theme);
            connect(b, &QPushButton::clicked, this, [this, name] { emit demoteFromJudgeRequested(name); });
            add(b);
        } else if (isReg) {
            auto *b = makeBtn(tr("+ Judge"), tr("Promote to judge"), actionArea, theme);
            connect(b, &QPushButton::clicked, this, [this, name] { emit promoteToJudgeRequested(name); });
            add(b);
        }
    }

    actionArea->adjustSize();
}

void UserInfoPopup::updateActionButtons(const ServerInfo_User &userInfo, bool online, bool isBuddy, bool isIgnored)
{
    rebuildActionButtons(userInfo, online, isBuddy, isIgnored);
    adjustSize();
}

void UserInfoPopup::onGamesContextMenu(const QPoint &pos)
{
    const QModelIndex idx = gamesView->indexAt(pos);
    if (!idx.isValid()) {
        return;
    }

    const QVariant var = idx.data(PopupRoles::GameData);
    if (!var.isValid()) {
        return;
    }
    const ServerInfo_Game game = var.value<ServerInfo_Game>();

    QMenu menu(this);
    menu.setStyleSheet(
        QStringLiteral("QMenu{background:%1;color:%2;border:1px solid %3;border-radius:4px;}"
                       "QMenu::item:selected{background:%4;}")
            .arg(colorStr(theme.bg), colorStr(theme.text), colorStr(theme.border), colorStr(theme.buttonHover)));

    const bool canJoin = !game.started() && game.player_count() < game.max_players();
    QAction *join = menu.addAction(tr("Join game"));
    join->setEnabled(canJoin);

    QAction *spec = nullptr;
    if (game.spectators_allowed()) {
        spec = menu.addAction(tr("Spectate"));
    }

    const QAction *chosen = menu.exec(gamesView->viewport()->mapToGlobal(pos));
    if (!chosen) {
        return;
    }

    if (chosen == join) {
        emit joinGameRequested(game.game_id(), game.room_id(), false);
    } else if (spec && chosen == spec) {
        emit joinGameRequested(game.game_id(), game.room_id(), true);
    }
}

// ── showForUser ───────────────────────────────────────────────────────────────

void UserInfoPopup::refreshHeader()
{
    if (currentUser.isEmpty()) {
        return;
    }

    const QPixmap avatar = avatarCache ? avatarCache->value(currentUser) : QPixmap{};
    const CardArtParams params = (cardArtParamsMap && cardArtParamsMap->contains(currentUser))
                                     ? cardArtParamsMap->value(currentUser)
                                     : CardArtParams{};
    const QString artKey = currentUser + u'|' + params.cardName + u'|' + params.cardProviderId;
    const QPixmap cardArt = (cardArtCache && !params.cardName.isEmpty()) ? cardArtCache->value(artKey) : QPixmap{};
    header->setUserData(currentUserInfo, currentOnline, avatar, cardArt, params);
}

void UserInfoPopup::showForUser(const QString &userName,
                                const ServerInfo_User &userInfo,
                                bool online,
                                bool isBuddy,
                                bool isIgnored)
{
    currentUser = userName;
    currentUserInfo = userInfo;
    currentOnline = online;

    // Header
    refreshHeader();

    // Actions
    rebuildActionButtons(userInfo, online, isBuddy, isIgnored);

    // Games list reset
    gamesModel->clear();
    gamesView->hide();
    gamesStatus->setText(tr("Loading games…"));
    gamesStatus->show();

    // Close button, top right corner, above everything
    closeBtn->move(PopupWidth - closeBtn->width() - 6, 6);
    closeBtn->raise();

    adjustSize();
    fetchGames();
}

// ── Games fetch ───────────────────────────────────────────────────────────────

void UserInfoPopup::fetchGames()
{
    if (!client || currentUser.isEmpty()) {
        return;
    }

    Command_GetGamesOfUser cmd;
    cmd.set_user_name(currentUser.toStdString());

    const QString snapshot = currentUser;
    PendingCommand *pend = client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this,
            [this, snapshot](const Response &r) { onGamesReceived(r, snapshot); });
    client->sendCommand(pend);
}

void UserInfoPopup::onGamesReceived(const Response &r, const QString &forUser)
{
    if (forUser != currentUser) {
        return; // stale response, different user showing now
    }

    gamesModel->clear();

    if (r.response_code() != Response::RespOk) {
        gamesStatus->setText(tr("Could not load games."));
        gamesStatus->show();
        gamesView->hide();
        return;
    }

    const auto &resp = r.GetExtension(Response_GetGamesOfUser::ext);
    if (resp.game_list_size() == 0) {
        gamesStatus->setText(tr("No active games."));
        gamesStatus->show();
        gamesView->hide();
        return;
    }

    for (int i = 0; i < resp.game_list_size(); ++i) {
        auto *item = new QStandardItem;
        item->setData(QVariant::fromValue(resp.game_list(i)), PopupRoles::GameData);
        item->setEditable(false);
        gamesModel->appendRow(item);
    }

    gamesStatus->hide();
    gamesView->show();

    // Fit exactly to the number of visible rows, scroll when more than 5
    constexpr int rowH = 38; // must match PopupGameDelegate::sizeHint
    constexpr int maxRows = 5;
    const int count = gamesModel->rowCount();
    const int visible = qMin(count, maxRows);
    gamesView->setFixedHeight(visible * rowH + 2);
    gamesView->setVerticalScrollBarPolicy(count > maxRows ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);

    adjustSize();
}

void UserInfoPopup::refreshGames()
{
    gamesModel->clear();
    gamesView->hide();
    gamesStatus->setText(tr("Loading games…"));
    gamesStatus->show();
    fetchGames();
}

// ── Mouse events ──────────────────────────────────────────────────────────────

void UserInfoPopup::enterEvent(QEnterEvent *e)
{
    QFrame::enterEvent(e);
    emit mouseEnteredPopup();
}

void UserInfoPopup::leaveEvent(QEvent *e)
{
    QFrame::leaveEvent(e);
    emit mouseLeftPopup();
}
