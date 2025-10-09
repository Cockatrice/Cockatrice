/**
 * @file dlg_load_deck.h
 * @ingroup LocalDeckStorageDialogs
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef DLG_LOAD_DECK_H
#define DLG_LOAD_DECK_H

#include <QFileDialog>

/**
 * The file dialog for "Load Deck" operations.
 * Handles remembering the most recently used deck loading directory.
 */
class DlgLoadDeck : public QFileDialog
{
    Q_OBJECT

    void actAccepted();

public:
    explicit DlgLoadDeck(QWidget *parent = nullptr);
};

#endif // DLG_LOAD_DECK_H
