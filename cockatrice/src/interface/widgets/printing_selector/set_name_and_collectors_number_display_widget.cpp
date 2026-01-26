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
    : QWidget(parent), cardSizeSlider(_cardSizeSlider)
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
    connect(cardSizeSlider, &QSlider::valueChanged, this, &SetNameAndCollectorsNumberDisplayWidget::adjustSize);

    // Add labels to the layout
    layout->addWidget(setName);
    layout->addWidget(collectorsNumber);
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
