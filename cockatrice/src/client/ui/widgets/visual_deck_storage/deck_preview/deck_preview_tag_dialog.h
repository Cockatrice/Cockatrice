#ifndef DECK_PREVIEW_TAG_DIALOG_H
#define DECK_PREVIEW_TAG_DIALOG_H

#include <QDialog>
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

private:
    QListWidget *tagListView_;
    QLineEdit *newTagInput_;
    QPushButton *addTagButton_;
    QStringList activeTags_;
};

#endif // DECK_PREVIEW_TAG_DIALOG_H
