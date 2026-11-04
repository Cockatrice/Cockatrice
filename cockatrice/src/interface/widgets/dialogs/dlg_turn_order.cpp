#include "dlg_turn_order.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>

DlgTurnOrder::DlgTurnOrder(const QStringList &_playerNames, QWidget *parent)
    : QDialog(parent), randomizeRequested(false)
{
    listWidget = new QListWidget(this);
    listWidget->addItems(_playerNames);
    listWidget->setCurrentRow(0);
    connect(listWidget, &QListWidget::currentRowChanged, this, &DlgTurnOrder::selectionChanged);

    moveUpButton = new QPushButton(this);
    connect(moveUpButton, &QPushButton::clicked, this, &DlgTurnOrder::actMoveUp);
    moveDownButton = new QPushButton(this);
    connect(moveDownButton, &QPushButton::clicked, this, &DlgTurnOrder::actMoveDown);
    randomizeButton = new QPushButton(this);
    connect(randomizeButton, &QPushButton::clicked, this, &DlgTurnOrder::actRandomize);

    // Shown in place of the list while the server is asked to randomize: the
    // order is chosen server side, so there is nothing to preview here.
    hintLabel = new QLabel(this);
    hintLabel->setWordWrap(true);
    hintLabel->setAlignment(Qt::AlignCenter);

    auto *buttonColumn = new QVBoxLayout;
    buttonColumn->addWidget(moveUpButton);
    buttonColumn->addWidget(moveDownButton);
    buttonColumn->addWidget(randomizeButton);
    buttonColumn->addStretch();

    auto *listLayout = new QHBoxLayout;
    listLayout->addWidget(listWidget);
    listLayout->addWidget(hintLabel, 1);
    listLayout->addLayout(buttonColumn);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgTurnOrder::actOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgTurnOrder::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(listLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    updateButtons();

    updateMode();
    setMinimumWidth(300);
}

void DlgTurnOrder::retranslateUi()
{
    setWindowTitle(tr("Set turn order"));
    moveUpButton->setText(tr("Move up"));
    moveDownButton->setText(tr("Move down"));
    randomizeButton->setText(randomizeRequested ? tr("Set order manually") : tr("Randomize"));
    hintLabel->setText(tr("The server will randomize the turn order."));
}

QStringList DlgTurnOrder::order() const
{
    QStringList playerNames;
    for (int i = 0; i < listWidget->count(); ++i) {
        playerNames.append(listWidget->item(i)->text());
    }
    return playerNames;
}

void DlgTurnOrder::moveSelected(int delta)
{
    const int row = listWidget->currentRow();
    const int newRow = row + delta;
    if (row < 0 || newRow < 0 || newRow >= listWidget->count()) {
        return;
    }

    QListWidgetItem *item = listWidget->takeItem(row);
    listWidget->insertItem(newRow, item);
    listWidget->setCurrentRow(newRow);
    updateButtons();
}

void DlgTurnOrder::updateMode()
{
    listWidget->setVisible(!randomizeRequested);
    hintLabel->setVisible(randomizeRequested);
    moveUpButton->setVisible(!randomizeRequested);
    moveDownButton->setVisible(!randomizeRequested);
    retranslateUi();
}

void DlgTurnOrder::updateButtons()
{
    const int row = listWidget->currentRow();
    const int count = listWidget->count();
    const bool manualOrder = !randomizeRequested;
    moveUpButton->setEnabled(manualOrder && row > 0);
    moveDownButton->setEnabled(manualOrder && row >= 0 && row < count - 1);
}

void DlgTurnOrder::actMoveUp()
{
    moveSelected(-1);
}

void DlgTurnOrder::actMoveDown()
{
    moveSelected(1);
}

void DlgTurnOrder::actRandomize()
{
    // Randomizing is a request to the server, not a local shuffle: showing a
    // preview would let the host re-open the dialog until the order looks
    // favourable for them.
    randomizeRequested = !randomizeRequested;
    updateMode();
    updateButtons();
}

void DlgTurnOrder::actOk()
{
    accept();
}

void DlgTurnOrder::selectionChanged()
{
    updateButtons();
}
