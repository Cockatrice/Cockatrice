#ifndef COCKATRICE_VISUAL_DECK_EDITOR_PLACEHOLDER_WIDGET_H
#define COCKATRICE_VISUAL_DECK_EDITOR_PLACEHOLDER_WIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class VisualDeckEditorPlaceholderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualDeckEditorPlaceholderWidget(QWidget *parent = nullptr);
    void retranslateUi();

private:
    QVBoxLayout *mainLayout;
    QLabel *imageLabel;
    QLabel *textLabel;
};

#endif // COCKATRICE_VISUAL_DECK_EDITOR_PLACEHOLDER_WIDGET_H
