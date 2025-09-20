#ifndef TAB_HOME_H
#define TAB_HOME_H

#include "../interface/widgets/general/home_widget.h"
#include "../server/abstract_client.h"
#include "tab.h"

#include <QHBoxLayout>
#include <qgroupbox.h>

class AbstractClient;

class TabHome : public Tab
{
    Q_OBJECT
private:
    AbstractClient *client;
    HomeWidget *homeWidget;

public:
    TabHome(TabSupervisor *_tabSupervisor, AbstractClient *_client);
    void retranslateUi() override;
    QString getTabText() const override
    {
        return tr("Home");
    }
};

#endif // TAB_HOME_H
