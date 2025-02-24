#ifndef VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H
#define VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H

#include "../../../../game/filters/filter_tree_model.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

class VisualDatabaseDisplayColorFilterCircleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayColorFilterCircleWidget(QChar color, QWidget *parent = nullptr);
    void setColorActive(bool active);
    bool isColorActive() const;
    QChar getColorChar() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void colorToggled(QChar color, bool active);

private:
    QChar colorChar;
    bool isActive;
    int circleDiameter;
};

class VisualDatabaseDisplayColorFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDatabaseDisplayColorFilterWidget(QWidget *parent, FilterTreeModel *filterModel);
    void retranslateUi();

signals:
    void filterModeChanged(bool exactMatchMode);
    void activeColorsChanged();

private slots:
    void handleColorToggled(QChar color, bool active);
    void updateColorFilter();
    void updateFilterMode(bool checked);

private:
    FilterTreeModel *filterModel;
    QHBoxLayout *layout;
    QPushButton *toggleButton;
    QMap<QChar, bool> activeColors;
    bool exactMatchMode = false; // Default to "includes" mode
};

#endif // VISUAL_DATABASE_DISPLAY_COLOR_FILTER_WIDGET_H
