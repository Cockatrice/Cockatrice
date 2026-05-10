#ifndef COCKATRICE_QUICK_SETUP_PANEL_H
#define COCKATRICE_QUICK_SETUP_PANEL_H

#include "color_button.h"

#include <QWidget>

class QPushButton;
class QHBoxLayout;
class QLabel;
class QSlider;

// The "Quick Setup" bar: accent color picker + intensity slider.
// Emits generateRequested() when the user clicks Generate.
// The dialog connects that signal and calls PaletteGenerator::fromAccent().
class QuickSetupPanel : public QWidget
{
    Q_OBJECT
public:
    explicit QuickSetupPanel(QWidget *parent = nullptr);
    void retranslateUi();

    QColor accentColor() const;
    int intensity() const;

    // Called by the dialog when switching schemes, to keep the swatch in sync
    void setAccentColor(const QColor &c);

signals:
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