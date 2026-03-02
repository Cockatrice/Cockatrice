/**
 * @file card_size_widget.h
 * @ingroup CardWidgets
 * @ingroup DeckEditorWidgets
 * @ingroup DeckStorageWidgets
 * @brief TODO: Document this.
 */

#ifndef CARD_SIZE_WIDGET_H
#define CARD_SIZE_WIDGET_H

#include "../general/layout_containers/flow_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QWidget>

class CardSizeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardSizeWidget(QWidget *parent, FlowWidget *flowWidget = nullptr, int defaultValue = 100);
    [[nodiscard]] QSlider *getSlider() const;

private slots:
    void updateCardSizeSetting(int newValue);

signals:
    /**
     * Emitted when the slider value changes, but on a debounce timer.
     * Any parents that care about saving the value to settings should use this signal to indicate when to save the new
     * value to settings.
     */
    void cardSizeSettingUpdated(int newValue);

private:
    QWidget *parent;
    FlowWidget *flowWidget;
    QHBoxLayout *cardSizeLayout;
    QLabel *cardSizeLabel;
    QSlider *cardSizeSlider;
    QTimer debounceTimer; // Debounce timer
    int pendingValue;     // Stores the latest slider value
};

#endif // CARD_SIZE_WIDGET_H
