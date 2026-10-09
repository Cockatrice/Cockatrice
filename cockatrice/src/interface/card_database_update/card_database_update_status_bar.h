#ifndef CARD_DATABASE_UPDATE_STATUS_BAR_H
#define CARD_DATABASE_UPDATE_STATUS_BAR_H

#include "card_update_progress_parser.h"

#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class QEvent;
class QHBoxLayout;
class QLabel;
class QProgressBar;

/**
 * @class CardDatabaseUpdateStatusBar
 * @brief Status bar widget showing what the card database updater (Oracle) is doing.
 *
 * The updater runs as a separate process, so this widget is the in-client view of
 * its progress: it appears when an update starts, tracks the download/scan/import
 * stages reported by the updater, and disappears when the update finishes.
 */
class CardDatabaseUpdateStatusBar : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a hidden progress widget.
     * @param parent Parent widget
     */
    explicit CardDatabaseUpdateStatusBar(QWidget *parent = nullptr);

    /**
     * @brief Human readable description of the running stage, e.g. "Downloading (42%)".
     * @return the description, or an empty string when no update is running
     */
    QString statusText() const;

public slots:
    /**
     * @brief Shows the widget and resets it to the indeterminate state.
     */
    void updateStarted();

    /**
     * @brief Shows the progress of one update stage.
     */
    void updateProgress(const CardUpdateProgress &progress);

    /**
     * @brief Hides the widget and clears the reported progress.
     */
    void updateFinished();

protected:
    void changeEvent(QEvent *event) override;

private:
    void refresh();

    QHBoxLayout *layout;       ///< Horizontal layout holding the stage label and the progress bar
    QLabel *stageLabel;        ///< Name of the stage currently running
    QProgressBar *progressBar; ///< Progress within the current stage
    CardUpdateStage stage = CardUpdateStage::Unknown; ///< Last reported stage
    qint64 done = 0;                                  ///< Last reported progress numerator
    qint64 total = 0;                                 ///< Last reported progress denominator, 0 when unknown
    bool active = false;                              ///< Whether an update is currently running
};

#endif // CARD_DATABASE_UPDATE_STATUS_BAR_H
