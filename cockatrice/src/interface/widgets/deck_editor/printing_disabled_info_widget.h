#ifndef COCKATRICE_PRINTING_DISABLED_INFO_WIDGET_H
#define COCKATRICE_PRINTING_DISABLED_INFO_WIDGET_H
#include <QWidget>

class QPushButton;
class QLabel;

class PrintingDisabledInfoWidget : public QWidget
{
    Q_OBJECT

    QLabel *textLabel;
    QPushButton *settingsButton;

private slots:
    void disableOverridePrintings();

public:
    explicit PrintingDisabledInfoWidget(QWidget *parent);

    void retranslateUi();
};

#endif // COCKATRICE_PRINTING_DISABLED_INFO_WIDGET_H
