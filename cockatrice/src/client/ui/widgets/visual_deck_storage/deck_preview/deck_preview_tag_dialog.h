#ifndef DECK_PREVIEW_TAG_DIALOG_H
#define DECK_PREVIEW_TAG_DIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>

class DeckPreviewTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeckPreviewTagDialog(const QStringList &knownTags,
                                  const QStringList &activeTags,
                                  QWidget *parent = nullptr);
    QStringList getActiveTags() const;

private slots:
    void addTag();
    void onCheckboxStateChanged();
    void retranslateUi();

private:
    QLabel *instructionLabel;
    QListWidget *tagListView_;
    QLineEdit *newTagInput_;
    QPushButton *addTagButton_;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QStringList activeTags_;
};

#endif // DECK_PREVIEW_TAG_DIALOG_H
