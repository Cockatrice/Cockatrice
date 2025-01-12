#ifndef DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
#define DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H

#include "../visual_deck_storage_widget.h"

#include <QChar>
#include <QHBoxLayout>
#include <QList>
#include <QPushButton>
#include <QWidget>

class DeckPreviewWidget;
class VisualDeckStorageWidget;

class DeckPreviewColorIdentityFilterCircleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewColorIdentityFilterCircleWidget(QChar color, QWidget *parent = nullptr);
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

class DeckPreviewColorIdentityFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeckPreviewColorIdentityFilterWidget(VisualDeckStorageWidget *parent);
    void retranslateUi();
    QList<DeckPreviewWidget *> filterWidgets(QList<DeckPreviewWidget *> &widgets);

signals:
    void filterModeChanged(bool exactMatchMode);
    void activeColorsChanged();

private slots:
    void handleColorToggled(QChar color, bool active);
    void updateFilterMode(bool checked);

private:
    QHBoxLayout *layout;
    QPushButton *toggleButton;
    QMap<QChar, bool> activeColors;
    bool exactMatchMode = false; // Default to "includes" mode
};

#endif // DECK_PREVIEW_COLOR_IDENTITY_FILTER_WIDGET_H
