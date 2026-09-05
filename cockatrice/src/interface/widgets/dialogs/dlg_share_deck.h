/**
 * @file dlg_share_deck.h
 * @ingroup Dialogs
 */
//! \todo Document this file.

#ifndef DLG_SHARE_DECK_H
#define DLG_SHARE_DECK_H

#include <QDialog>
#include <QSharedPointer>

class AbstractClient;
class CommandContainer;
class DeckList;
class QDialogButtonBox;
class QLineEdit;
class Response;

/**
 * @brief Slim dialog to create a temporary share for the deck open in the editor.
 *
 * Asks for a share name, sends Command_DeckShareCreate for the single inline
 * deck, and copies the resulting link to the clipboard.
 */
class DlgShareDeck : public QDialog
{
    Q_OBJECT
public:
    DlgShareDeck(AbstractClient *_client, const QSharedPointer<DeckList> &_deck, QWidget *parent = nullptr);

private slots:
    void actShare();
    void shareFinished(const Response &response, const CommandContainer &commandContainer);

private:
    AbstractClient *client;
    QSharedPointer<DeckList> deck;
    QLineEdit *nameEdit;
    QDialogButtonBox *buttonBox;
};

#endif // DLG_SHARE_DECK_H