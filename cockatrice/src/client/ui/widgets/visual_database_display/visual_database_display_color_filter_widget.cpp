#include "visual_database_display_color_filter_widget.h"

#include "../../../../game/filters/filter_card.h"
#include "../../../../game/filters/filter_tree_model.h"

#include <QJsonObject>
#include <QMouseEvent>
#include <QPainter>

VisualDatabaseDisplayColorFilterCircleWidget::VisualDatabaseDisplayColorFilterCircleWidget(QChar color, QWidget *parent)
    : QWidget(parent), colorChar(color), isActive(false), circleDiameter(30)
{
    setFixedSize(circleDiameter, circleDiameter);
}

void VisualDatabaseDisplayColorFilterCircleWidget::setColorActive(bool active)
{
    if (isActive != active) {
        isActive = active;
        update();
    }
}

bool VisualDatabaseDisplayColorFilterCircleWidget::isColorActive() const
{
    return isActive;
}

QChar VisualDatabaseDisplayColorFilterCircleWidget::getColorChar() const
{
    return colorChar;
}

void VisualDatabaseDisplayColorFilterCircleWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor circleColor;
    switch (colorChar.unicode()) {
        case 'W':
            circleColor = Qt::white;
            break;
        case 'U':
            circleColor = QColor(0, 115, 230);
            break;
        case 'B':
            circleColor = QColor(50, 50, 50);
            break;
        case 'R':
            circleColor = QColor(230, 30, 30);
            break;
        case 'G':
            circleColor = QColor(30, 180, 30);
            break;
        default:
            circleColor = Qt::transparent;
            break;
    }

    if (!isActive) {
        circleColor.setAlpha(100); // Dim inactive circles
    }

    painter.setBrush(circleColor);
    painter.setPen(Qt::black);
    painter.drawEllipse(rect());

    if (isActive) {
        QFont font = painter.font();
        font.setBold(true);
        font.setPointSize(circleDiameter / 3);
        painter.setFont(font);
        painter.setPen(colorChar.unicode() == 'B' ? Qt::white : Qt::black);
        painter.drawText(rect(), Qt::AlignCenter, colorChar);
    }
}

void VisualDatabaseDisplayColorFilterCircleWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    isActive = !isActive;
    emit colorToggled(colorChar, isActive);
    update();
}

VisualDatabaseDisplayColorFilterWidget::VisualDatabaseDisplayColorFilterWidget(QWidget *parent,
                                                                               FilterTreeModel *_filterModel)
    : QWidget(parent), filterModel(_filterModel), layout(new QHBoxLayout(this))
{
    setLayout(layout);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);

    QString fullColorIdentity = "WUBRG";
    for (const QChar &color : fullColorIdentity) {
        auto *circle = new VisualDatabaseDisplayColorFilterCircleWidget(color, this);

        layout->addWidget(circle);

        // Initialize the activeColors map
        activeColors[color] = false;

        // Connect the color toggled signal
        connect(circle, &VisualDatabaseDisplayColorFilterCircleWidget::colorToggled, this,
                &VisualDatabaseDisplayColorFilterWidget::handleColorToggled);
    }

    toggleButton = new QPushButton(this);
    toggleButton->setCheckable(true);
    layout->addWidget(toggleButton);

    // Connect the button's toggled signal
    connect(toggleButton, &QPushButton::toggled, this, &VisualDatabaseDisplayColorFilterWidget::updateFilterMode);
    connect(this, &VisualDatabaseDisplayColorFilterWidget::activeColorsChanged, this,
            &VisualDatabaseDisplayColorFilterWidget::updateColorFilter);
    connect(this, &VisualDatabaseDisplayColorFilterWidget::filterModeChanged, this,
            &VisualDatabaseDisplayColorFilterWidget::updateColorFilter);

    // Call retranslateUi to set the initial text
    retranslateUi();
}

void VisualDatabaseDisplayColorFilterWidget::retranslateUi()
{
    // Set the toggle button text based on the current mode
    toggleButton->setText(exactMatchMode ? tr("Mode: Exact Match") : tr("Mode: Includes"));
}

void VisualDatabaseDisplayColorFilterWidget::handleColorToggled(QChar color, bool active)
{
    activeColors[color] = active;
    emit activeColorsChanged();
}

void VisualDatabaseDisplayColorFilterWidget::updateColorFilter()
{
    // Clear existing filters related to color
    filterModel->clearFiltersOfType(CardFilter::Attr::AttrColor);

    if (exactMatchMode) {
        // Exact match: card must have ONLY the selected colors
        QSet<QString> selectedColors;
        for (const auto &color : activeColors.keys()) {
            if (activeColors[color]) {
                selectedColors.insert(color);
            }
        }

        if (!selectedColors.isEmpty()) {
            // Require all selected colors (TypeAnd)
            for (const auto &color : selectedColors) {
                QString colorString = color;
                filterModel->addFilter(
                    new CardFilter(colorString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor));
            }

            // Exclude any other colors (TypeAndNot)
            QStringList allPossibleColors = {"W", "U", "B", "R", "G"};
            for (const auto &color : allPossibleColors) {
                if (!selectedColors.contains(color)) {
                    QString colorString = color;
                    filterModel->addFilter(
                        new CardFilter(colorString, CardFilter::Type::TypeAndNot, CardFilter::Attr::AttrColor));
                }
            }
        }
    } else {
        // Default includes mode (TypeAnd) - match any selected colors
        for (const auto &color : activeColors.keys()) {
            if (activeColors[color]) { // If the color is active
                QString colorString = color;
                filterModel->addFilter(
                    new CardFilter(colorString, CardFilter::Type::TypeAnd, CardFilter::Attr::AttrColor));
            }
        }
    }
}

void VisualDatabaseDisplayColorFilterWidget::updateFilterMode(bool checked)
{
    exactMatchMode = checked; // Toggle between modes
    retranslateUi();          // Update the button text
    emit filterModeChanged(exactMatchMode);
}
