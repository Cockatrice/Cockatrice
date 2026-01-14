#ifndef COCKATRICE_PRINTING_SELECTOR_PLACEHOLDER_WIDGET_H
#define COCKATRICE_PRINTING_SELECTOR_PLACEHOLDER_WIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class PrintingSelectorPlaceholderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorPlaceholderWidget(QWidget *parent = nullptr);

private:
    QVBoxLayout *mainLayout;
    QLabel *imageLabel;
    QLabel *textLabel;

    void retranslateUi();
};

#endif // COCKATRICE_PRINTING_SELECTOR_PLACEHOLDER_WIDGET_H
