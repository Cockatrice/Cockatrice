#ifndef COCKATRICE_FONT_GRID_WIDGET_H
#define COCKATRICE_FONT_GRID_WIDGET_H

#include "../theme_config.h"

#include <QMap>
#include <QWidget>
#include <qtmetamacros.h>

class QFontComboBox;
class QGridLayout;
class QLabel;
class QSpinBox;

// Per-role font editors — one (family, point size) row per AppFont::Role.
// Owns the load/read round-trip for FontConfig but has no file I/O, mirroring
// PaletteGridWidget. Only family and point size are edited; weight and italic
// stay caller-side concerns and are stripped when reading back.
class FontGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FontGridWidget(QWidget *parent = nullptr);

    void loadFonts(const FontConfig &cfg);
    FontConfig currentFontConfig() const;
    void retranslateUi();

signals:
    void fontsChanged();

private:
    struct FontRow
    {
        QLabel *label = nullptr;
        QFontComboBox *familyCombo = nullptr;
        QSpinBox *sizeSpin = nullptr;
    };

    QMap<AppFont::Role, FontRow> fontRows;
};

#endif // COCKATRICE_FONT_GRID_WIDGET_H
