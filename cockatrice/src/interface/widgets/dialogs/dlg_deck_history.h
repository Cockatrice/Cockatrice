/**
 * @file dlg_deck_history.h
 * @brief Browser for the stored versions of one server deck.
 */

#ifndef DLG_DECK_HISTORY_H
#define DLG_DECK_HISTORY_H

#include <QDialog>
#include <QList>
#include <QString>

class AbstractClient;
class CommandContainer;
class QLabel;
class QListWidget;
class QPushButton;
class Response;

/** @brief One stored deck version for display. */
struct DeckHistoryVersion
{
    unsigned int versionNo = 0;
    qint64 createdAtSecs = 0;
    QString createdByClientId;
    QString hash;
    QString content;
};

/**
 * @class DlgDeckHistory
 * @brief Lists stored versions with a diff preview against the current deck.
 *
 * Restoring uploads the selected version as the new current content through
 * the regular deck-upload path, so it becomes a version itself instead of
 * rewriting history. Rejected dialogs restore nothing.
 */
class DlgDeckHistory : public QDialog
{
    Q_OBJECT
public:
    DlgDeckHistory(int deckId, const QString &deckName, int limit, AbstractClient *client, QWidget *parent = nullptr);
    void retranslateUi();

signals:
    void restoreRequested(int deckId, const QString &content);

private slots:
    void onHistoryFinished(const Response &response, const CommandContainer &commandContainer);
    void onCurrentFinished(const Response &response, const CommandContainer &commandContainer);
    void onSelectionChanged();
    void onRestoreClicked();

private:
    void renderVersions();
    void renderPreview();

    int deckId = -1;
    QString deckName;
    int limit = 20;
    AbstractClient *client = nullptr;
    QList<DeckHistoryVersion> versions;
    QString currentContent;
    bool historyArrived = false;
    bool currentArrived = false;

    QLabel *detailLabel;
    QListWidget *versionsList;
    QListWidget *previewList;
    QPushButton *restoreButton;
};

#endif // DLG_DECK_HISTORY_H
