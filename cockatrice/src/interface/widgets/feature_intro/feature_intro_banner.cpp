#include "feature_intro_banner.h"

#include <QFont>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/utility/qt_utils.h>

namespace
{
/// Hairline accent down the left edge; the same reading as other inline notices.
constexpr int kAccentWidth = 3;
constexpr int kContentMargin = 12;
} // namespace

FeatureIntroBanner::FeatureIntroBanner(const FeatureIntro &intro, QWidget *parent) : QFrame(parent), intro(intro)
{
    setObjectName(QStringLiteral("featureIntroBanner"));
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::AlternateBase);

    auto *outer = QtUtils::makeLayout<QHBoxLayout>(this, 0, 0);

    auto *accent = new QWidget(this);
    accent->setObjectName(QStringLiteral("featureIntroBannerAccent"));
    accent->setFixedWidth(kAccentWidth);
    accent->setAutoFillBackground(true);
    accent->setBackgroundRole(QPalette::Highlight);
    outer->addWidget(accent);

    auto *content = QtUtils::makeLayout<QVBoxLayout>(nullptr, 8, kContentMargin);
    outer->addLayout(content, 1);

    auto *titleRow = QtUtils::makeLayout<QHBoxLayout>(nullptr, 8, 0);
    auto *titleLabel = new QLabel(intro.title, this);
    titleLabel->setObjectName(QStringLiteral("featureIntroBannerTitle"));
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    titleRow->addWidget(titleLabel, 1);

    auto *closeButton = new QToolButton(this);
    closeButton->setObjectName(QStringLiteral("featureIntroBannerClose"));
    closeButton->setText(QStringLiteral("✕"));
    closeButton->setAutoRaise(true);
    closeButton->setToolTip(tr("Don't show this again"));
    titleRow->addWidget(closeButton, 0, Qt::AlignTop);
    titleCloseButton = closeButton;
    content->addLayout(titleRow);

    auto *bodyLabel = new QLabel(intro.body, this);
    bodyLabel->setObjectName(QStringLiteral("featureIntroBannerBody"));
    bodyLabel->setWordWrap(true);
    bodyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    content->addWidget(bodyLabel);

    actionLayout = QtUtils::makeLayout<QHBoxLayout>(nullptr, 8, 0);
    content->addLayout(actionLayout);

    connect(closeButton, &QToolButton::clicked, this, &FeatureIntroBanner::dismissed);

    // Escape is "later", not "never", and only while the banner has the focus, so it
    // cannot swallow the game's own Escape handling.
    auto *laterShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    laterShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(laterShortcut, &QShortcut::activated, this, &FeatureIntroBanner::laterRequested);

    showInvitation();
}

void FeatureIntroBanner::showInvitation()
{
    QtUtils::clearLayoutRec(actionLayout);

    auto *accept = new QPushButton(intro.acceptLabel, this);
    accept->setObjectName(QStringLiteral("featureIntroBannerAccept"));
    accept->setAutoDefault(false);
    accept->setDefault(true);

    auto *decline = new QPushButton(intro.declineLabel, this);
    decline->setObjectName(QStringLiteral("featureIntroBannerDecline"));
    decline->setAutoDefault(false);

    auto *settings = new QPushButton(tr("Settings…"), this);
    settings->setObjectName(QStringLiteral("featureIntroBannerSettings"));
    settings->setAutoDefault(false);
    settings->setFlat(true);

    auto *later = new QPushButton(tr("Later"), this);
    later->setObjectName(QStringLiteral("featureIntroBannerLater"));
    later->setAutoDefault(false);
    later->setFlat(true);
    later->setToolTip(tr("Ask me another time"));

    connect(accept, &QPushButton::clicked, this, &FeatureIntroBanner::accepted);
    connect(decline, &QPushButton::clicked, this, &FeatureIntroBanner::declined);
    connect(settings, &QPushButton::clicked, this, &FeatureIntroBanner::settingsRequested);
    connect(later, &QPushButton::clicked, this, &FeatureIntroBanner::laterRequested);

    actionLayout->addWidget(accept);
    actionLayout->addWidget(decline);
    actionLayout->addStretch(1);
    actionLayout->addWidget(settings);
    actionLayout->addWidget(later);
}

void FeatureIntroBanner::showOutcome(FeatureIntroAction undo)
{
    // Deferred delete is required here: the caller runs this from the clicked()
    // handler of one of the buttons in this layout. A widget removed from a layout
    // stays visible at its old geometry until the event loop gets to it, painting
    // under the row that replaces it, so hide them before handing the layout over.
    for (int i = 0; i < actionLayout->count(); ++i) {
        if (QWidget *stale = actionLayout->itemAt(i)->widget()) {
            stale->hide();
        }
    }
    QtUtils::clearLayoutRec(actionLayout);

    // The decision is made now, so the "don't show this again" X would only let a
    // click replace it with `Dismissed`. Closing this row keeps what the user chose.
    if (titleCloseButton) {
        titleCloseButton->setVisible(false);
    }

    auto *outcome = new QLabel(intro.declinedOutcome, this);
    outcome->setObjectName(QStringLiteral("featureIntroBannerOutcome"));
    outcome->setWordWrap(true);
    actionLayout->addWidget(outcome, 1);

    if (undo) {
        auto *undoButton = new QPushButton(tr("Undo"), this);
        undoButton->setObjectName(QStringLiteral("featureIntroBannerUndo"));
        undoButton->setAutoDefault(false);
        connect(undoButton, &QPushButton::clicked, this, [this, undo] {
            undo();
            emit undone();
        });
        actionLayout->addWidget(undoButton);
    }

    auto *settings = new QPushButton(tr("Settings…"), this);
    settings->setObjectName(QStringLiteral("featureIntroBannerSettingsAfter"));
    settings->setAutoDefault(false);
    settings->setFlat(true);
    connect(settings, &QPushButton::clicked, this, &FeatureIntroBanner::settingsRequested);
    actionLayout->addWidget(settings);

    auto *closeButton = new QPushButton(tr("Close"), this);
    closeButton->setObjectName(QStringLiteral("featureIntroBannerCloseAfter"));
    closeButton->setAutoDefault(false);
    closeButton->setFlat(true);
    // Removes the row without a new decision: `accepted` here would record
    // "you are using it" while the settings still say classic.
    connect(closeButton, &QPushButton::clicked, this, &FeatureIntroBanner::laterRequested);
    actionLayout->addWidget(closeButton);
}
