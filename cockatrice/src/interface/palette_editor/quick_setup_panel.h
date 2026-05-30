#ifndef COCKATRICE_QUICK_SETUP_PANEL_H
#define COCKATRICE_QUICK_SETUP_PANEL_H

#include "color_button.h"

#include <QWidget>

class QPushButton;
class QHBoxLayout;
class QLabel;
class QSlider;

/**
 * @class QuickSetupPanel
 * @brief Provides a compact "Quick Setup" interface for generating theme palettes.
 *
 * The panel contains:
 * - an accent color picker,
 * - an intensity slider,
 * - and a generate button.
 *
 * When the user clicks the generate button, the panel emits
 * generateRequested() with the currently selected accent color
 * and intensity value.
 *
 * Typically used together with PaletteGenerator::fromAccent()
 * to quickly generate color schemes from a chosen accent color.
 */
class QuickSetupPanel : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the quick setup panel.
     *
     * @param parent Optional parent widget.
     */
    explicit QuickSetupPanel(QWidget *parent = nullptr);

    /**
     * @brief Retranslates all user-visible strings.
     *
     * Intended to be called when the application language changes.
     */
    void retranslateUi();

    /**
     * @brief Returns the currently selected accent color.
     *
     * @return The selected accent color.
     */
    QColor accentColor() const;

    /**
     * @brief Returns the current intensity slider value.
     *
     * @return The selected intensity value.
     */
    int intensity() const;

    /**
     * @brief Updates the displayed accent color.
     *
     * Used by the parent dialog when switching schemes to keep
     * the color swatch synchronized with the active palette.
     *
     * @param c The new accent color.
     */
    void setAccentColor(const QColor &c);

signals:
    /**
     * @brief Emitted when the user requests palette generation.
     *
     * @param accent The selected accent color.
     * @param intensity The selected intensity value.
     */
    void generateRequested(QColor accent, int intensity);

private:
    QHBoxLayout *layout;
    QLabel *heading;
    QLabel *accentLabel;
    ColorButton *accentButton;
    QLabel *intensityLabel;
    QLabel *labelLow;
    QLabel *labelHigh;
    QSlider *intensitySlider;
    QLabel *intensityPercentageLabel;
    QPushButton *generateButton;
};

#endif // COCKATRICE_QUICK_SETUP_PANEL_H