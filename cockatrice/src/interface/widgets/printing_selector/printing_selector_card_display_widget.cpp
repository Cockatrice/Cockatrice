#include "printing_selector_card_display_widget.h"

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/printing/printing_info.h"
#include "libcockatrice/card/set/card_set.h"
#include "printing_selector.h"
#include "printing_selector_card_overlay_widget.h"
#include "printing_selector_host.h"
#include "set_name_and_collectors_number_display_widget.h"

#include <QMap>
#include <QSharedPointer>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <qnamespace.h>

class QSlider;

/**
 * @brief Constructs a PrintingSelectorCardDisplayWidget to display card information.
 *
 * This widget is responsible for displaying the selected card's printing information, including
 * the card's image and set details. It also handles the layout of the card's display, including
 * its size, set name, and collectors number. The card is displayed within a `QVBoxLayout` with
 * two main components: the overlay (which combines the card image and buttons) and the set name and collectors number
 * display.
 *
 * @param parent The parent widget for this display.
 * @param host The host supplying deck state and routing card activations.
 * @param cardSizeSlider The slider controlling the size of the displayed card.
 * @param rootCard The root card object, representing the card to be displayed.
 */
PrintingSelectorCardDisplayWidget::PrintingSelectorCardDisplayWidget(QWidget *parent,
                                                                     PrintingSelectorHost *host,
                                                                     QSlider *cardSizeSlider,
                                                                     const ExactCard &rootCard)
    : QWidget(parent), rootCard(rootCard), selectionOnly(host->isSelectionOnly())
{
    if (selectionOnly) {
        // A plain QWidget only paints a stylesheet border with a styled background.
        setAttribute(Qt::WA_StyledBackground, true);
    }
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(3);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Create the overlay widget for the card display
    overlayWidget = new PrintingSelectorCardOverlayWidget(this, host, cardSizeSlider, rootCard);
    connect(overlayWidget, &PrintingSelectorCardOverlayWidget::cardPreferenceChanged, this,
            [this]() { emit cardPreferenceChanged(); });
    connect(overlayWidget, &PrintingSelectorCardOverlayWidget::printingSelected, this,
            [this](const ExactCard &card) { emit printingSelected(card); });

    CardSetPtr set = rootCard.getPrinting().getSet();

    // Create the widget to display the set name and collector's number
    QString combinedSetName = QString(set->getLongName() + " (" + set->getShortName() + ")");
    setNameAndCollectorsNumberDisplayWidget =
        new SetNameAndCollectorsNumberDisplayWidget(this, combinedSetName, rootCard.getPrinting().getProperty("num"));

    // Add the widgets to the layout
    layout->addWidget(overlayWidget, 0, Qt::AlignHCenter);
    layout->addWidget(setNameAndCollectorsNumberDisplayWidget, 1, Qt::AlignHCenter | Qt::AlignBottom);
}

/**
 * @brief Adjusts the width of the set name display to fit the card overlay widget.
 *
 * This method ensures that the set name and collector's number display widget does not exceed
 * the width of the card's overlay widget. It clamps the set name widget to match the width of
 * the overlay widget and updates the display.
 */
void PrintingSelectorCardDisplayWidget::clampSetNameToPicture()
{
    if (overlayWidget != nullptr && setNameAndCollectorsNumberDisplayWidget != nullptr) {
        setNameAndCollectorsNumberDisplayWidget->setMaximumWidth(overlayWidget->width());
    }
    update();
}

void PrintingSelectorCardDisplayWidget::updateCardAmounts(const QMap<QString, ZoneCounts> &uuidToAmounts)
{
    auto counts = uuidToAmounts.value(rootCard.getPrinting().getUuid());
    overlayWidget->updateCardAmounts(counts.mainboard, counts.sideboard, counts.tokensboard);
}

void PrintingSelectorCardDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    clampSetNameToPicture();
}

void PrintingSelectorCardDisplayWidget::setSelected(bool selected)
{
    if (!selectionOnly) {
        return;
    }
    // Keep an invisible border in both states so selection does not reflow the grid.
    const QString border = selected ? QStringLiteral("palette(highlight)") : QStringLiteral("transparent");
    setStyleSheet(
        QStringLiteral("PrintingSelectorCardDisplayWidget { border: 2px solid %1; border-radius: 4px; }").arg(border));
}

QString PrintingSelectorCardDisplayWidget::getProviderId() const
{
    return rootCard.getPrinting().getUuid();
}
