#ifndef SET_NAME_AND_COLLECTORS_NUMBER_DISPLAY_WIDGET_H
#define SET_NAME_AND_COLLECTORS_NUMBER_DISPLAY_WIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class SetNameAndCollectorsNumberDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    SetNameAndCollectorsNumberDisplayWidget(QWidget *parent, const QString &setName, const QString &collectorsNumber);
    void resizeEvent(QResizeEvent *event) override;

private:
    QVBoxLayout *layout;
    QLabel *setName;
    QLabel *collectorsNumber;
};

#endif // SET_NAME_AND_COLLECTORS_NUMBER_DISPLAY_WIDGET_H
