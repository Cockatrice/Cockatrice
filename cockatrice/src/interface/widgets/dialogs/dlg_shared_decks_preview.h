#ifndef COCKATRICE_DLG_SHARED_DECKS_PREVIEW_H
#define COCKATRICE_DLG_SHARED_DECKS_PREVIEW_H

#include <QDialog>
#include <QList>

class FlowWidget;
class QCloseEvent;
class QLabel;
class QPushButton;
class ServerInfo_DeckShareItem;
class SharedDeckPreviewWidget;
class CardDatabaseQuerier;

/**
 * @brief Non-modal preview of the decks contained in a shared-deck link.
 *
 * Lets the user pick which of the shared decks to open before anything is
 * downloaded. Emits openRequested with the ids of the chosen decks, or
 * cancelled when the user closes the dialog without choosing. Once the user
 * picks, the dialog switches into a "downloading" state: the tiles and open
 * buttons are disabled, a progress label shows the current download and Cancel
 * stays functional so the download can be aborted.
 */
class DlgSharedDecksPreview : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSharedDecksPreview(QWidget *parent,
                                   const CardDatabaseQuerier *querier,
                                   const QString &shareName,
                                   qint64 expiresAt,
                                   const QString &serverText,
                                   const QList<ServerInfo_DeckShareItem> &items);

    void setDownloadProgress(int done, int total, const QString &currentDeckName);

public slots:
    void setDownloading(bool downloading);

signals:
    void openRequested(const QList<int> &itemIds);
    void cancelled();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openSelected();
    void openAll();
    void updateOpenSelectedEnabled();
    void onCancel();

private:
    QList<int> selectedItemIds() const;

    FlowWidget *flowWidget;
    QList<SharedDeckPreviewWidget *> tiles;
    QList<int> itemIds;
    QPushButton *openSelectedButton;
    QPushButton *openAllButton;
    QLabel *downloadStatusLabel;
    bool resultEmitted = false;
    bool downloadInProgress = false;
};

#endif // COCKATRICE_DLG_SHARED_DECKS_PREVIEW_H