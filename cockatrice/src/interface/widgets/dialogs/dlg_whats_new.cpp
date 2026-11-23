#include "dlg_whats_new.h"

#include "../../../client/features/feature_intro.h"
#include "../../../client/features/feature_intro_service.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <libcockatrice/settings/feature_intro_settings.h>
#include <libcockatrice/utility/qt_utils.h>

DlgWhatsNew::DlgWhatsNew(FeatureIntroService *service, QWidget *parent) : QDialog(parent), service(service)
{
    setObjectName(QStringLiteral("whatsNewDialog"));
    setWindowTitle(tr("What's New"));
    resize(560, 520);

    auto *outer = QtUtils::makeLayout<QVBoxLayout>(this, 12, 16);

    auto *lead =
        new QLabel(tr("Recent additions to Cockatrice, and the settings that control them. Every feature can be "
                      "tried or turned off here, without leaving the dialog."),
                   this);
    lead->setObjectName(QStringLiteral("whatsNewLead"));
    lead->setWordWrap(true);
    outer->addWidget(lead);

    auto *scrollArea = QtUtils::makeScrollArea(this);
    scrollArea->setObjectName(QStringLiteral("whatsNewScroll"));
    auto *content = new QWidget;
    entriesLayout = QtUtils::makeLayout<QVBoxLayout>(content, 12, 0);
    scrollArea->setWidget(content);
    outer->addWidget(scrollArea, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttons->setObjectName(QStringLiteral("whatsNewButtons"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    outer->addWidget(buttons);

    rebuild();
}

QString DlgWhatsNew::stateText(const FeatureIntro &intro) const
{
    switch (service->decisionFor(intro.id)) {
        case FeatureIntroDecision::Pending:
            return tr("You have not tried this yet.");
        case FeatureIntroDecision::Accepted:
            return tr("You are using it.");
        case FeatureIntroDecision::Declined:
            return intro.declinedOutcome.isEmpty() ? tr("You are using the classic behavior.") : intro.declinedOutcome;
        case FeatureIntroDecision::Dismissed:
            return tr("Hidden for now. You can still turn it on whenever you like.");
        case FeatureIntroDecision::Baseline:
            return tr("This was already part of your install.");
        case FeatureIntroDecision::Skipped:
            return tr("Already configured in the settings.");
    }
    return {};
}

void DlgWhatsNew::rebuild()
{
    QtUtils::clearLayoutRec(entriesLayout, QtUtils::DeletePolicy::Immediate);

    const QVector<FeatureIntro> &catalog = featureIntroCatalog();
    if (catalog.isEmpty()) {
        auto *empty = new QLabel(tr("No new features to show."), entriesLayout->parentWidget());
        empty->setWordWrap(true);
        entriesLayout->addWidget(empty);
        entriesLayout->addStretch(1);
        return;
    }

    for (const FeatureIntro &intro : catalog) {
        addEntry(entriesLayout, intro);
    }
    entriesLayout->addStretch(1);
}

void DlgWhatsNew::addEntry(QVBoxLayout *layout, const FeatureIntro &intro)
{
    auto *card = new QFrame(layout->parentWidget());
    card->setObjectName(QStringLiteral("whatsNewEntry"));
    card->setFrameShape(QFrame::StyledPanel);
    auto *cardLayout = QtUtils::makeLayout<QVBoxLayout>(card, 8, 12);

    auto *titleRow = QtUtils::makeLayout<QHBoxLayout>(nullptr, 8, 0);
    auto *title = new QLabel(intro.title, card);
    title->setObjectName(QStringLiteral("whatsNewEntryTitle"));
    title->setFont(QtUtils::boldFont(title->font()));
    title->setTextInteractionFlags(Qt::TextSelectableByMouse);
    titleRow->addWidget(title, 1);
    if (service->isNew(intro.id)) {
        auto *badge = new QLabel(tr("New"), card);
        badge->setObjectName(QStringLiteral("whatsNewEntryBadge"));
        badge->setFont(QtUtils::boldFont(badge->font()));
        QPalette badgePalette = badge->palette();
        badgePalette.setColor(QPalette::WindowText, badge->palette().color(QPalette::Highlight));
        badge->setPalette(badgePalette);
        titleRow->addWidget(badge, 0, Qt::AlignTop);
    }
    cardLayout->addLayout(titleRow);

    auto *body = new QLabel(intro.body, card);
    body->setObjectName(QStringLiteral("whatsNewEntryBody"));
    body->setWordWrap(true);
    body->setTextInteractionFlags(Qt::TextSelectableByMouse);
    cardLayout->addWidget(body);

    auto *status = new QLabel(stateText(intro), card);
    status->setObjectName(QStringLiteral("whatsNewEntryState"));
    status->setWordWrap(true);
    // Muted, not disabled and not PlaceholderText: a disabled label drops to the
    // palette's disabled text and a placeholder is tuned for empty inputs, so both
    // leave this line at roughly 3.5:1 against the card. This line answers "what
    // happens if I do nothing?", so it has to stay readable: 65% of the text colour
    // is ~6:1 on a light theme and follows any dark theme automatically.
    QPalette statusPalette = status->palette();
    QColor mutedText = status->palette().color(QPalette::WindowText);
    mutedText.setAlphaF(0.65);
    statusPalette.setColor(QPalette::WindowText, mutedText);
    status->setPalette(statusPalette);
    cardLayout->addWidget(status);

    auto *actions = QtUtils::makeLayout<QHBoxLayout>(nullptr, 8, 0);
    const FeatureIntroDecision decision = service->decisionFor(intro.id);
    const QString id = intro.id;

    if (decision == FeatureIntroDecision::Pending || decision == FeatureIntroDecision::Declined ||
        decision == FeatureIntroDecision::Dismissed) {
        auto *accept = new QPushButton(intro.acceptLabel, card);
        accept->setObjectName(QStringLiteral("whatsNewEntryAccept"));
        accept->setAutoDefault(false);
        connect(accept, &QPushButton::clicked, this, [this, id] {
            service->decide(id, FeatureIntroDecision::Accepted);
            // Rebuild after the click returns: the sender is one of the widgets the
            // rebuild deletes.
            QTimer::singleShot(0, this, [this] { rebuild(); });
        });
        actions->addWidget(accept);
    }

    if (decision == FeatureIntroDecision::Pending || decision == FeatureIntroDecision::Accepted) {
        auto *decline = new QPushButton(intro.declineLabel, card);
        decline->setObjectName(QStringLiteral("whatsNewEntryDecline"));
        decline->setAutoDefault(false);
        connect(decline, &QPushButton::clicked, this, [this, id] {
            service->decide(id, FeatureIntroDecision::Declined);
            QTimer::singleShot(0, this, [this] { rebuild(); });
        });
        actions->addWidget(decline);
    }

    actions->addStretch(1);

    auto *settings = new QPushButton(tr("Settings…"), card);
    settings->setObjectName(QStringLiteral("whatsNewEntrySettings"));
    settings->setAutoDefault(false);
    settings->setFlat(true);
    const QString pageKey = intro.settingsPage;
    const QString query = intro.settingsQuery;
    connect(settings, &QPushButton::clicked, this, [this, pageKey, query] { emit settingsRequested(pageKey, query); });
    actions->addWidget(settings);

    cardLayout->addLayout(actions);
    layout->addWidget(card);
}
