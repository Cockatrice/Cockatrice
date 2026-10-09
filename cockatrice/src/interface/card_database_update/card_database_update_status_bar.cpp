#include "card_database_update_status_bar.h"

#include <QCoreApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QtGlobal>
#include <climits>

namespace
{
/**
 * @brief Translated name of one updater stage.
 * @param fallback shown when the stage is unknown or not reported yet
 */
QString stageName(CardUpdateStage stage, const QString &fallback)
{
    switch (stage) {
        case CardUpdateStage::Download:
            return QCoreApplication::translate("CardDatabaseUpdateStatusBar", "Downloading");
        case CardUpdateStage::Scan:
            return QCoreApplication::translate("CardDatabaseUpdateStatusBar", "Parsing");
        case CardUpdateStage::Import:
            return QCoreApplication::translate("CardDatabaseUpdateStatusBar", "Importing");
        case CardUpdateStage::Unknown:
            return fallback;
    }
    return fallback;
}
} // namespace

CardDatabaseUpdateStatusBar::CardDatabaseUpdateStatusBar(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    stageLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    progressBar->setMinimumWidth(120);

    layout->addWidget(stageLabel);
    layout->addWidget(progressBar);

    setLayout(layout);
    refresh();
    hide();
}

QString CardDatabaseUpdateStatusBar::statusText() const
{
    if (!active) {
        return {};
    }
    const QString name = stageName(progress.stage, tr("Updating the card database"));
    if (progress.total <= 0) {
        return name;
    }
    const int percent = static_cast<int>((100.0 * progress.done) / progress.total);
    return tr("%1 (%2%)").arg(name).arg(percent);
}

void CardDatabaseUpdateStatusBar::updateStarted()
{
    active = true;
    progress = {};
    refresh();
    show();
}

void CardDatabaseUpdateStatusBar::updateProgress(const CardUpdateProgress &progress)
{
    active = true;
    this->progress = progress;
    refresh();
}

void CardDatabaseUpdateStatusBar::updateFinished()
{
    active = false;
    progress = {};
    refresh();
    hide();
}

void CardDatabaseUpdateStatusBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        refresh();
    }
    QWidget::changeEvent(event);
}

void CardDatabaseUpdateStatusBar::refresh()
{
    const QString name = stageName(progress.stage, tr("Updating the card database"));
    stageLabel->setText(name);

    const qint64 cappedTotal = qMin<qint64>(progress.total, INT_MAX);
    const qint64 cappedDone = qMin<qint64>(progress.done, INT_MAX);
    const int minimum = 0;
    const int maximum = static_cast<int>(cappedTotal);
    if (progressBar->minimum() != minimum || progressBar->maximum() != maximum) {
        // A zero range is the busy indicator, used until the first report arrives.
        progressBar->setRange(minimum, maximum);
    }
    progressBar->setValue(static_cast<int>(cappedDone));

    if (active && progress.total > 0) {
        const int percent = static_cast<int>((100.0 * progress.done) / progress.total);
        setToolTip(tr("Card database update in progress: %1 (%2%)").arg(name).arg(percent));
    } else if (active) {
        setToolTip(tr("Card database update in progress."));
    } else {
        setToolTip({});
    }
}
