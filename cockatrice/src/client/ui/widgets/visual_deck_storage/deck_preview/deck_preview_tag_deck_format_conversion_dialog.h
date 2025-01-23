#ifndef DECK_PREVIEW_TAG_DECK_FORMAT_CONVERSION_DIALOG_H
#define DECK_PREVIEW_TAG_DECK_FORMAT_CONVERSION_DIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

class DeckPreviewTagDeckFormatConversionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeckPreviewTagDeckFormatConversionDialog(QWidget *parent = nullptr);
    void retranslateUi();

    bool dontAskAgain() const;

private:
    QVBoxLayout *layout;
    QLabel *label;
    QCheckBox *dontAskAgainCheckbox;
    QDialogButtonBox *buttonBox;

    Q_DISABLE_COPY(DeckPreviewTagDeckFormatConversionDialog)
};

#endif // DECK_PREVIEW_TAG_DECK_FORMAT_CONVERSION_DIALOG_H
