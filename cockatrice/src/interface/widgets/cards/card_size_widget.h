/**
 * @file card_size_widget.h
 * @ingroup CardWidgets
 * @ingroup DeckEditorWidgets
 * @ingroup DeckStorageWidgets
 */
//! \todo Document this file.

#ifndef CARD_SIZE_WIDGET_H
#define CARD_SIZE_WIDGET_H

#include "../general/layout_containers/flow_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QWidget>

class QWheelEvent;

class CardSizeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardSizeWidget(QWidget *parent, FlowWidget *flowWidget = nullptr, int defaultValue = 100);
    [[nodiscard]] QSlider *getSlider() const;

    /**
     * @brief Resizes the cards when the user Ctrl + scrolls over @p host or any of its descendants.
     *
     * Installs this widget as an event filter on the given host widget. If the host contains a
     * scroll area the filter is also installed on the scroll area's content widget, so the resize
     * intercepts the wheel event before the scroll area would scroll the view.
     */
    void enableCtrlScrollResize(QWidget *host);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

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

    bool adjustSliderForWheel(QWheelEvent *event);

    static constexpr int CARD_SIZE_WHEEL_STEP = 10; ///< Slider step applied per Ctrl + scroll notch.
};

#endif // CARD_SIZE_WIDGET_H
