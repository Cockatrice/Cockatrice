#include "set_name_and_collectors_number_display_widget.h"

SetNameAndCollectorsNumberDisplayWidget::SetNameAndCollectorsNumberDisplayWidget(QWidget *parent,
                                                                                 const QString &_setName,
                                                                                 const QString &_collectorsNumber)
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

    layout->addWidget(setName);
    layout->addWidget(collectorsNumber);
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
