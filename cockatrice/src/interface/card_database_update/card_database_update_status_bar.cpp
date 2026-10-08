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
 * @param stage one of "download", "scan" or "import"
 * @param fallback shown when the stage is unknown or not reported yet
 */
QString stageName(const QString &stage, const QString &fallback)
{
    if (stage == QLatin1String("download")) {
        return QCoreApplication::translate("CardDatabaseUpdateStatusBar", "Downloading");
    }
    if (stage == QLatin1String("scan")) {
        return QCoreApplication::translate("CardDatabaseUpdateStatusBar", "Parsing");
    }
    if (stage == QLatin1String("import")) {
        return QCoreApplication::translate("CardDatabaseUpdateStatusBar", "Importing");
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
    const QString name = stageName(stage, tr("Updating the card database"));
    if (total <= 0) {
        return name;
    }
    const int percent = static_cast<int>((100.0 * done) / total);
    return tr("%1 (%2%)").arg(name).arg(percent);
}

void CardDatabaseUpdateStatusBar::updateStarted()
{
    active = true;
    stage.clear();
    done = 0;
    total = 0;
    refresh();
    show();
}

void CardDatabaseUpdateStatusBar::updateProgress(const QString &_stage, qint64 _done, qint64 _total)
{
    active = true;
    stage = _stage;
    done = _done;
    total = _total;
    refresh();
}

void CardDatabaseUpdateStatusBar::updateFinished()
{
    active = false;
    stage.clear();
    done = 0;
    total = 0;
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
    const QString name = stageName(stage, tr("Updating the card database"));
    stageLabel->setText(name);

    const qint64 cappedTotal = qMin<qint64>(total, INT_MAX);
    const qint64 cappedDone = qMin<qint64>(done, INT_MAX);
    const int minimum = 0;
    const int maximum = static_cast<int>(cappedTotal);
    if (progressBar->minimum() != minimum || progressBar->maximum() != maximum) {
        // A zero range is the busy indicator, used until the first report arrives.
        progressBar->setRange(minimum, maximum);
    }
    progressBar->setValue(static_cast<int>(cappedDone));

    if (active && total > 0) {
        const int percent = static_cast<int>((100.0 * done) / total);
        setToolTip(tr("Card database update in progress: %1 (%2%)").arg(name).arg(percent));
    } else if (active) {
        setToolTip(tr("Card database update in progress."));
    } else {
        setToolTip({});
    }
}
