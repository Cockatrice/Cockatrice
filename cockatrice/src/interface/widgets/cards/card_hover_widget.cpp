#include "card_hover_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/card_counter_settings.h"
#include "../../../game_graphics/board/card_item.h"
#include "../../../game_graphics/game_scene.h"
#include "../../../game_graphics/player/player_graphics_item.h"
#include "../../pixel_map_generator.h"
#include "card_info_picture_widget.h"
#include "card_info_text_widget.h"

#include <QAbstractAnimation>
#include <QApplication>
#include <QButtonGroup>
#include <QEasingCurve>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollArea>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/game/abstract_game.h>
#include <libcockatrice/game/board/card_state.h>
#include <libcockatrice/game/player/card_menu_action_type.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_info.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/player/player_manager.h>
#include <libcockatrice/game/zones/card_zone_logic.h>
#include <libcockatrice/game/zones/view_zone_logic.h>
#include <libcockatrice/utility/zone_names.h>

static constexpr int CARD_IMAGE_WIDTH = 270;
static constexpr int TAB_MIN_WIDTH = 250;
static constexpr int TAB_MAX_WIDTH = 300;
static constexpr int TAB_MIN_HEIGHT = 360;
static constexpr int NAV_BUTTON_WIDTH = 78;
static constexpr int NAV_ICON_SIZE = 24;
static constexpr int POPUP_GAP = 6;
static constexpr int FADE_DURATION_MS = 140;
static constexpr int REFRESH_DEBOUNCE_MS = 30;
static constexpr int OVERLAY_BTN_SIZE = 24;
static constexpr int OVERLAY_CORNER_RADIUS = 4;

// P/T pill: centred on the card's printed P/T box, in fractions of the picture rect.
static constexpr double PT_PILL_ANCHOR_X = 0.92;
static constexpr double PT_PILL_ANCHOR_Y = 0.94;
static constexpr int PT_PILL_EDGE_INSET = 2;
static constexpr int PT_PILL_MIN_WIDTH = 48;
static constexpr int PT_PILL_MIN_HEIGHT = 22;
static constexpr int PT_PILL_TEXT_PADDING = 4;

// Hover trigger: the bottom-right quadrant of the art, around the printed P/T box.
static constexpr double PT_HOVER_REGION_WIDTH = 0.35;
static constexpr double PT_HOVER_REGION_HEIGHT = 0.20;
// Halo around the resting pill that still counts as "on the P/T box" for opening the rows.
static constexpr int PT_OPEN_HALO = 4;

enum HoverPanelPage
{
    PAGE_INFO,
    PAGE_MAIN,
    PAGE_MOVE,
    PAGE_COUNTER,
    PAGE_TOKEN
};

CardHoverWidget::CardHoverWidget(QWidget *parent) : QFrame(parent), currentCard(nullptr), currentPlayer(nullptr)
{
    setObjectName("cardHoverWidget");
    setFrameShape(QFrame::StyledPanel);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setMouseTracking(true);
    setContentsMargins(0, 0, 0, 0);

    applyThemeStyle();

    setWindowOpacity(0.0);

    fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
    fadeIn->setDuration(FADE_DURATION_MS);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    fadeOut = new QPropertyAnimation(this, "windowOpacity", this);
    fadeOut->setDuration(FADE_DURATION_MS);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);
    connect(fadeOut, &QPropertyAnimation::finished, this, &QWidget::hide);

    refreshTimer = new QTimer(this);
    refreshTimer->setSingleShot(true);
    refreshTimer->setInterval(REFRESH_DEBOUNCE_MS);
    connect(refreshTimer, &QTimer::timeout, this, &CardHoverWidget::onCardChanged);

    createImageArea();
    createContentStack();
    createNavBar();

    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    mainLayout->addWidget(imageContainer, 0);
    mainLayout->addWidget(sideBar, 0);
    mainLayout->addWidget(contentStack, 1);
    setLayout(mainLayout);

    showPage(PAGE_INFO);
}

void CardHoverWidget::applyThemeStyle()
{
    const QPalette &pal = QApplication::palette();
    const QString window = pal.color(QPalette::Window).name();
    const QString windowText = pal.color(QPalette::WindowText).name();
    const QString base = pal.color(QPalette::Base).name();
    const QString text = pal.color(QPalette::Text).name();
    const QString mid = pal.color(QPalette::Mid).name();
    const QString highlight = pal.color(QPalette::Highlight).name();
    const QString highlightedText = pal.color(QPalette::HighlightedText).name();
    const QString disabled = pal.color(QPalette::PlaceholderText).name();

    // Opaque theme colors throughout — translucency proved too hard to read.
    // The dark scrims over the card artwork itself are kept, they sit on the image, not the panel.
    // Guard: setStyleSheet() itself emits PaletteChange, which would otherwise
    // re-enter applyThemeStyle() via changeEvent() forever (freeze, then stack overflow).
    QString sheet = QStringLiteral("QFrame#cardHoverWidget {"
                                   "  background-color: %1;"
                                   "  border: 1px solid %2;"
                                   "  border-radius: 10px;"
                                   "}"
                                   "QScrollArea { background: transparent; border: none; }"
                                   "QToolButton#cardHoverNavBtn {"
                                   "  background: transparent; color: %4;"
                                   "  border: 1px solid transparent; border-radius: 8px;"
                                   "  padding: 7px 4px; font-size: 11px;"
                                   "}"
                                   "QToolButton#cardHoverNavBtn:hover {"
                                   "  background: %3; border-color: %2;"
                                   "}"
                                   "QToolButton#cardHoverNavBtn:checked {"
                                   "  background: %5; color: %6; border-color: %5;"
                                   "}"
                                   "QLabel#cardHoverName {"
                                   "  color: %8; font-weight: bold; font-size: 12px;"
                                   "}"
                                   "QPushButton#cardHoverActionBtn {"
                                   "  background: %3; color: %4;"
                                   "  border: 1px solid %2; border-radius: 6px;"
                                   "  padding: 5px 10px; font-size: 12px; text-align: center;"
                                   "}"
                                   "QPushButton#cardHoverActionBtn:hover {"
                                   "  background: %5; color: %6; border-color: %5;"
                                   "}"
                                   "QPushButton#cardHoverActionBtn:pressed {"
                                   "  background: %5; color: %6;"
                                   "}"
                                   "QPushButton#cardHoverActionBtn:checked { background: %5; color: %6; }"
                                   "QPushButton#cardHoverActionBtn:disabled { color: %7; }"
                                   "QToolButton#cardHoverGridBtn {"
                                   "  background: %3; color: %4;"
                                   "  border: 1px solid %2; border-radius: 6px;"
                                   "  padding: 6px 4px; font-size: 11px;"
                                   "}"
                                   "QToolButton#cardHoverGridBtn:hover {"
                                   "  background: %5; color: %6; border-color: %5;"
                                   "}"
                                   "QToolButton#cardHoverGridBtn:pressed {"
                                   "  background: %5; color: %6;"
                                   "}"
                                   "QToolButton#cardHoverGridBtn:checked { background: %5; color: %6; }"
                                   "QToolButton#cardHoverGridBtn:disabled { color: %7; }"
                                   "QLabel#cardHoverSectionHeader {"
                                   "  background: %2; color: %8;"
                                   "  border-radius: 4px; padding: 3px 6px;"
                                   "  font-weight: bold; font-size: 11px;"
                                   "}"
                                   "QLabel#cardHoverNote { color: %7; font-style: italic; font-size: 11px; }"
                                   "QLineEdit#cardHoverNameEdit {"
                                   "  color: %4; background: %3; border: 1px solid %2;"
                                   "  border-radius: 3px; padding: 2px;"
                                   "  font-weight: bold; font-size: 13px;"
                                   "}"
                                   // Accent edge on the resting P/T pill: the cue that the
                                   // control rows live behind it, and the divider once they lift out.
                                   "QLabel#cardHoverPTPill {"
                                   "  border-top: 2px solid %5;"
                                   "}")
                        .arg(window, mid, base, text, highlight, highlightedText, disabled, windowText);
    if (sheet == styleSheet()) {
        return;
    }
    setStyleSheet(sheet);
}

// ---------- Overlay styling (dark scrims over the card artwork) ----------

static QString overlayBgStyle()
{
    return QStringLiteral("background-color: rgba(0, 0, 0, 180);"
                          "border-radius: %1px;")
        .arg(OVERLAY_CORNER_RADIUS);
}

/** @brief Clamps @p start so that a run of @p size stays within [@p origin, @p origin + @p extent]. */
static int clampToSpan(int origin, int extent, int size, int start)
{
    if (size >= extent) {
        return origin;
    }
    return qBound(origin, start, origin + extent - size);
}

// ---------- Image area with overlaid controls ----------

void CardHoverWidget::createImageArea()
{
    imageContainer = new QWidget(this);
    imageContainer->setObjectName("cardHoverImageContainer");
    imageContainer->setMouseTracking(true);
    imageContainer->setFixedWidth(CARD_IMAGE_WIDTH);

    pic = new CardInfoPictureWidget(imageContainer);
    pic->setObjectName("cardHoverPic");
    pic->setMouseTracking(true);
    pic->setHoverToZoomEnabled(false);
    pic->setScaleFactor(135); // full-picture expanded size (~270px wide)

    auto *picLayout = new QVBoxLayout(imageContainer);
    picLayout->setContentsMargins(0, 0, 0, 0);
    picLayout->setSpacing(0);
    picLayout->addWidget(pic);
    imageContainer->setLayout(picLayout);

    createNameBarOverlay();
    createPTOverlay();

    pic->installEventFilter(this);
    // The artwork rect is only known after the pixmap is drawn (letterboxing, sideways-card
    // rotation), so re-anchor the overlays once it lands.
    connect(pic, &CardInfoPictureWidget::artRectChanged, this, &CardHoverWidget::positionOverlays,
            Qt::QueuedConnection);
}

// ---------- Name bar overlay (top of card) ----------

void CardHoverWidget::createNameBarOverlay()
{
    nameBarOverlay = new QWidget(imageContainer);
    nameBarOverlay->setObjectName("cardHoverNameBar");
    nameBarOverlay->setMouseTracking(true);
    nameBarOverlay->setStyleSheet(overlayBgStyle());

    nameLabel = new QLabel(nameBarOverlay);
    nameLabel->setObjectName("cardHoverNameLabel");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet(
        "color: white; font-weight: bold; font-size: 13px; background: transparent; padding: 2px;");
    nameLabel->setWordWrap(false);

    nameEdit = new QLineEdit(nameBarOverlay);
    nameEdit->setObjectName("cardHoverNameEdit");
    nameEdit->setAlignment(Qt::AlignCenter);
    nameEdit->setFixedHeight(22);
    nameEdit->setPlaceholderText(tr("Set annotation..."));
    nameEdit->setVisible(false);
    connect(nameEdit, &QLineEdit::editingFinished, this, [this]() {
        if (!nameEdit->isVisible()) {
            return;
        }
        if (currentCard && currentCard->getOwner()) {
            auto *actions = currentCard->getOwner()->getPlayerActions();
            if (actions) {
                QList<CardState *> cardList{currentCard->getState()};
                actions->actSetAnnotation(cardList, nameEdit->text());
            }
        }
        nameEdit->setVisible(false);
        nameLabel->setVisible(true);
        refreshDisplay();
    });

    annotationLabel = new QLabel(nameBarOverlay);
    annotationLabel->setObjectName("cardHoverAnnotationLabel");
    annotationLabel->setAlignment(Qt::AlignCenter);
    annotationLabel->setStyleSheet(
        "color: #dddddd; font-style: italic; font-size: 11px; background: transparent; padding: 1px;");
    annotationLabel->setWordWrap(false);
    annotationLabel->setVisible(false);

    auto *nameLayout = new QVBoxLayout(nameBarOverlay);
    nameLayout->setContentsMargins(6, 3, 6, 3);
    nameLayout->setSpacing(1);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);
    nameLayout->addWidget(annotationLabel);
    nameBarOverlay->setLayout(nameLayout);

    nameLabel->installEventFilter(this);
}

// ---------- P/T overlay (bottom-right of card): pill at rest, controls on hover ----------

void CardHoverWidget::createPTOverlay()
{
    ptOverlay = new QWidget(imageContainer);
    ptOverlay->setObjectName("cardHoverPTOverlay");
    ptOverlay->setMouseTracking(true);
    ptOverlay->setStyleSheet(overlayBgStyle());

    auto getActions = [this]() -> PlayerActions * {
        if (currentCard && currentCard->getOwner()) {
            return currentCard->getOwner()->getPlayerActions();
        }
        return nullptr;
    };
    auto cardList = [this]() -> QList<CardState *> {
        return currentCard && currentCard->getState() ? QList<CardState *>{currentCard->getState()}
                                                      : QList<CardState *>();
    };

    // Control rows: hidden at rest, shown while the P/T area is hovered.
    ptControls = new QWidget(ptOverlay);
    ptControls->setObjectName("cardHoverPTControls");
    ptControls->setMouseTracking(true);
    ptControls->setStyleSheet(QStringLiteral("background: transparent;"));

    // Value pill: the resting state, and the bottom (never-moving) row of the stack.
    // The accent top edge is the only cue that controls sit behind the pill; it doubles
    // as the divider between the pill and the rows once they are lifted out.
    ptPillLabel = new QLabel(ptOverlay);
    ptPillLabel->setObjectName("cardHoverPTPill");
    ptPillLabel->setAlignment(Qt::AlignCenter);
    // The accent top edge (the only cue that controls sit behind the pill) comes from the
    // palette-driven sheet in applyThemeStyle(), so it follows theme switches.
    ptPillLabel->setStyleSheet(QStringLiteral("color: white; font-weight: bold; font-size: 12px;"
                                              " background: transparent; padding: 0 %1px;")
                                   .arg(PT_PILL_TEXT_PADDING));
    ptPillLabel->setMinimumHeight(PT_PILL_MIN_HEIGHT);
    // Seed text: the resting size has to exist before the first card is shown.
    ptPillLabel->setText(QStringLiteral("00/00"));

    ptPowerLabel = new QLabel(ptControls);
    ptPowerLabel->setObjectName("cardHoverPTPowerValue");
    ptPowerLabel->setAlignment(Qt::AlignCenter);
    ptPowerLabel->setStyleSheet(
        "color: white; font-weight: bold; font-size: 12px; background: transparent; padding: 0 2px;");
    ptPowerLabel->setMinimumWidth(28);

    ptToughLabel = new QLabel(ptControls);
    ptToughLabel->setObjectName("cardHoverPTToughValue");
    ptToughLabel->setAlignment(Qt::AlignCenter);
    ptToughLabel->setStyleSheet(
        "color: white; font-weight: bold; font-size: 12px; background: transparent; padding: 0 2px;");
    ptToughLabel->setMinimumWidth(28);

    btnDecP = createOverlayButton("-P", tr("Decrease power"), [getActions, cardList]() {
        if (auto *a = getActions()) {
            a->actDecP(cardList());
        }
    });
    btnIncP = createOverlayButton("+P", tr("Increase power"), [getActions, cardList]() {
        if (auto *a = getActions()) {
            a->actIncP(cardList());
        }
    });
    btnDecT = createOverlayButton("-T", tr("Decrease toughness"), [getActions, cardList]() {
        if (auto *a = getActions()) {
            a->actDecT(cardList());
        }
    });
    btnIncT = createOverlayButton("+T", tr("Increase toughness"), [getActions, cardList]() {
        if (auto *a = getActions()) {
            a->actIncT(cardList());
        }
    });
    btnSetPT = createOverlayButton("SET", tr("Set power/toughness..."), [getActions, cardList]() {
        if (auto *a = getActions()) {
            a->actRequestSetPTDialog(cardList());
        }
    });
    btnResetPT = createOverlayButton("RST", tr("Reset power/toughness"), [getActions, cardList]() {
        if (auto *a = getActions()) {
            a->actResetPT(cardList());
        }
    });

    auto *powerRow = new QHBoxLayout();
    powerRow->setSpacing(2);
    powerRow->addWidget(btnDecP);
    powerRow->addWidget(ptPowerLabel);
    powerRow->addWidget(btnIncP);

    auto *toughRow = new QHBoxLayout();
    toughRow->setSpacing(2);
    toughRow->addWidget(btnDecT);
    toughRow->addWidget(ptToughLabel);
    toughRow->addWidget(btnIncT);

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(2);
    btnRow->addWidget(btnSetPT);
    btnRow->addWidget(btnResetPT);

    auto *controlsLayout = new QVBoxLayout(ptControls);
    controlsLayout->setContentsMargins(4, 3, 4, 3);
    controlsLayout->setSpacing(2);
    controlsLayout->addLayout(powerRow);
    controlsLayout->addLayout(toughRow);
    controlsLayout->addLayout(btnRow);
    ptControls->setLayout(controlsLayout);

    auto *ptLayout = new QVBoxLayout(ptOverlay);
    ptLayout->setContentsMargins(0, 0, 0, 0);
    ptLayout->setSpacing(2);
    ptLayout->addWidget(ptControls);
    ptLayout->addWidget(ptPillLabel);
    ptOverlay->setLayout(ptLayout);

    // Both states are driven from sizes cached here: hiding the controls would shrink
    // layout()->sizeHint(), so the two states must not be measured from the live layout.
    ptFullSize = ptLayout->sizeHint();
    ptControls->setVisible(false);
    ptCompactSize = measureCompactPTSize();
    ptPillLabel->clear();
    ptOverlay->resize(ptCompactSize);

    // The whole subtree feeds the hover state: moving onto a button must not read as a leave.
    ptOverlay->installEventFilter(this);
    const QList<QWidget *> ptChildren = ptOverlay->findChildren<QWidget *>();
    for (QWidget *child : ptChildren) {
        child->installEventFilter(this);
    }

    ptOverlay->setVisible(false);
}

// ---------- Nav strip: vertical icon buttons with horizontal text ----------

void CardHoverWidget::createNavBar()
{
    sideBar = new QWidget(this);
    sideBar->setObjectName("cardHoverSideBar");

    navNameLabel = new QLabel(sideBar);
    navNameLabel->setObjectName("cardHoverName");
    navNameLabel->setAlignment(Qt::AlignCenter);
    navNameLabel->setWordWrap(true);

    navGroup = new QButtonGroup(sideBar);
    navGroup->setExclusive(true);

    navInfoBtn = createNavButton(tr("Info"), QStringLiteral("icons/info"), PAGE_INFO);
    navMainBtn = createNavButton(tr("Main"), QStringLiteral("icons/cogwheel"), PAGE_MAIN);
    navMoveBtn = createNavButton(tr("Move"), QStringLiteral("icons/swap"), PAGE_MOVE);
    navCounterBtn = createNavButton(tr("Counters"), QStringLiteral("icons/increment"), PAGE_COUNTER);
    navTokenBtn = createNavButton(tr("Tokens"), QStringLiteral("icons/dragon"), PAGE_TOKEN);

    connect(navGroup, &QButtonGroup::idClicked, this, [this](int id) { contentStack->setCurrentIndex(id); });

    auto *barLayout = new QVBoxLayout(sideBar);
    barLayout->setContentsMargins(0, 0, 0, 0);
    barLayout->setSpacing(4);
    barLayout->setAlignment(Qt::AlignTop);
    barLayout->addWidget(navNameLabel);
    barLayout->addWidget(navInfoBtn);
    barLayout->addWidget(navMainBtn);
    barLayout->addWidget(navMoveBtn);
    barLayout->addWidget(navCounterBtn);
    barLayout->addWidget(navTokenBtn);
    barLayout->addStretch(1);
    sideBar->setLayout(barLayout);
}

QToolButton *CardHoverWidget::createNavButton(const QString &title, const QString &iconName, int pageIndex)
{
    auto *btn = new QToolButton(sideBar);
    btn->setObjectName("cardHoverNavBtn");
    btn->setText(title);
    btn->setToolTip(title);
    btn->setIcon(QIcon(themePixmap(iconName)));
    btn->setIconSize(QSize(NAV_ICON_SIZE, NAV_ICON_SIZE));
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setCheckable(true);
    btn->setAutoExclusive(true);
    btn->setMinimumWidth(NAV_BUTTON_WIDTH);
    navGroup->addButton(btn, pageIndex);
    return btn;
}

void CardHoverWidget::showPage(int index)
{
    contentStack->setCurrentIndex(index);
    if (auto *btn = navGroup->button(index)) {
        btn->setChecked(true);
    }
}

// ---------- Content pages (no nested tab bars) ----------

void CardHoverWidget::createContentStack()
{
    contentStack = new QStackedWidget(this);
    contentStack->setMinimumWidth(TAB_MIN_WIDTH);
    contentStack->setMaximumWidth(TAB_MAX_WIDTH);
    contentStack->setMinimumHeight(TAB_MIN_HEIGHT);

    infoPage = new QWidget(contentStack);
    text = new CardInfoTextWidget(infoPage);
    text->setObjectName("cardHoverText");
    connect(text, &CardInfoTextWidget::linkActivated, this, [this](const QString &cardName) {
        ExactCard card = CardDatabaseManager::query()->guessCard({cardName});
        if (!card.isEmpty()) {
            text->setCard(card);
        }
    });

    auto *infoScroll = new QScrollArea(infoPage);
    infoScroll->setWidget(text);
    infoScroll->setWidgetResizable(true);
    infoScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    infoScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    infoScroll->setFrameShape(QFrame::NoFrame);

    infoPageLayout = new QVBoxLayout(infoPage);
    infoPageLayout->setContentsMargins(4, 4, 4, 4);
    infoPageLayout->addWidget(infoScroll, 1);
    infoPage->setLayout(infoPageLayout);

    mainPage = makeStackPage(mainPageLayout);
    movePage = makeStackPage(movePageLayout);
    counterPage = makeStackPage(counterPageLayout);
    tokenPage = makeStackPage(tokenPageLayout);

    contentStack->addWidget(infoPage);
    contentStack->addWidget(mainPage);
    contentStack->addWidget(movePage);
    contentStack->addWidget(counterPage);
    contentStack->addWidget(tokenPage);
}

QWidget *CardHoverWidget::makeStackPage(QVBoxLayout *&layout)
{
    auto *page = new QWidget(contentStack);
    auto *inner = new QWidget(page);
    layout = new QVBoxLayout(inner);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    layout->setAlignment(Qt::AlignTop);
    inner->setLayout(layout);

    auto *scroll = new QScrollArea(page);
    scroll->setWidget(inner);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll, 1);
    page->setLayout(outer);

    return page;
}

QPushButton *CardHoverWidget::createSmallButton(const QString &text, const std::function<void()> &slot, QWidget *parent)
{
    return createSmallButton(text, QString(), slot, parent);
}

// Packs icon-over-text tool buttons into a uniform grid so action pages use the full width.
class ButtonGrid
{
public:
    explicit ButtonGrid(QVBoxLayout *layout, int columns = 2) : columns(columns)
    {
        grid = new QGridLayout();
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(4);
        layout->addLayout(grid);
    }

    void addSection(QWidget *header)
    {
        if (col != 0) {
            ++row;
            col = 0;
        }
        grid->addWidget(header, row++, 0, 1, columns);
    }

    void addButton(QWidget *button)
    {
        grid->addWidget(button, row, col++);
        if (col >= columns) {
            col = 0;
            ++row;
        }
    }

private:
    QGridLayout *grid;
    int columns;
    int row = 0;
    int col = 0;
};

QToolButton *CardHoverWidget::createGridButton(const QString &text,
                                               const QString &iconName,
                                               const std::function<void()> &slot,
                                               QWidget *parent,
                                               const QString &tooltip)
{
    auto *btn = new QToolButton(parent);
    btn->setObjectName("cardHoverGridBtn");
    btn->setText(text);
    // Short cell labels can clip; the full text is always one hover away.
    btn->setToolTip(tooltip.isEmpty() ? text : tooltip);
    btn->setIcon(QIcon(themePixmap(iconName)));
    btn->setIconSize(QSize(22, 22));
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setMinimumHeight(58);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(btn, &QToolButton::clicked, this, [slot]() { slot(); });
    return btn;
}

QPushButton *CardHoverWidget::createSmallButton(const QString &text,
                                                const QString &tooltip,
                                                const std::function<void()> &slot,
                                                QWidget *parent)
{
    auto *btn = new QPushButton(text, parent ? parent : contentStack);
    btn->setObjectName("cardHoverActionBtn");
    btn->setMinimumHeight(30);
    if (!tooltip.isEmpty()) {
        btn->setToolTip(tooltip);
    }
    connect(btn, &QPushButton::clicked, this, [slot]() { slot(); });
    return btn;
}

QPushButton *
CardHoverWidget::createOverlayButton(const QString &text, const QString &tooltip, const std::function<void()> &slot)
{
    const QPalette &pal = QApplication::palette();
    QColor bg = pal.color(QPalette::Dark);
    bg.setAlpha(180);
    QColor hoverBg = pal.color(QPalette::Mid);
    hoverBg.setAlpha(200);
    QColor pressBg = pal.color(QPalette::Shadow);
    pressBg.setAlpha(200);
    QColor fg = pal.color(QPalette::ButtonText);

    auto *btn = new QPushButton(text, imageContainer);
    btn->setObjectName("cardHoverOverlayBtn");
    btn->setToolTip(tooltip);
    btn->setFixedSize(OVERLAY_BTN_SIZE + 8, OVERLAY_BTN_SIZE);
    btn->setFlat(true);
    btn->setStyleSheet(QStringLiteral("QPushButton {"
                                      "  color: %1; font-size: 10px; font-weight: bold;"
                                      "  background: %2; border: none;"
                                      "  border-radius: 3px;"
                                      "}"
                                      "QPushButton:hover { background: %3; }"
                                      "QPushButton:pressed { background: %4; }")
                           .arg(fg.name(), bg.name(), hoverBg.name(), pressBg.name()));
    connect(btn, &QPushButton::clicked, this, [slot]() { slot(); });
    return btn;
}

QLabel *CardHoverWidget::createSectionHeader(const QString &text, QWidget *parent)
{
    auto *lbl = new QLabel(text, parent);
    lbl->setObjectName("cardHoverSectionHeader");
    lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl->setMinimumHeight(22);
    return lbl;
}

QLabel *CardHoverWidget::createNoteLabel(const QString &text, QWidget *parent)
{
    auto *lbl = new QLabel(text, parent);
    lbl->setObjectName("cardHoverNote");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setWordWrap(true);
    return lbl;
}

// ---------- Live updates: stay in sync while the popup is open ----------

void CardHoverWidget::connectCardSignals()
{
    if (!currentCard) {
        return;
    }
    if (CardState *state = currentCard->getState()) {
        connect(state, &CardState::stateChanged, this, &CardHoverWidget::scheduleRefresh);
    }
    connect(currentCard, &CardItem::tappedChanged, this, &CardHoverWidget::scheduleRefresh);
    connect(currentCard, &CardItem::faceDownChanged, this, &CardHoverWidget::scheduleRefresh);
    connect(currentCard, &QObject::destroyed, this, &CardHoverWidget::onCardDestroyed);
}

void CardHoverWidget::disconnectCardSignals()
{
    if (currentCard) {
        if (CardState *state = currentCard->getState()) {
            disconnect(state, nullptr, this, nullptr);
        }
        disconnect(currentCard, nullptr, this, nullptr);
    }
}

void CardHoverWidget::scheduleRefresh()
{
    if (!currentCard) {
        return;
    }
    if (!refreshTimer->isActive()) {
        refreshTimer->start();
    }
}

void CardHoverWidget::onCardChanged()
{
    if (!currentCard) {
        return;
    }

    // The underlying card may have changed identity (copy effects, flip to another face)
    ExactCard now = currentCard->getCard();
    if (now.getCardPtr() != shownCard.getCardPtr()) {
        shownCard = now;
        pic->setCard(now);
        text->setCard(now);
        nameLabel->setText(now.getInfo().getName());
        navNameLabel->setText(now.getInfo().getName());
    }

    refreshDisplay();
    rebuildActionsForZone();
    positionOverlays();
}

void CardHoverWidget::onCardDestroyed()
{
    // Never touch the sender here: it is mid-teardown. Qt already auto-disconnects
    // its signals, so just drop our reference; clearCard() then skips sender access.
    currentCard = nullptr;
    clearCard();
    hideAnimated();
}

// ---------- Grouped action builders (mirror CardMenu per zone) ----------

void CardHoverWidget::rebuildActionsForZone()
{
    clearLayout(mainPageLayout);
    clearLayout(movePageLayout);
    clearLayout(counterPageLayout);
    clearLayout(tokenPageLayout);

    if (!currentCard || !currentPlayer) {
        return;
    }

    auto *card = currentCard.data();
    QList<CardState *> cardList{card->getState()};

    bool canModify = currentPlayer->getLogic()->getPlayerInfo()->getLocalOrJudge();
    if (auto *view = qobject_cast<ZoneViewZoneLogic *>(card->getZone())) {
        if (view->getRevealZone() && !view->getWriteableRevealZone()) {
            canModify = false;
        }
    }

    QString zoneName = card->getZone() ? card->getZone()->getName() : QString();

    buildMainTab(cardList, zoneName, canModify);
    buildMoveTab(cardList, zoneName, canModify);
    buildCounterTab(cardList, zoneName, canModify);
    buildTokenTab(card, canModify);
}

void CardHoverWidget::buildMainTab(const QList<CardState *> &cardList, const QString &zoneName, bool canModify)
{
    auto *layout = mainPageLayout;
    auto *card = currentCard.data();
    auto *player = currentPlayer.data();
    auto *actions = player->getLogic()->getPlayerActions();
    auto *gameScene = player->getGameScene();
    auto invoke = [actions, cardList](CardMenuActionType type) {
        return [actions, cardList, type]() { actions->cardMenuAction(cardList, type); };
    };
    auto attach = [card]() { card->drawAttachArrow(); };
    auto drawArrow = [card]() { card->drawArrow(Qt::red); };

    if (!canModify) {
        ButtonGrid grid(layout);
        grid.addSection(createSectionHeader(tr("Revealed"), mainPage));
        grid.addButton(createGridButton(
            tr("Hide"), QStringLiteral("icons/arrow_left_green"), [actions, cardList]() { actions->actHide(cardList); },
            mainPage));
        grid.addButton(createGridButton(tr("Clone"), QStringLiteral("icons/increment"), invoke(cmClone), mainPage));
        grid.addSection(createSectionHeader(tr("Select"), mainPage));
        grid.addButton(createGridButton(
            tr("Select All"), QStringLiteral("icons/search"), [gameScene, card]() { gameScene->selectAllInZone(card); },
            mainPage));
        grid.addButton(createGridButton(
            tr("Select Column"), QStringLiteral("icons/filter"),
            [gameScene, card]() { gameScene->selectSameColumn(card); }, mainPage));
        layout->addStretch(1);
        return;
    }

    if (zoneName == ZoneNames::TABLE) {
        ButtonGrid grid(layout);
        grid.addSection(createSectionHeader(tr("Battlefield"), mainPage));
        grid.addButton(createGridButton(tr("Tap / Untap"), QStringLiteral("icons/sync"), invoke(cmTap), mainPage));
        auto *doesntUntap = createGridButton(
            tr("Skip untapping"), QStringLiteral("icons/lock"),
            [actions, cardList]() { actions->cardMenuAction(cardList, cmDoesntUntap); }, mainPage);
        doesntUntap->setCheckable(true);
        doesntUntap->setChecked(card->getDoesntUntap());
        grid.addButton(doesntUntap);
        grid.addButton(createGridButton(tr("Turn Over"), QStringLiteral("icons/swap"), invoke(cmFlip), mainPage));
        if (card->getFaceDown()) {
            grid.addButton(
                createGridButton(tr("Peek at card face"), QStringLiteral("icons/view"), invoke(cmPeek), mainPage));
        }
        grid.addButton(createGridButton(tr("Clone"), QStringLiteral("icons/increment"), invoke(cmClone), mainPage));
        grid.addButton(createGridButton(tr("Attach to card..."), QStringLiteral("icons/pin"), attach, mainPage));
        if (card->getAttachedTo()) {
            grid.addButton(createGridButton(
                tr("Unattach"), QStringLiteral("icons/arrow_undo"),
                [actions, cardList]() { actions->actUnattach(cardList); }, mainPage));
        }
        grid.addButton(createGridButton(tr("Draw arrow..."), QStringLiteral("icons/share"), drawArrow, mainPage));
        grid.addButton(createGridButton(
            tr("Set annotation..."), QStringLiteral("icons/pencil"),
            [actions, cardList]() { actions->actRequestSetAnnotationDialog(cardList); }, mainPage));
        grid.addButton(createGridButton(
            tr("Reduce life"), QStringLiteral("icons/arrow_down_green"),
            [actions, cardList]() { actions->actReduceLifeByPower(cardList); }, mainPage));
        grid.addSection(createSectionHeader(tr("Select"), mainPage));
        grid.addButton(createGridButton(
            tr("Select All"), QStringLiteral("icons/search"), [gameScene, card]() { gameScene->selectAllInZone(card); },
            mainPage));
        grid.addButton(createGridButton(
            tr("Select Row"), QStringLiteral("icons/filter"), [gameScene, card]() { gameScene->selectSameRow(card); },
            mainPage));
    } else if (!zoneName.isEmpty()) {
        ButtonGrid grid(layout);
        grid.addSection(createSectionHeader(tr("Zone"), mainPage));
        grid.addButton(createGridButton(
            tr("Play"), QStringLiteral("icons/arrow_right_green"),
            [actions, cardList]() { actions->actPlay(cardList); }, mainPage));
        grid.addButton(createGridButton(
            tr("Play Face Down"), QStringLiteral("icons/arrow_right_blue"),
            [actions, cardList]() { actions->actPlayFacedown(cardList); }, mainPage));
        grid.addButton(createGridButton(tr("Clone"), QStringLiteral("icons/increment"), invoke(cmClone), mainPage));
        if (zoneName == ZoneNames::STACK || zoneName == ZoneNames::HAND || zoneName == ZoneNames::GRAVE ||
            zoneName == ZoneNames::EXILE) {
            grid.addButton(createGridButton(tr("Attach to card..."), QStringLiteral("icons/pin"), attach, mainPage));
            grid.addButton(createGridButton(tr("Draw arrow..."), QStringLiteral("icons/share"), drawArrow, mainPage));
        }
        grid.addSection(createSectionHeader(tr("Select"), mainPage));
        grid.addButton(createGridButton(
            tr("Select All"), QStringLiteral("icons/search"), [gameScene, card]() { gameScene->selectAllInZone(card); },
            mainPage));
        if (qobject_cast<ZoneViewZoneLogic *>(card->getZone())) {
            grid.addButton(createGridButton(
                tr("Select Column"), QStringLiteral("icons/filter"),
                [gameScene, card]() { gameScene->selectSameColumn(card); }, mainPage));
        }
    } else {
        layout->addWidget(createNoteLabel(tr("No card actions here — see the Move tab."), mainPage));
    }

    layout->addStretch(1);
}

void CardHoverWidget::buildMoveTab(const QList<CardState *> &cardList, const QString &zoneName, bool canModify)
{
    auto *layout = movePageLayout;
    if (!canModify) {
        layout->addWidget(createNoteLabel(tr("Read-only."), movePage));
        layout->addStretch(1);
        return;
    }

    auto *player = currentPlayer.data();
    auto *actions = player->getLogic()->getPlayerActions();
    auto invoke = [actions, cardList](CardMenuActionType type) {
        return [actions, cardList, type]() { actions->cardMenuAction(cardList, type); };
    };

    ButtonGrid grid(layout);
    grid.addSection(createSectionHeader(tr("Move to"), movePage));
    grid.addButton(createGridButton(tr("Top of library"), QStringLiteral("icons/arrow_top_green"),
                                    invoke(cmMoveToTopLibrary), movePage));
    grid.addButton(createGridButton(
        tr("X from top..."), QStringLiteral("icons/search"),
        [actions]() { actions->actRequestMoveCardXCardsFromTopDialog(); }, movePage, tr("X from top of library...")));
    grid.addButton(createGridButton(tr("Bottom of library"), QStringLiteral("icons/arrow_bottom_green"),
                                    invoke(cmMoveToBottomLibrary), movePage));
    grid.addButton(createGridButton(tr("Table"), QStringLiteral("icons/player"), invoke(cmMoveToTable), movePage));
    grid.addButton(
        createGridButton(tr("Hand"), QStringLiteral("icons/arrow_left_green"), invoke(cmMoveToHand), movePage));
    grid.addButton(createGridButton(tr("Graveyard"), QStringLiteral("icons/arrow_down_green"),
                                    invoke(cmMoveToGraveyard), movePage));
    grid.addButton(
        createGridButton(tr("Exile"), QStringLiteral("icons/arrow_right_blue"), invoke(cmMoveToExile), movePage));

    // Reveal actions live on hand / custom zones in CardMenu
    if (!zoneName.isEmpty() && zoneName != ZoneNames::TABLE && zoneName != ZoneNames::STACK &&
        zoneName != ZoneNames::GRAVE && zoneName != ZoneNames::EXILE) {
        auto *game = player->getLogic()->getGame();
        if (game && game->getPlayerManager()) {
            grid.addSection(createSectionHeader(tr("Reveal to..."), movePage));
            grid.addButton(createGridButton(
                tr("All players"), QStringLiteral("icons/share"),
                [actions, cardList]() {
                    auto *fake = new QAction(nullptr);
                    fake->setData(-1);
                    actions->actReveal(cardList, fake);
                    fake->deleteLater();
                },
                movePage));
            const auto players = game->getPlayerManager()->getPlayers().values();
            for (auto *other : players) {
                if (other == player->getLogic()) {
                    continue;
                }
                QString name = other->getPlayerInfo()->getName();
                int id = other->getPlayerInfo()->getId();
                grid.addButton(createGridButton(
                    name, QStringLiteral("icons/player"),
                    [actions, cardList, id]() {
                        auto *fake = new QAction(nullptr);
                        fake->setData(id);
                        actions->actReveal(cardList, fake);
                        fake->deleteLater();
                    },
                    movePage));
            }
        }
    }

    layout->addStretch(1);
}

void CardHoverWidget::buildCounterTab(const QList<CardState *> &cardList, const QString &zoneName, bool canModify)
{
    auto *layout = counterPageLayout;
    auto *actions = currentPlayer->getLogic()->getPlayerActions();

    if (!canModify) {
        layout->addWidget(createNoteLabel(tr("Read-only."), counterPage));
        layout->addStretch(1);
        return;
    }
    if (zoneName != ZoneNames::TABLE) {
        layout->addWidget(createNoteLabel(
            tr("Counters live on battlefield cards. PT and counters can be changed there."), counterPage));
        layout->addStretch(1);
        return;
    }

    layout->addWidget(createSectionHeader(tr("Power / Toughness"), counterPage));
    layout->addWidget(createSmallButton(
        tr("Set power and toughness..."), [actions, cardList]() { actions->actRequestSetPTDialog(cardList); },
        counterPage));
    layout->addWidget(createSmallButton(
        tr("Reset power and toughness"), [actions, cardList]() { actions->actResetPT(cardList); }, counterPage));

    // One group per counter: color dot plus live value in the header, its actions in
    // a compact row beneath it. Rebuilt on every card change, so values never go stale.
    const QMap<int, int> counters = currentCard ? currentCard->getCounters() : QMap<int, int>{};
    for (int i = 0; i < 6; ++i) {
        const QString counterName = SettingsCache::instance().cardCounters().displayName(i);
        const QString dot = QStringLiteral("<font color=\"%1\">\u25cf</font> ")
                                .arg(SettingsCache::instance().cardCounters().color(i).name());
        const int value = counters.value(i, 0);
        layout->addWidget(createSectionHeader(dot + tr("%1 \u00d7 %2").arg(counterName).arg(value), counterPage));

        auto *row = new QHBoxLayout();
        row->setSpacing(4);
        row->addWidget(createSmallButton(
            tr("Add"), [actions, cardList, i]() { actions->actAddCardCounter(cardList, i); }, counterPage));
        if (value > 0) {
            row->addWidget(createSmallButton(
                tr("Remove"), [actions, cardList, i]() { actions->actRemoveCardCounter(cardList, i); }, counterPage));
        }
        row->addWidget(createSmallButton(
            tr("Set..."), [actions, cardList, i]() { actions->actRequestSetCardCounterDialog(cardList, i); },
            counterPage));
        layout->addLayout(row);
    }

    layout->addStretch(1);
}

void CardHoverWidget::buildTokenTab(CardItem *card, bool canModify)
{
    auto *layout = tokenPageLayout;
    if (!card || !currentPlayer) {
        layout->addStretch(1);
        return;
    }
    auto *actions = currentPlayer->getLogic()->getPlayerActions();
    ExactCard exactCard = card->getCard();

    bool anyToken = false;
    if (canModify && !exactCard.isEmpty()) {
        QList<CardRelation *> relatedCards = exactCard.getInfo().getAllRelatedCards();
        bool hasMulti = relatedCards.size() > 1;
        int known = 0;
        for (CardRelation *relation : relatedCards) {
            ExactCard related =
                CardDatabaseManager::query()->getCardFromSameSet(relation->getName(), exactCard.getPrinting());
            if (!related) {
                related = CardDatabaseManager::query()->getCard({relation->getName()});
            }
            if (!related) {
                continue;
            }
            if (known == 0) {
                layout->addWidget(createSectionHeader(tr("Tokens"), tokenPage));
            }
            ++known;
            QString relatedName = related.getInfo().getPowTough().isEmpty()
                                      ? related.getName()
                                      : related.getInfo().getPowTough() + " " + related.getName();
            CardRelation *captured = relation;
            CardItem *source = card;
            layout->addWidget(createSmallButton(
                tr("Token: ") + relatedName,
                [actions, source, captured]() {
                    actions->actRequestCreateRelatedFromRelationDialog(source->getState(), captured);
                },
                tokenPage));
        }
        if (hasMulti && known > 0) {
            layout->addWidget(createSmallButton(
                tr("All tokens"), [actions, card]() { actions->actCreateAllRelatedCards(card->getState()); },
                tokenPage));
        }
        anyToken = known > 0;
    }

    // Save the tab index before adding related-view buttons so the header only shows when useful
    int beforeRelated = layout->count();
    addRelatedViewButtons(layout);
    bool anyRelated = layout->count() != beforeRelated;

    if (!anyToken && !anyRelated) {
        layout->addWidget(createNoteLabel(tr("No tokens or related cards."), tokenPage));
    }

    layout->addStretch(1);
}

void CardHoverWidget::addRelatedViewButtons(QVBoxLayout *layout)
{
    if (!currentCard) {
        return;
    }
    ExactCard exactCard = currentCard->getCard();
    if (exactCard.isEmpty()) {
        return;
    }
    QList<CardRelation *> relatedCards = exactCard.getInfo().getAllRelatedCards();
    bool anyKnown = false;
    for (const CardRelation *relation : relatedCards) {
        if (CardDatabaseManager::query()->getCardInfo(relation->getName())) {
            anyKnown = true;
            break;
        }
    }
    if (!anyKnown) {
        return;
    }
    QWidget *page = layout == tokenPageLayout ? tokenPage : contentStack;

    layout->addWidget(createSectionHeader(tr("Related cards"), page));
    for (const CardRelation *relation : relatedCards) {
        QString name = relation->getName();
        if (!CardDatabaseManager::query()->getCardInfo(name)) {
            continue;
        }
        layout->addWidget(createSmallButton(
            name,
            [this, name, exactCard]() {
                ExactCard related = CardDatabaseManager::query()->getCard({name, exactCard.getPrinting().getUuid()});
                if (!related.isEmpty()) {
                    text->setCard(related);
                    showPage(PAGE_INFO);
                }
            },
            page));
    }
}

// ---------- Layout helper ----------

void CardHoverWidget::clearLayout(QLayout *layout)
{
    if (!layout) {
        return;
    }
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        if (item->layout()) {
            clearLayout(item->layout());
        }
        delete item;
    }
}

// ---------- Overlay positioning ----------

void CardHoverWidget::positionOverlays()
{
    if (!currentCard || !pic) {
        return;
    }

    nameBarOverlay->show();

    int containerWidth = imageContainer->width();

    // Name bar at top — width constrained, height from layout
    int nameWidth = containerWidth - 8;
    nameBarOverlay->setFixedWidth(nameWidth);
    nameBarOverlay->adjustSize();
    nameBarOverlay->move(4, 4);

    // Show/hide based on card data; an empty P/T always falls back to the compact pill
    const bool hasPT = !currentCard->getPT().isEmpty();
    if (!hasPT && ptExpanded) {
        setPTExpanded(false);
    }
    applyPTOverlayGeometry();
    ptOverlay->setVisible(hasPT);
}

void CardHoverWidget::setPTExpanded(bool expanded)
{
    if (!ptOverlay || !ptControls) {
        return;
    }
    // Nothing to expand into when the card has no P/T.
    if (expanded && (!currentCard || currentCard->getPT().isEmpty())) {
        expanded = false;
    }
    // Mouse movement drives this on every event; only apply a real state change.
    if (ptExpanded == expanded) {
        return;
    }

    ptExpanded = expanded;
    ptControls->setVisible(expanded);
    if (expanded) {
        // The stack covers more of the art than the pill did; keep it above sibling overlays.
        ptOverlay->raise();
    }
    applyPTOverlayGeometry();
}

void CardHoverWidget::applyPTOverlayGeometry()
{
    if (!ptOverlay || !pic || ptCompactSize.isEmpty() || ptFullSize.isEmpty()) {
        return;
    }
    // Anchor on the drawn artwork, not on imageContainer: the container can be taller than the
    // picture, and sideways-layout cards are letterboxed inside pic. Both used to drop the
    // overlay below the printed P/T box.
    const QRect picture =
        pic->artRect()
            .translated(pic->pos())
            .adjusted(PT_PILL_EDGE_INSET, PT_PILL_EDGE_INSET, -PT_PILL_EDGE_INSET, -PT_PILL_EDGE_INSET);
    const QRect compact = compactPillRect();
    if (picture.isEmpty() || compact.isEmpty()) {
        return;
    }

    // Expanded: same bottom edge and right edge as the pill, so the controls grow upward
    // and never appear under the cursor that is resting on the pill. Never narrower than
    // the current pill, or a three-digit value would clip against the stack's edge.
    const QSize size =
        ptExpanded ? QSize(qMax(ptFullSize.width(), ptCompactSize.width()), ptFullSize.height()) : ptCompactSize;
    const int x = clampToSpan(picture.x(), picture.width(), size.width(), compact.x() + compact.width() - size.width());
    const int y =
        clampToSpan(picture.y(), picture.height(), size.height(), compact.y() + compact.height() - size.height());

    ptOverlay->setGeometry(x, y, size.width(), size.height());
}

QRect CardHoverWidget::compactPillRect() const
{
    if (!pic || ptCompactSize.isEmpty()) {
        return {};
    }
    const QRect picture =
        pic->artRect()
            .translated(pic->pos())
            .adjusted(PT_PILL_EDGE_INSET, PT_PILL_EDGE_INSET, -PT_PILL_EDGE_INSET, -PT_PILL_EDGE_INSET);
    if (picture.isEmpty()) {
        return {};
    }
    // Centred on the printed P/T box (bottom-right of the art).
    const int anchorX = picture.x() + qRound(picture.width() * PT_PILL_ANCHOR_X);
    const int anchorY = picture.y() + qRound(picture.height() * PT_PILL_ANCHOR_Y);
    return QRect(
        clampToSpan(picture.x(), picture.width(), ptCompactSize.width(), anchorX - ptCompactSize.width() / 2),
        clampToSpan(picture.y(), picture.height(), ptCompactSize.height(), anchorY - ptCompactSize.height() / 2),
        ptCompactSize.width(), ptCompactSize.height());
}

QRect CardHoverWidget::ptOpenRegion() const
{
    const QRect pill = compactPillRect();
    if (pill.isEmpty() || !pic) {
        return {};
    }
    // The opening target is the P/T box itself (plus a small halo), not the whole bottom-right
    // quadrant: a wider trigger lifts the rows out from under a cursor that merely passed by,
    // and the click that selects the card would land on a P/T button instead.
    return pill.translated(-pic->pos()).adjusted(-PT_OPEN_HALO, -PT_OPEN_HALO, PT_OPEN_HALO, PT_OPEN_HALO);
}

QRect CardHoverWidget::ptHoverRegion() const
{
    if (!pic) {
        return {};
    }
    // The band is relative to the drawn artwork, so a letterboxed (sideways-layout) card keeps
    // its P/T corner where the printed box actually is.
    const QRect art = pic->artRect();
    if (art.isEmpty()) {
        return {};
    }
    return QRect(art.x() + qRound(art.width() * (1.0 - PT_HOVER_REGION_WIDTH)),
                 art.y() + qRound(art.height() * (1.0 - PT_HOVER_REGION_HEIGHT)),
                 qRound(art.width() * PT_HOVER_REGION_WIDTH), qRound(art.height() * PT_HOVER_REGION_HEIGHT));
}

bool CardHoverWidget::isCursorOverPTArea() const
{
    if (!ptOverlay || !pic || !imageContainer) {
        return false;
    }
    if (ptOverlay->rect().contains(ptOverlay->mapFromGlobal(QCursor::pos()))) {
        return true;
    }
    return ptHoverRegion().translated(pic->pos()).contains(imageContainer->mapFromGlobal(QCursor::pos()));
}

QSize CardHoverWidget::measureCompactPTSize() const
{
    if (!ptPillLabel) {
        return QSize(PT_PILL_MIN_WIDTH, PT_PILL_MIN_HEIGHT);
    }
    // QLabel::sizeHint() leaves out the stylesheet padding, so add it back explicitly.
    const QSize hint = ptPillLabel->sizeHint();
    return QSize(qMax(PT_PILL_MIN_WIDTH, hint.width() + 2 * PT_PILL_TEXT_PADDING),
                 qMax(PT_PILL_MIN_HEIGHT, hint.height()));
}

void CardHoverWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    positionOverlays();
}

void CardHoverWidget::hideEvent(QHideEvent *event)
{
    // A reopened panel for the same card must not come back with the rows still lifted.
    setPTExpanded(false);
    // The window is already hidden here; repaint whatever it was covering.
    invalidateCoveredArea();
    QFrame::hideEvent(event);
}

// ---------- Display updates ----------

void CardHoverWidget::updatePTDisplay()
{
    if (!currentCard) {
        ptPowerLabel->clear();
        ptToughLabel->clear();
        ptPillLabel->clear();
        setPTExpanded(false);
        return;
    }

    QString pt = currentCard->getPT();
    if (pt.isEmpty()) {
        ptPowerLabel->setText("--");
        ptToughLabel->setText("--");
        ptPillLabel->setText("--");
        setPTExpanded(false);
    } else {
        int slashIdx = pt.indexOf('/');
        if (slashIdx >= 0) {
            ptPowerLabel->setText(pt.left(slashIdx));
            ptToughLabel->setText(pt.mid(slashIdx + 1));
        } else {
            ptPowerLabel->setText(pt);
            ptToughLabel->setText("--");
        }
        ptPillLabel->setText(pt);
    }

    // Three-digit values (100/100, -1/-1) are legal, so the resting pill cannot keep a
    // size measured from the seed text: re-measure on every value change and carry the
    // width into the expanded stack.
    ptCompactSize = measureCompactPTSize();
    applyPTOverlayGeometry();
}

// ---------- Card data ----------

void CardHoverWidget::setCard(CardItem *card, PlayerGraphicsItem *player)
{
    disconnectCardSignals();

    if (!card || !player) {
        clearCard();
        return;
    }

    currentCard = card;
    currentPlayer = player;

    ExactCard exactCard = card->getCard();
    shownCard = exactCard;
    pic->setCard(exactCard);
    text->setCard(exactCard);

    nameLabel->setText(exactCard.getInfo().getName());
    navNameLabel->setText(exactCard.getInfo().getName());

    connectCardSignals();
    refreshDisplay();
    rebuildActionsForZone();
    // A new card always starts compact; the rows only lift out while P/T is hovered.
    setPTExpanded(false);
    positionOverlays();
    // The widget may have been disabled by a previous teardown; it is interactive again.
    setEnabled(true);
}

void CardHoverWidget::refreshDisplay()
{
    updatePTDisplay();

    if (!currentCard) {
        annotationLabel->setVisible(false);
        return;
    }

    QString annotation = currentCard->getAnnotation();
    if (annotation.isEmpty()) {
        annotationLabel->setVisible(false);
    } else {
        annotationLabel->setText(annotation);
        annotationLabel->setVisible(true);
    }
}

void CardHoverWidget::clearCard()
{
    // Go non-interactive synchronously: the action buttons below are only deleteLater'd,
    // so without this a click landing before deferred deletion runs would invoke a lambda
    // holding the destroyed card/player. Disabled widgets deliver no button clicks, which
    // also covers the fade-out window (hideAnimated only fades; hide() lands on finish).
    setEnabled(false);
    disconnectCardSignals();
    refreshTimer->stop();
    currentCard = nullptr;
    currentPlayer = nullptr;
    shownCard = ExactCard();
    pic->setCard(ExactCard());
    text->setCard(ExactCard());
    nameLabel->clear();
    navNameLabel->clear();
    nameEdit->clear();
    annotationLabel->clear();
    annotationLabel->setVisible(false);
    ptPowerLabel->clear();
    ptToughLabel->clear();
    ptPillLabel->clear();
    setPTExpanded(false);
    ptOverlay->setVisible(false);
    clearLayout(mainPageLayout);
    clearLayout(movePageLayout);
    clearLayout(counterPageLayout);
    clearLayout(tokenPageLayout);
    showPage(PAGE_INFO);
}

bool CardHoverWidget::hasCard() const
{
    return currentCard != nullptr;
}

// ---------- Positioning: the picture lands exactly over the card, tabs spill to the side ----------

void CardHoverWidget::positionNextTo(const QRectF &cardSceneRect,
                                     const QTransform &viewportTransform,
                                     const QWidget *viewport)
{
    QRect cardViewRect = viewportTransform.mapRect(cardSceneRect).toRect();
    QPoint cardCenter = viewport->mapToGlobal(cardViewRect.center());

    QSize popupSize = sizeHint();
    popupSize = QSize(qMax(popupSize.width(), TAB_MIN_WIDTH + NAV_BUTTON_WIDTH + CARD_IMAGE_WIDTH + 40),
                      qMax(popupSize.height(), TAB_MIN_HEIGHT + 16));

    auto *screen = QGuiApplication::screenAt(cardCenter);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    QRect screenRect = screen->availableGeometry();

    // Clamp oversized popups to the screen before choosing a side
    popupSize.setWidth(qMin(popupSize.width(), screenRect.width() - 2 * POPUP_GAP));
    popupSize.setHeight(qMin(popupSize.height(), screenRect.height() - 2 * POPUP_GAP));

    const int mLeft = mainLayout->contentsMargins().left();
    const int mRight = mainLayout->contentsMargins().right();
    const int mTop = mainLayout->contentsMargins().top();
    const int halfImgW = CARD_IMAGE_WIDTH / 2;
    // Picture height at full size (the pic has a fixed scale-factor size here; the morph
    // collapse happens later in showAnimated).
    const int imageH = pic->sizeHint().height();

    // Center the picture over the card (not corner-pinned) so the overhang is even on
    // all sides; only the nav panel spills out to the side. Prefer spilling right, then left.
    const int xRight = cardCenter.x() - mLeft - halfImgW;
    const int xLeft = cardCenter.x() + mRight + halfImgW - popupSize.width();
    const bool rightFits = xRight + popupSize.width() <= screenRect.right() - POPUP_GAP;
    const bool leftFits = xLeft >= screenRect.left() + POPUP_GAP;
    popupOnRight = rightFits || !leftFits;
    mainLayout->setDirection(popupOnRight ? QBoxLayout::LeftToRight : QBoxLayout::RightToLeft);

    int x = popupOnRight ? xRight : xLeft;

    // Center the picture vertically on the card (it is top-aligned in its container).
    int y = cardCenter.y() - mTop - imageH / 2;
    if (y + popupSize.height() > screenRect.bottom() - POPUP_GAP) {
        y = screenRect.bottom() - popupSize.height() - POPUP_GAP;
    }
    if (y < screenRect.top() + POPUP_GAP) {
        y = screenRect.top() + POPUP_GAP;
    }

    x = qMax(screenRect.left() + POPUP_GAP, x);
    x = qMin(screenRect.right() - popupSize.width() - POPUP_GAP, x);

    move(x, y);
    resize(popupSize);
    invalidateCoveredArea();
}

void CardHoverWidget::invalidateCoveredArea()
{
    // The panel is an overlay window: moving or hiding it does not reliably invalidate
    // the pixels it uncovered in the game view, which then stay stale (a black square)
    // until something else repaints that area. Do it explicitly, unioned with the
    // previous placement so a move repaints both sides.
    QWidget *viewport = parentWidget();
    if (!viewport) {
        lastViewportRect = QRect();
        return;
    }

    QRect covered;
    if (isVisible()) {
        covered = QRect(viewport->mapFromGlobal(geometry().topLeft()), size());
    }
    const QRect dirty = lastViewportRect.united(covered);
    if (!dirty.isEmpty()) {
        viewport->update(dirty.adjusted(-1, -1, 1, 1));
    }
    lastViewportRect = covered;
}

// ---------- Show / hide: no window travel, the board zoom is the morph ----------
// The panel opens instantly at its final geometry (GameScene positioned it with the
// picture centered over the zoomed card) and only fades in. Any geometry animation
// here read as the panel "flying in", so there is none — not even for card switches.

void CardHoverWidget::showAnimated()
{
    if (!hasCard()) {
        return;
    }
    fadeOut->stop();

    // Fresh open: fade the whole panel in over the zoomed card. Card switches while
    // visible keep full opacity and just show.
    bool wasHidden = !isVisible();
    if (wasHidden) {
        setWindowOpacity(0.0);
    }
    show();
    positionOverlays();

    fadeIn->setStartValue(windowOpacity());
    fadeIn->setEndValue(1.0);
    fadeIn->start();
    invalidateCoveredArea();
}

void CardHoverWidget::hideAnimated()
{
    if (isClosing()) {
        // Repeated hide requests must not restart the fade, or the panel never finishes
        // closing while the cursor keeps moving.
        return;
    }
    fadeIn->stop();
    fadeOut->setStartValue(windowOpacity());
    fadeOut->setEndValue(0.0);
    fadeOut->start();
}

bool CardHoverWidget::isClosing() const
{
    return fadeOut->state() == QAbstractAnimation::Running;
}

// ---------- Event filter ----------

bool CardHoverWidget::eventFilter(QObject *obj, QEvent *event)
{
    // Handle double-click on name label to start editing annotation
    if (obj == nameLabel && event->type() == QEvent::MouseButtonDblClick) {
        nameLabel->setVisible(false);
        annotationLabel->setVisible(false);
        nameEdit->setVisible(true);
        nameEdit->setText(currentCard ? currentCard->getAnnotation() : QString());
        nameEdit->setFocus();
        nameEdit->selectAll();
        return true;
    }

    // Handle click on card image to select the card in game
    if (obj == pic && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton && currentCard) {
            if (auto *scene = qobject_cast<GameScene *>(currentCard->scene())) {
                scene->setActiveCard(currentCard);
            }
            currentCard->setSelected(true);
        }
    }

    // The P/T trigger is the printed P/T area, not just the pill: hovering that quadrant of
    // the art lifts the control rows out. Driving it from pic's mouse moves (instead of an
    // invisible child widget over the picture) keeps click-to-select working underneath.
    if (obj == pic) {
        if (event->type() == QEvent::MouseMove) {
            const auto *me = static_cast<QMouseEvent *>(event);
            const QPoint pos = me->position().toPoint();
            if (ptOpenRegion().contains(pos)) {
                setPTExpanded(true);
            } else if (!isCursorOverPTArea()) {
                setPTExpanded(false);
            }
        } else if (event->type() == QEvent::Leave && !isCursorOverPTArea()) {
            setPTExpanded(false);
        }
    }

    // P/T overlay: hovering the P/T area lifts the control rows out of the pill. The
    // filter sits on the whole subtree so moving onto a button keeps it expanded.
    const auto *ptWidget = qobject_cast<const QWidget *>(obj);
    if (ptWidget && ptOverlay && (ptWidget == ptOverlay || ptOverlay->isAncestorOf(ptWidget))) {
        if (event->type() == QEvent::Enter) {
            setPTExpanded(true);
        } else if (event->type() == QEvent::Leave) {
            // Moving onto a child widget fires Leave on the parent too; only collapse once
            // the cursor has left both the overlay and the printed P/T area.
            if (!isCursorOverPTArea()) {
                setPTExpanded(false);
            }
        }
    }

    return QFrame::eventFilter(obj, event);
}

// ---------- Mouse tracking on the popup itself ----------

void CardHoverWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        applyThemeStyle();
    }
    QFrame::changeEvent(event);
}

void CardHoverWidget::enterEvent(QEnterEvent *event)
{
    QFrame::enterEvent(event);
    emit hoverEntered();
}

void CardHoverWidget::leaveEvent(QEvent *event)
{
    QFrame::leaveEvent(event);
    // Moving the cursor onto a child widget also triggers leaveEvent on this widget;
    // only report a real departure from the popup.
    if (rect().contains(mapFromGlobal(QCursor::pos()))) {
        return;
    }
    emit hoverLeft();
}
