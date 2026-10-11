#include "dlg_sync_onboarding.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

DlgSyncOnboarding::DlgSyncOnboarding(int _localCount, int _serverCount, QWidget *parent)
    : QDialog(parent), localCount(_localCount), serverCount(_serverCount)
{
    auto *layout = new QVBoxLayout(this);

    detailLabel = new QLabel(this);
    detailLabel->setWordWrap(true);
    mergeRadio = new QRadioButton(this);
    uploadRadio = new QRadioButton(this);
    downloadRadio = new QRadioButton(this);
    separateRadio = new QRadioButton(this);
    mergeRadio->setChecked(true);

    layout->addWidget(detailLabel);
    layout->addWidget(mergeRadio);
    layout->addWidget(uploadRadio);
    layout->addWidget(downloadRadio);
    layout->addWidget(separateRadio);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    retranslateUi();
}

void DlgSyncOnboarding::retranslateUi()
{
    setWindowTitle(tr("Sync decks to this server"));
    detailLabel->setText(tr("You have %1 local decks and %2 decks stored on this server. How should they be combined?")
                             .arg(localCount)
                             .arg(serverCount));
    mergeRadio->setText(tr("Merge: keep everything, sync both ways (recommended)"));
    uploadRadio->setText(tr("Upload mine: push local decks to the server"));
    downloadRadio->setText(tr("Download server: take the server decks"));
    separateRadio->setText(tr("Keep separate: do not sync with this server"));
}

DeckSyncMode DlgSyncOnboarding::selectedMode() const
{
    if (uploadRadio->isChecked()) {
        return DeckSyncMode::UploadOnly;
    }
    if (downloadRadio->isChecked()) {
        return DeckSyncMode::DownloadOnly;
    }
    if (separateRadio->isChecked()) {
        return DeckSyncMode::Separate;
    }
    return DeckSyncMode::TwoWay;
}
