#include "printing_selector_placeholder_widget.h"

#include "../interface/theme_manager.h"

#include <QFont>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <qnamespace.h>

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
            padding: 10px;
        }
    )");
    applyInterfaceDefaultFont();
    connect(themeManager, &ThemeManager::fontsChanged, this,
            &PrintingSelectorPlaceholderWidget::applyInterfaceDefaultFont);

    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(textLabel);

    retranslateUi();
}

void PrintingSelectorPlaceholderWidget::retranslateUi()
{
    textLabel->setText(tr("Select a card to view its available printings"));
}

void PrintingSelectorPlaceholderWidget::applyInterfaceDefaultFont()
{
    // Family comes from the InterfaceDefault theme role; the 14px size stays caller-side.
    QFont placeholderFont = themeManager->appFont(AppFont::InterfaceDefault);
    placeholderFont.setPointSize(14);
    textLabel->setFont(placeholderFont);
}