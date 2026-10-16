#include "visual_deck_editor_placeholder_widget.h"

#include "../interface/theme_manager.h"

#include <QFont>
#include <QLabel>
#include <QVBoxLayout>
#include <qnamespace.h>

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
            padding: 10px;
        }
    )");
    applyInterfaceDefaultFont();
    connect(themeManager, &ThemeManager::fontsChanged, this,
            &VisualDeckEditorPlaceholderWidget::applyInterfaceDefaultFont);

    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(textLabel);

    retranslateUi();
}

void VisualDeckEditorPlaceholderWidget::retranslateUi()
{
    textLabel->setText(tr("Add cards using the search bar or database tab to have them appear here"));
}

void VisualDeckEditorPlaceholderWidget::applyInterfaceDefaultFont()
{
    // Family comes from the InterfaceDefault theme role; the 14px size stays caller-side.
    QFont placeholderFont = themeManager->appFont(AppFont::InterfaceDefault);
    placeholderFont.setPointSize(14);
    textLabel->setFont(placeholderFont);
}