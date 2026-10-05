#include "dlg_turn_order.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QRandomGenerator>
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

    auto *buttonColumn = new QVBoxLayout;
    buttonColumn->addWidget(moveUpButton);
    buttonColumn->addWidget(moveDownButton);
    buttonColumn->addWidget(randomizeButton);
    buttonColumn->addStretch();

    auto *listLayout = new QHBoxLayout;
    listLayout->addWidget(listWidget);
    listLayout->addLayout(buttonColumn);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgTurnOrder::actOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgTurnOrder::reject);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(listLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    updateButtons();

    retranslateUi();
    setMinimumWidth(300);
}

void DlgTurnOrder::retranslateUi()
{
    setWindowTitle(tr("Set turn order"));
    moveUpButton->setText(tr("Move up"));
    moveDownButton->setText(tr("Move down"));
    randomizeButton->setText(tr("Randomize"));
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
    randomizeRequested = false;
    updateButtons();
}

void DlgTurnOrder::updateButtons()
{
    const int row = listWidget->currentRow();
    const int count = listWidget->count();
    moveUpButton->setEnabled(row > 0);
    moveDownButton->setEnabled(row >= 0 && row < count - 1);
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
    QList<QListWidgetItem *> items;
    while (listWidget->count() > 0) {
        items.append(listWidget->takeItem(0));
    }
    while (!items.isEmpty()) {
        const int index = QRandomGenerator::global()->bounded(items.size());
        listWidget->addItem(items.takeAt(index));
    }
    randomizeRequested = true;
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
