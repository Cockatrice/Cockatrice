#ifndef COCKATRICE_PALETTE_GRID_WIDGET_H
#define COCKATRICE_PALETTE_GRID_WIDGET_H

#include "../theme_config.h"
#include "color_button.h"

#include <QMap>
#include <QPalette>
#include <QVBoxLayout>
#include <QWidget>

class QLabel;
class QScrollArea;
// Scrollable grid of ColorButtons — one per (ColorGroup × ColorRole) cell.
// Owns the load/read round-trip for PaletteConfig but has no file I/O itself.
class PaletteGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PaletteGridWidget(QWidget *parent = nullptr);

    void loadPalette(const PaletteConfig &cfg);
    PaletteConfig currentPaletteConfig() const;

private:
    void buildGrid(QWidget *host);
    void changeEvent(QEvent *e);
    void refreshChromePalettes();

    QMap<QPalette::ColorGroup, QMap<QPalette::ColorRole, ColorButton *>> colorButtons;
    QScrollArea *scroll;
    QWidget *gridHost;
    QVBoxLayout *layout;
    QVector<QLabel *> headerLabels;
    QVector<QWidget *> rowShadeWidgets;
};

#endif // COCKATRICE_PALETTE_GRID_WIDGET_H