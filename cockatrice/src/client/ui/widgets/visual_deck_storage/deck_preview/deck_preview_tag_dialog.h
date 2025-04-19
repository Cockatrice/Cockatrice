#ifndef DECK_PREVIEW_TAG_DIALOG_H
#define DECK_PREVIEW_TAG_DIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

class DeckPreviewTagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeckPreviewTagDialog(const QStringList &knownTags,
                                  const QStringList &activeTags,
                                  QWidget *parent = nullptr);
    QStringList getActiveTags() const;
    void filterTags(const QString &text);

private slots:
    void addTag();
    void onCheckboxStateChanged();
    void retranslateUi();

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
    QPushButton *cancelButton;
    QStringList activeTags;
};

#endif // DECK_PREVIEW_TAG_DIALOG_H
