#include "card_picture_loader_status_bar.h"

#include "../widgets/quick_settings/settings_button_widget.h"
#include "../widgets/quick_settings/settings_popup_widget.h"
#include "card_picture_loader_request_status_display_widget.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QList>
#include <QProgressBar>
#include <QSharedPointer>
#include <QTimer>
#include <QUrl>
#include <QtGlobal>

class ExactCard;

CardPictureLoaderStatusBar::CardPictureLoaderStatusBar(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    progressBar = new QProgressBar(this);
    progressBar->setMaximum(0);
    progressBar->setFormat("%v/%m");
    layout->addWidget(progressBar);

    loadLog = new SettingsButtonWidget(this);
    layout->addWidget(loadLog);

    cleaner = new QTimer(this);
    cleaner->setInterval(1000);
    connect(cleaner, &QTimer::timeout, this, &CardPictureLoaderStatusBar::cleanOldEntries);
    cleaner->start();

    setLayout(layout);
}

void CardPictureLoaderStatusBar::cleanOldEntries()
{
    if (!loadLog || !loadLog->popup) {
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    // Removing an entry mutates the log, so walk a copy of it.
    const QList<CardPictureLoaderRequestStatusDisplayWidget *> entries = logEntries;
    for (CardPictureLoaderRequestStatusDisplayWidget *statusDisplayWidget : entries) {
        statusDisplayWidget->queryElapsedSeconds();
        if (!statusDisplayWidget->getFinished()) {
            continue;
        }
        if (QDateTime::fromString(statusDisplayWidget->getStartTime()).secsTo(now) > LOG_ENTRY_LIFETIME_SECS) {
            removeLogEntry(statusDisplayWidget);
        }
    }
}

void CardPictureLoaderStatusBar::addQueuedImageLoad(const QUrl &url, const ExactCard &card, const QString &setName)
{
    if (logEntries.size() >= MAX_LOG_ENTRIES) {
        trimLog();
    }

    auto *entry = new CardPictureLoaderRequestStatusDisplayWidget(loadLog, url, card, setName);
    loadLog->addSettingsWidget(entry);
    logEntries.append(entry);
    entriesByUrl.insert(entry->getUrl(), entry);
    progressBar->setMaximum(progressBar->maximum() + 1);
}

void CardPictureLoaderStatusBar::addSuccessfulImageLoad(const QUrl &url)
{
    const QList<CardPictureLoaderRequestStatusDisplayWidget *> entries = entriesByUrl.values(url.toString());
    if (entries.isEmpty()) {
        // The entry may already have been dropped from the log; the download
        // itself still succeeded, there is just nothing left to mark.
        return;
    }

    int finishedEntries = 0;
    for (CardPictureLoaderRequestStatusDisplayWidget *statusDisplayWidget : entries) {
        if (statusDisplayWidget->getFinished()) {
            continue;
        }
        statusDisplayWidget->queryElapsedSeconds();
        statusDisplayWidget->setFinished();
        ++finishedEntries;
    }

    // The bar counts the entries currently in the log, so only entries that are
    // still listed move it.
    progressBar->setValue(progressBar->value() + finishedEntries);
}

void CardPictureLoaderStatusBar::trimLog()
{
    if (logEntries.isEmpty()) {
        return;
    }

    // Prefer evicting an entry that has already finished; only fall back to an
    // in-flight request when the whole log is still pending.
    CardPictureLoaderRequestStatusDisplayWidget *toRemove = nullptr;
    for (CardPictureLoaderRequestStatusDisplayWidget *entry : logEntries) {
        if (entry->getFinished()) {
            toRemove = entry;
            break;
        }
    }

    removeLogEntry(toRemove != nullptr ? toRemove : logEntries.constFirst());
}

void CardPictureLoaderStatusBar::removeLogEntry(CardPictureLoaderRequestStatusDisplayWidget *entry)
{
    if (entry == nullptr) {
        return;
    }

    logEntries.removeOne(entry);
    entriesByUrl.remove(entry->getUrl(), entry);

    // The progress bar's maximum counts the entries currently in the log and
    // its value the finished ones, so dropping an entry moves both counters back.
    // Move the value first: a maximum of 0 turns the bar into a busy indicator and
    // resets its value.
    if (entry->getFinished()) {
        progressBar->setValue(qMax(0, progressBar->value() - 1));
    }
    progressBar->setMaximum(qMax(0, progressBar->maximum() - 1));

    loadLog->removeSettingsWidget(entry);
}
