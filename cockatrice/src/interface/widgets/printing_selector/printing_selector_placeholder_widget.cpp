#include "printing_selector_placeholder_widget.h"

PrintingSelectorPlaceholderWidget::PrintingSelectorPlaceholderWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    setLayout(mainLayout);

    // Image label with the background image
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet(R"(
        QLabel {
            background-image: url(theme:backgrounds/placeholder_printing_selector.svg);
            background-repeat: no-repeat;
            background-position: center;
        }
    )");
    imageLabel->setFixedSize(300, 300);

    textLabel = new QLabel(this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet(R"(
        QLabel {
            color: palette(mid);
            font-size: 14px;
            padding: 10px;
        }
    )");

    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(textLabel);

    retranslateUi();
}

void PrintingSelectorPlaceholderWidget::retranslateUi()
{
    textLabel->setText(tr("Select a card to view its available printings"));
}