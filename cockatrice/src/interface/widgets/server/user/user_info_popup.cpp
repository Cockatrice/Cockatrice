#include "user_info_popup.h"

#include "../../interface/pixel_map_generator.h"
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
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/response_get_games_of_user.pb.h>
#include <libcockatrice/protocol/pending_command.h>

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

        p->fillRect(rect, selected ? QColor(35, 45, 62) : QColor(14, 18, 26));

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
        p->setPen(QColor(205, 215, 230));
        const int textX = rect.left() + 26;
        const int countW = 52;
        const int titleW = rect.width() - textX - countW - 6;
        p->drawText(QRect(textX, rect.top(), titleW, rect.height()), Qt::AlignVCenter | Qt::AlignLeft,
                    QFontMetrics(tf).elidedText(QString::fromStdString(game.description()), Qt::ElideRight, titleW));

        // Player count
        const bool full = game.player_count() >= game.max_players();
        p->setFont(option.font);
        p->setPen(full ? QColor(249, 115, 22) : QColor(110, 128, 150));
        p->drawText(QRect(rect.right() - countW - 4, rect.top(), countW, rect.height()),
                    Qt::AlignVCenter | Qt::AlignRight,
                    QStringLiteral("%1/%2").arg(game.player_count()).arg(game.max_players()));

        // Row separator
        p->setPen(QColor(24, 32, 44));
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

void UserInfoHeaderWidget::setUserData(const ServerInfo_User &user,
                                       bool online,
                                       const QPixmap &avatar,
                                       const QPixmap &cardArt,
                                       const CardArtParams &params)
{
    m_user = user;
    m_online = online;
    m_avatar = avatar;
    m_cardArt = cardArt;
    m_params = params;
    update();
}

void UserInfoHeaderWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    const QRect rect = this->rect();
    const UserLevelFlags level(m_user.user_level());
    const QString userName = QString::fromStdString(m_user.name());
    const QString privLevel = QString::fromStdString(m_user.privlevel());

    // Dark base
    p.fillRect(rect, QColor(14, 18, 26));

    // ── Card art background ───────────────────────────────────────────────────
    if (!m_cardArt.isNull()) {
        const int w = rect.width();
        const int h = rect.height();
        const int mL = qRound(w * m_params.marginPctL);
        const int mR = qRound(w * m_params.marginPctR);
        const int dW = w - mL - mR;

        const double base = qMax(double(dW) / m_cardArt.width(), double(h) / m_cardArt.height());
        const double scale = base * m_params.zoom;
        const int sW = qRound(m_cardArt.width() * scale);
        const int sH = qRound(m_cardArt.height() * scale);

        const QPixmap scaled = m_cardArt.scaled(sW, sH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        const int srcX = (sW - dW) / 2;
        const int srcY = qBound(0, qRound((sH - h) * m_params.verticalOffset), qMax(0, sH - h));

        QImage img = scaled.copy(srcX, srcY, dW, h).toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
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

    // Bottom gradient overlay so avatar and text are always legible
    {
        QLinearGradient ov(0, 0, 0, rect.height());
        ov.setColorAt(0.0, QColor(14, 18, 26, 0));
        ov.setColorAt(0.55, QColor(14, 18, 26, 110));
        ov.setColorAt(1.0, QColor(14, 18, 26, 230));
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

    if (!m_avatar.isNull()) {
        p.drawPixmap(ar, m_avatar.scaled(ar.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(accent.darker(200));
        p.drawEllipse(ar);
        const QPixmap pawn =
            UserLevelPixmapGenerator::generatePixmap(AvatarPawnSize, level, m_user.pawn_colors(), false, privLevel);
        p.drawPixmap(ar.center().x() - AvatarPawnSize / 2, ar.center().y() - AvatarPawnSize / 2, pawn);
    }
    p.restore();

    // Status ring
    p.setPen(QPen(m_online ? QColor(34, 197, 94) : QColor(70, 80, 95), 2.5));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(ar).adjusted(-1.25, -1.25, 1.25, 1.25));

    // ── Username + badge ──────────────────────────────────────────────────────
    const int tx = ax + AvatarSize + AvatarToTextGap;
    const int tw = rect.width() - tx - 8;

    QFont nf = font();
    nf.setBold(true);
    nf.setPointSizeF(nf.pointSizeF() * 1.12);
    p.setFont(nf);
    p.setPen(m_online ? QColor(220, 228, 240) : QColor(90, 100, 115));
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
        p.setBrush(badge.color.darker(160));
        p.drawRoundedRect(br, 3, 3);
        p.setPen(badge.color.lighter(150));
        p.drawText(br, Qt::AlignCenter, badge.text);
    }
}

// ── UserInfoPopup ─────────────────────────────────────────────────────────────

UserInfoPopup::UserInfoPopup(TabSupervisor *ts,
                             AbstractClient *client,
                             const QMap<QString, QPixmap> *avatarCache,
                             const QMap<QString, QPixmap> *cardArtCache,
                             const QMap<QString, CardArtParams> *cardArtParamsMap,
                             QWidget *parent)
    : QFrame(parent, Qt::Tool | Qt::FramelessWindowHint), m_ts(ts), m_client(client), m_avatarCache(avatarCache),
      m_cardArtCache(cardArtCache), m_cardArtParamsMap(cardArtParamsMap)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedWidth(PopupWidth);
    setFrameShape(QFrame::NoFrame);
    buildUi();
}

void UserInfoPopup::buildUi()
{
    setStyleSheet(QStringLiteral("UserInfoPopup {"
                                 "  background:#0e1218;"
                                 "  border:1px solid #1e2838;"
                                 "  border-radius:8px;"
                                 "}"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header
    m_header = new UserInfoHeaderWidget(this);
    root->addWidget(m_header);

    // Action area — rebuilt per user
    m_actionArea = new QWidget(this);
    m_actionArea->setStyleSheet(QStringLiteral("background:#0e1218;"));
    root->addWidget(m_actionArea);

    // Thin separator
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QStringLiteral("color:#1a2434; margin: 0 8px;"));
    root->addWidget(sep);

    // Games header row
    auto *gh = new QHBoxLayout;
    gh->setContentsMargins(10, 4, 8, 2);
    auto *gl = new QLabel(tr("Games"), this);
    gl->setStyleSheet(QStringLiteral("color:#6882a0; font-size:11px; font-weight:bold; background:transparent;"));
    gh->addWidget(gl);
    gh->addStretch();
    m_refreshBtn = new QPushButton(QStringLiteral("↻"), this);
    m_refreshBtn->setFixedSize(20, 20);
    m_refreshBtn->setFlat(true);
    m_refreshBtn->setStyleSheet(
        QStringLiteral("QPushButton{color:#6882a0;border:none;font-size:14px;background:transparent;}"
                       "QPushButton:hover{color:white;}"));
    connect(m_refreshBtn, &QPushButton::clicked, this, &UserInfoPopup::refreshGames);
    gh->addWidget(m_refreshBtn);
    root->addLayout(gh);

    // Status label
    m_gamesStatus = new QLabel(this);
    m_gamesStatus->setAlignment(Qt::AlignCenter);
    m_gamesStatus->setStyleSheet(
        QStringLiteral("color:#3a4a5e; font-size:11px; padding:10px; background:transparent;"));
    root->addWidget(m_gamesStatus);

    // Games list
    m_gamesModel = new QStandardItemModel(this);
    m_gamesView = new QListView(this);
    m_gamesView->setModel(m_gamesModel);
    m_gamesView->setItemDelegate(new PopupGameDelegate(m_gamesView));
    m_gamesView->setFrameShape(QFrame::NoFrame);
    m_gamesView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gamesView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_gamesView->setMaximumHeight(220);
    m_gamesView->setStyleSheet(QStringLiteral("QListView{background:#0e1218;border:none;}"
                                              "QListView::item:selected{background:#232e42;}"));
    root->addWidget(m_gamesView);

    // Close button — positioned absolutely in the top-right corner
    m_closeBtn = new QPushButton(QStringLiteral("✕"), this);
    m_closeBtn->setFixedSize(22, 22);
    m_closeBtn->setFlat(true);
    m_closeBtn->setStyleSheet(QStringLiteral("QPushButton{background:rgba(14,18,26,180);color:#607080;"
                                             "border:none;border-radius:11px;font-size:10px;}"
                                             "QPushButton:hover{color:white;background:rgba(200,50,50,200);}"));
    connect(m_closeBtn, &QPushButton::clicked, this, &UserInfoPopup::closeRequested);
}

// ── Action button factory ─────────────────────────────────────────────────────

static QPushButton *makeBtn(const QString &label, const QString &tip, QWidget *p)
{
    auto *b = new QPushButton(label, p);
    b->setToolTip(tip);
    b->setFixedHeight(26);
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    b->setStyleSheet(QStringLiteral("QPushButton{"
                                    "  background:#192030;color:#b8c8de;border:1px solid #263040;"
                                    "  border-radius:4px;font-size:11px;padding:0 4px;"
                                    "}"
                                    "QPushButton:hover{background:#223050;color:white;}"
                                    "QPushButton:pressed{background:#162030;}"
                                    "QPushButton:disabled{color:#384858;border-color:#192030;}"));
    return b;
}

void UserInfoPopup::rebuildActionButtons(const ServerInfo_User &userInfo, bool online, bool isBuddy, bool isIgnored)
{
    // Clear previous contents
    delete m_actionArea->layout();
    const auto old = m_actionArea->findChildren<QPushButton *>(QString{}, Qt::FindDirectChildrenOnly);
    for (auto *w : old) {
        w->deleteLater();
    }

    const QString name = QString::fromStdString(userInfo.name());
    const auto ownLevel = UserLevelFlags(m_ts->getUserInfo()->user_level());
    const bool isSelf = (name == QString::fromStdString(m_ts->getUserInfo()->name()));
    const bool isMod = ownLevel.testFlag(ServerInfo_User::IsModerator);
    const bool isAdmin = ownLevel.testFlag(ServerInfo_User::IsAdmin);
    const auto their = UserLevelFlags(userInfo.user_level());
    const bool isReg = their.testFlag(ServerInfo_User::IsRegistered);

    auto *grid = new QGridLayout(m_actionArea);
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
    auto *chat = makeBtn(tr("Chat"), tr("Open private chat"), m_actionArea);
    chat->setEnabled(!isSelf && online);
    connect(chat, &QPushButton::clicked, this, [this, name] { emit chatRequested(name); });
    add(chat);

    auto *prof = makeBtn(tr("Profile"), tr("View user profile"), m_actionArea);
    connect(prof, &QPushButton::clicked, this, [this, name] { emit detailsRequested(name); });
    add(prof);

    auto *games = makeBtn(tr("Games"), tr("Show this user's games"), m_actionArea);
    games->setEnabled(!isSelf && online);
    connect(games, &QPushButton::clicked, this, [this, name] { emit showGamesRequested(name); });
    add(games);

    // ── Buddy / ignore (registered users only) ────────────────────────────────
    if (!isSelf && isReg) {
        if (isBuddy) {
            auto *b = makeBtn(tr("− Buddy"), tr("Remove from buddy list"), m_actionArea);
            connect(b, &QPushButton::clicked, this, [this, name] { emit removeBuddyRequested(name); });
            add(b);
        } else {
            auto *b = makeBtn(tr("+ Buddy"), tr("Add to buddy list"), m_actionArea);
            connect(b, &QPushButton::clicked, this, [this, name] { emit addBuddyRequested(name); });
            add(b);
        }
        if (isIgnored) {
            auto *b = makeBtn(tr("− Ignore"), tr("Remove from ignore list"), m_actionArea);
            connect(b, &QPushButton::clicked, this, [this, name] { emit removeIgnoreRequested(name); });
            add(b);
        } else {
            auto *b = makeBtn(tr("+ Ignore"), tr("Add to ignore list"), m_actionArea);
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

        auto *ban = makeBtn(tr("Ban"), tr("Ban from server"), m_actionArea);
        auto *warn = makeBtn(tr("Warn"), tr("Warn user"), m_actionArea);
        auto *bLog = makeBtn(tr("Ban log"), tr("View ban history"), m_actionArea);
        auto *wLog = makeBtn(tr("Warn log"), tr("View warning history"), m_actionArea);
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
        auto *notes = makeBtn(tr("Notes"), tr("View admin notes"), m_actionArea);
        connect(notes, &QPushButton::clicked, this, [this, name] { emit adminNotesRequested(name); });
        add(notes);

        if (their.testFlag(ServerInfo_User::IsModerator)) {
            auto *b = makeBtn(tr("− Mod"), tr("Demote from moderator"), m_actionArea);
            connect(b, &QPushButton::clicked, this, [this, name] { emit demoteFromModRequested(name); });
            add(b);
        } else if (isReg) {
            auto *b = makeBtn(tr("+ Mod"), tr("Promote to moderator"), m_actionArea);
            connect(b, &QPushButton::clicked, this, [this, name] { emit promoteToModRequested(name); });
            add(b);
        }
        if (their.testFlag(ServerInfo_User::IsJudge)) {
            auto *b = makeBtn(tr("− Judge"), tr("Demote from judge"), m_actionArea);
            connect(b, &QPushButton::clicked, this, [this, name] { emit demoteFromJudgeRequested(name); });
            add(b);
        } else if (isReg) {
            auto *b = makeBtn(tr("+ Judge"), tr("Promote to judge"), m_actionArea);
            connect(b, &QPushButton::clicked, this, [this, name] { emit promoteToJudgeRequested(name); });
            add(b);
        }
    }

    m_actionArea->adjustSize();
}

// ── showForUser ───────────────────────────────────────────────────────────────

void UserInfoPopup::showForUser(const QString &userName,
                                const ServerInfo_User &userInfo,
                                bool online,
                                bool isBuddy,
                                bool isIgnored)
{
    m_currentUser = userName;
    m_currentUserInfo = userInfo;
    m_currentOnline = online;

    // Header
    const QPixmap avatar = m_avatarCache ? m_avatarCache->value(userName) : QPixmap{};
    const CardArtParams params = (m_cardArtParamsMap && m_cardArtParamsMap->contains(userName))
                                     ? m_cardArtParamsMap->value(userName)
                                     : CardArtParams{};
    const QString artKey = userName + u'|' + params.cardName;
    const QPixmap cardArt = (m_cardArtCache && !params.cardName.isEmpty()) ? m_cardArtCache->value(artKey) : QPixmap{};
    m_header->setUserData(userInfo, online, avatar, cardArt, params);

    // Actions
    rebuildActionButtons(userInfo, online, isBuddy, isIgnored);

    // Games list reset
    m_gamesModel->clear();
    m_gamesView->hide();
    m_gamesStatus->setText(tr("Loading games…"));
    m_gamesStatus->show();

    // Close button — top-right corner, above everything
    m_closeBtn->move(PopupWidth - m_closeBtn->width() - 6, 6);
    m_closeBtn->raise();

    adjustSize();
    fetchGames();
}

// ── Games fetch ───────────────────────────────────────────────────────────────

void UserInfoPopup::fetchGames()
{
    if (!m_client || m_currentUser.isEmpty()) {
        return;
    }

    Command_GetGamesOfUser cmd;
    cmd.set_user_name(m_currentUser.toStdString());

    const QString snapshot = m_currentUser;
    PendingCommand *pend = m_client->prepareSessionCommand(cmd);
    connect(pend, &PendingCommand::finished, this,
            [this, snapshot](const Response &r) { onGamesReceived(r, snapshot); });
    m_client->sendCommand(pend);
}

void UserInfoPopup::onGamesReceived(const Response &r, const QString &forUser)
{
    if (forUser != m_currentUser) {
        return; // stale response — different user showing now
    }

    m_gamesModel->clear();

    if (r.response_code() != Response::RespOk) {
        m_gamesStatus->setText(tr("Could not load games."));
        m_gamesStatus->show();
        m_gamesView->hide();
        return;
    }

    const auto &resp = r.GetExtension(Response_GetGamesOfUser::ext);
    if (resp.game_list_size() == 0) {
        m_gamesStatus->setText(tr("No active games."));
        m_gamesStatus->show();
        m_gamesView->hide();
        return;
    }

    for (int i = 0; i < resp.game_list_size(); ++i) {
        auto *item = new QStandardItem;
        item->setData(QVariant::fromValue(resp.game_list(i)), PopupRoles::GameData);
        item->setEditable(false);
        m_gamesModel->appendRow(item);
    }

    m_gamesStatus->hide();
    m_gamesView->show();
    adjustSize();
}

void UserInfoPopup::refreshGames()
{
    m_gamesModel->clear();
    m_gamesView->hide();
    m_gamesStatus->setText(tr("Loading games…"));
    m_gamesStatus->show();
    fetchGames();
}

// ── Mouse events ──────────────────────────────────────────────────────────────

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void UserInfoPopup::enterEvent(QEnterEvent *e)
{
    QFrame::enterEvent(e);
    emit mouseEnteredPopup();
}
#else
void UserInfoPopup::enterEvent(QEvent *e)
{
    QFrame::enterEvent(e);
    emit mouseEnteredPopup();
}
#endif
void UserInfoPopup::leaveEvent(QEvent *e)
{
    QFrame::leaveEvent(e);
    emit mouseLeftPopup();
}