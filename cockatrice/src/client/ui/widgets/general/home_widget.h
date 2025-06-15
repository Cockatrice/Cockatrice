#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H
#include "../../../tabs/tab_supervisor.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QWidget>

class HomeWidget : public QWidget
{

    Q_OBJECT

public:
    HomeWidget(QWidget *parent, TabSupervisor *tabSupervisor);
    QGroupBox *createSettingsButtonGroup(const QString &title);
    QGroupBox *createUpdatesButtonGroup(const QString &title);
    QGroupBox *createNavigationButtonGroup(const QString &title);
    QGroupBox *createPlayButtonGroup(const QString &title);

public slots:
    void paintEvent(QPaintEvent *event) override;

private:
    QGridLayout *layout;
    TabSupervisor *tabSupervisor;
    QPixmap background;
    QPixmap overlay;
};

#endif // HOME_WIDGET_H
