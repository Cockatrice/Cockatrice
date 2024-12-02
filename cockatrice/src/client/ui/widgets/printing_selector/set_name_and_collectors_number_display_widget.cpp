#include "set_name_and_collectors_number_display_widget.h"

#include <QSlider>

SetNameAndCollectorsNumberDisplayWidget::SetNameAndCollectorsNumberDisplayWidget(QWidget *parent,
                                                                                 const QString &_setName,
                                                                                 const QString &_collectorsNumber,
                                                                                 QSlider *_cardSizeSlider)
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    setMinimumSize(QWidget::sizeHint());

    setName = new QLabel(_setName);
    setName->setWordWrap(true);
    setName->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    setName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    collectorsNumber = new QLabel(_collectorsNumber);
    collectorsNumber->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    collectorsNumber->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    cardSizeSlider = _cardSizeSlider;

    connect(cardSizeSlider, &QSlider::valueChanged, this, &SetNameAndCollectorsNumberDisplayWidget::adjustFontSize);

    layout->addWidget(setName);
    layout->addWidget(collectorsNumber);
}


void SetNameAndCollectorsNumberDisplayWidget::adjustFontSize(int scalePercentage)
{
    // Define the base font size and the range
    const int minFontSize = 8;  // Minimum font size
    const int maxFontSize = 32; // Maximum font size
    const int basePercentage = 100; // Scale at 100%

    // Calculate the new font size
    int newFontSize = minFontSize + (scalePercentage - basePercentage) * (maxFontSize - minFontSize) / (250 - 25);

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
