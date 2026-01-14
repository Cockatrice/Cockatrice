#include "visual_deck_editor_placeholder_widget.h"

VisualDeckEditorPlaceholderWidget::VisualDeckEditorPlaceholderWidget(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    setLayout(mainLayout);

    // Image label with the background image
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet(R"(
        QLabel {
            background-image: url(theme:backgrounds/card_triplet.svg);
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

void VisualDeckEditorPlaceholderWidget::retranslateUi()
{
    textLabel->setText(tr("Add cards using the search bar or database tab to have them appear here"));
}