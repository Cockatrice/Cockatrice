#include "dlg_shared_decks_preview.h"

#include "../deck_share/shared_deck_preview_widget.h"
#include "../general/layout_containers/flow_widget.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/protocol/pb/serverinfo_deck_share_item.pb.h>

DlgSharedDecksPreview::DlgSharedDecksPreview(QWidget *parent,
                                             const CardDatabaseQuerier *querier,
                                             const QString &shareName,
                                             qint64 expiresAt,
                                             const QString &serverText,
                                             const QList<ServerInfo_DeckShareItem> &items)
    : QDialog(parent)
{
    setWindowTitle(tr("Open shared decks"));
    resize(700, 500);

    auto *mainLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel(tr("Share: %1").arg(shareName.isEmpty() ? tr("Untitled") : shareName), this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    if (!serverText.isEmpty()) {
        mainLayout->addWidget(new QLabel(tr("From %1").arg(serverText), this));
    }

    if (expiresAt > 0) {
        const QString expiryText = QDateTime::fromSecsSinceEpoch(expiresAt).toLocalTime().toString(Qt::TextDate);
        mainLayout->addWidget(new QLabel(tr("This share link expires on %1").arg(expiryText), this));
    }

    downloadStatusLabel = new QLabel(this);
    downloadStatusLabel->setVisible(false);
    mainLayout->addWidget(downloadStatusLabel);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    mainLayout->addWidget(flowWidget, 1);

    for (const ServerInfo_DeckShareItem &item : items) {
        QStringList tags;
        for (const auto &tag : item.tags()) {
            tags.append(QString::fromStdString(tag));
        }

        auto *tile = new SharedDeckPreviewWidget(
            this, querier, QString::fromStdString(item.name()), QString::fromStdString(item.banner_card()),
            QString::fromStdString(item.color_identity()), QString::fromStdString(item.game_format()), tags.join(", "));
        flowWidget->addWidget(tile);
        tiles.append(tile);
        itemIds.append(item.id());
    }

    if (tiles.size() == 1) {
        tiles.first()->setSelected(true);
    }

    auto *buttonBox = new QDialogButtonBox(this);
    openSelectedButton = buttonBox->addButton(tr("Open selected"), QDialogButtonBox::AcceptRole);
    openAllButton = buttonBox->addButton(tr("Open all"), QDialogButtonBox::ActionRole);
    buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        onCancel();
        close();
    });

    // Esc calls QDialog::reject() directly (which hides the dialog without a
    // close event), so route it through the same guarded cancel as the button.
    connect(this, &QDialog::rejected, this, [this]() {
        onCancel();
        close();
    });

    connect(openSelectedButton, &QPushButton::clicked, this, &DlgSharedDecksPreview::openSelected);
    connect(buttonBox, &QDialogButtonBox::clicked, this, [this, buttonBox](QAbstractButton *button) {
        if (buttonBox->buttonRole(button) == QDialogButtonBox::ActionRole) {
            openAll();
        }
    });

    for (SharedDeckPreviewWidget *tile : tiles) {
        connect(tile, &SharedDeckPreviewWidget::selectionToggled, this,
                &DlgSharedDecksPreview::updateOpenSelectedEnabled);
    }
    for (int i = 0; i < tiles.size(); ++i) {
        const int itemId = itemIds.at(i);
        // Double-clicking a tile selects it and opens just that deck.
        connect(tiles.at(i), &SharedDeckPreviewWidget::activated, this, [this, itemId]() {
            resultEmitted = true;
            setDownloading(true);
            emit openRequested(QList<int>{itemId});
        });
    }
    updateOpenSelectedEnabled();
}

QList<int> DlgSharedDecksPreview::selectedItemIds() const
{
    QList<int> selectedIds;
    for (int i = 0; i < tiles.size(); ++i) {
        if (tiles.at(i)->isSelected()) {
            selectedIds.append(itemIds.at(i));
        }
    }
    return selectedIds;
}

void DlgSharedDecksPreview::openSelected()
{
    const QList<int> selectedIds = selectedItemIds();
    if (selectedIds.isEmpty()) {
        return;
    }
    resultEmitted = true;
    setDownloading(true);
    emit openRequested(selectedIds);
}

void DlgSharedDecksPreview::openAll()
{
    resultEmitted = true;
    setDownloading(true);
    emit openRequested(itemIds);
}

void DlgSharedDecksPreview::setDownloading(bool downloading)
{
    if (downloadInProgress == downloading) {
        return;
    }
    downloadInProgress = downloading;
    downloadStatusLabel->setVisible(downloading);
    for (SharedDeckPreviewWidget *tile : tiles) {
        tile->setEnabled(!downloading);
    }
    openSelectedButton->setEnabled(!downloading);
    openAllButton->setEnabled(!downloading);
}

void DlgSharedDecksPreview::setDownloadProgress(int done, int total, const QString &currentDeckName)
{
    if (!downloadInProgress) {
        return;
    }
    downloadStatusLabel->setText(tr("Downloading deck %1 of %2: %3").arg(done).arg(total).arg(currentDeckName));
}

void DlgSharedDecksPreview::updateOpenSelectedEnabled()
{
    openSelectedButton->setEnabled(!selectedItemIds().isEmpty());
}

void DlgSharedDecksPreview::onCancel()
{
    if (!resultEmitted || downloadInProgress) {
        resultEmitted = true;
        emit cancelled();
    }
}

void DlgSharedDecksPreview::closeEvent(QCloseEvent *event)
{
    onCancel();
    QDialog::closeEvent(event);
}