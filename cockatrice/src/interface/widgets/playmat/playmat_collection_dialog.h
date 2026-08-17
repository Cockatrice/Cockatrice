#ifndef COCKATRICE_PLAYMAT_COLLECTION_DIALOG_H
#define COCKATRICE_PLAYMAT_COLLECTION_DIALOG_H

#include <QDialog>
#include <libcockatrice/utility/playmat_params.h>

class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;

/**
 * @brief Dialog for editing the user-level playmat collection.
 *
 * The collection is the fallback used when a deck has no playmat of its own.
 * It supports multiple entries and a pick mode (always first / round-robin /
 * random). The dialog edits a working copy and writes it to the settings only
 * when accepted.
 */
class PlaymatCollectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlaymatCollectionDialog(QWidget *parent = nullptr);

    void accept() override;

private slots:
    void addPlaymat();
    void editPlaymat();
    void removePlaymat();
    void movePlaymatUp();
    void movePlaymatDown();
    void selectionChanged();

private:
    void setupUi();
    void retranslateUi();
    int currentRow() const;

    QList<PlaymatResolution> playmats; ///< Working copy edited by the dialog.
    QListWidget *playmatList;
    QComboBox *modeCombo;
    QLabel *modeLabel;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *removeButton;
    QPushButton *moveUpButton;
    QPushButton *moveDownButton;
};

#endif // COCKATRICE_PLAYMAT_COLLECTION_DIALOG_H
