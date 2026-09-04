#include "share_bar_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

ShareBarWidget::ShareBarWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);

    hintLabel = new QLabel(this);
    hintLabel->setWordWrap(true);

    nameEdit = new QLineEdit(this);
    nameEdit->setMaximumWidth(260);

    countLabel = new QLabel(this);

    cancelButton = new QPushButton(this);
    connect(cancelButton, &QPushButton::clicked, this, &ShareBarWidget::cancelRequested);

    createButton = new QPushButton(this);
    createButton->setDefault(true);
    connect(createButton, &QPushButton::clicked, this, &ShareBarWidget::createRequested);

    layout->addWidget(hintLabel, 1);
    layout->addWidget(nameEdit);
    layout->addWidget(countLabel);
    layout->addStretch();
    layout->addWidget(cancelButton);
    layout->addWidget(createButton);

    setLayout(layout);

    retranslateUi();
}

void ShareBarWidget::retranslateUi()
{
    nameEdit->setPlaceholderText(tr("Share name"));
    cancelButton->setText(tr("Cancel"));
    createButton->setText(tr("Create share link"));
    hintLabel->setText(tr("Click deck tiles to select the decks you want to share."));
}

QString ShareBarWidget::name() const
{
    return nameEdit->text().trimmed();
}

void ShareBarWidget::setName(const QString &value)
{
    nameEdit->setText(value);
}

void ShareBarWidget::setCountText(const QString &text)
{
    countLabel->setText(text);
}

void ShareBarWidget::setHintText(const QString &text, bool visible)
{
    hintLabel->setText(text);
    hintLabel->setVisible(visible);
}

void ShareBarWidget::focusName()
{
    nameEdit->setFocus();
}