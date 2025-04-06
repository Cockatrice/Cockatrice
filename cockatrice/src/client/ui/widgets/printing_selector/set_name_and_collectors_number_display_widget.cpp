#include "set_name_and_collectors_number_display_widget.h"

#include <QSlider>

/**
 * @class SetNameAndCollectorsNumberDisplayWidget
 * @brief A widget to display the set name and collectors number with adjustable font size.
 *
 * This widget displays the set name and collectors number on two separate labels. The font size is resized dynamically
 * when the card size is changed.
 */
SetNameAndCollectorsNumberDisplayWidget::SetNameAndCollectorsNumberDisplayWidget(QWidget *parent,
                                                                                 const QString &_setName,
                                                                                 const QString &_collectorsNumber,
                                                                                 QSlider *_cardSizeSlider)
    : QWidget(parent)
{
    // Set up the layout for the widget
    layout = new QVBoxLayout(this);
    setLayout(layout);

    // Set the widget's size policy and minimum size
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    setMinimumSize(QWidget::sizeHint());

    // Create and configure the set name label
    setName = new QLabel(_setName);
    setName->setWordWrap(true);
    setName->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    setName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Create and configure the collectors number label
    collectorsNumber = new QLabel(_collectorsNumber);
    collectorsNumber->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    collectorsNumber->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Store the card size slider and connect its signal to the font size adjustment slot
    cardSizeSlider = _cardSizeSlider;
    connect(cardSizeSlider, &QSlider::valueChanged, this, &SetNameAndCollectorsNumberDisplayWidget::adjustFontSize);

    // Add labels to the layout
    layout->addWidget(setName);
    layout->addWidget(collectorsNumber);
}

/**
 * @brief Adjusts the font size of the labels based on the slider value.
 *
 * This method adjusts the font size of the set name and collectors number labels
 * according to the scale percentage provided by the slider. The font size is clamped
 * to a range between the defined minimum and maximum font sizes.
 *
 * @param scalePercentage The scale percentage from the slider.
 */
void SetNameAndCollectorsNumberDisplayWidget::adjustFontSize(int scalePercentage)
{
    // Define the base font size and the range
    const int minFontSize = 8;      // Minimum font size
    const int maxFontSize = 32;     // Maximum font size
    const int basePercentage = 100; // Scale at 100%

    // Calculate the new font size
    int newFontSize = minFontSize + (scalePercentage - basePercentage) * (maxFontSize - minFontSize) / 225;

    // Clamp the font size to the defined range
    newFontSize = std::clamp(newFontSize, minFontSize, maxFontSize);

    // Update the fonts for both labels
    QFont setNameFont = setName->font();
    setNameFont.setPointSize(newFontSize);
    setName->setFont(setNameFont);

    QFont collectorsNumberFont = collectorsNumber->font();
    collectorsNumberFont.setPointSize(newFontSize);
    collectorsNumber->setFont(collectorsNumberFont);

    // Optionally trigger a resize to accommodate new font size
    adjustSize();
}

/**
 * @brief Handles resize events to adjust the height of the set name label.
 *
 * This method calculates the height required to display the set name label with word wrapping.
 * It adjusts the minimum height of the set name label to fit the text.
 *
 * @param event The resize event.
 */
void SetNameAndCollectorsNumberDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event); // Ensure the parent class handles the event first

    QFontMetrics fm(setName->font());
    int labelWidth = setName->width(); // Get the current width of the QLabel
    QString text = setName->text();    // The text to be rendered

    // Calculate the height required to render the text with word wrapping
    int textHeight = fm.boundingRect(0, 0, labelWidth, 0, Qt::TextWordWrap, text).height();

    // Set the minimum height to accommodate the required text height
    setName->setMinimumHeight(textHeight);
}
