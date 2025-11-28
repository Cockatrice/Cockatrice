/**
 * @file deck_preview_tag_dialog.h
 * @ingroup VisualDeckPreviewWidgets
 * @ingroup Dialogs
 * @brief TODO: Document this.
 */

#ifndef DECK_PREVIEW_TAG_DIALOG_H
#define DECK_PREVIEW_TAG_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

class DeckPreviewTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeckPreviewTagDialog(const QStringList &knownTags,
                                  const QStringList &activeTags,
                                  QWidget *parent = nullptr);
    [[nodiscard]] QStringList getActiveTags() const;
    void filterTags(const QString &text);

private slots:
    void addTag();
    void onCheckboxStateChanged();
    void retranslateUi();
    void refreshTagList();

private:
    QVBoxLayout *mainLayout;
    QLabel *instructionLabel;
    QListWidget *tagListView;
    QLineEdit *filterInput;
    QHBoxLayout *addTagLayout;
    QLineEdit *newTagInput;
    QPushButton *addTagButton;
    QHBoxLayout *buttonLayout;
    QPushButton *okButton;
    QPushButton *editButton;
    QPushButton *cancelButton;
    QStringList activeTags;
    QStringList knownTags_;
};

#endif // DECK_PREVIEW_TAG_DIALOG_H
