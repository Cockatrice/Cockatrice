#ifndef TAB_H
#define TAB_H

#include <QMainWindow>

class QMenu;
class TabSupervisor;
class CardInfoDisplayWidget;

class Tab : public QMainWindow
{
    Q_OBJECT
signals:
    void userEvent(bool globalEvent = true);
    void tabTextChanged(Tab *tab, const QString &newTabText);

protected:
    TabSupervisor *tabSupervisor;
    void addTabMenu(QMenu *menu)
    {
        tabMenus.append(menu);
    }
protected slots:
    void showCardInfoPopup(const QPoint &pos, const QString &cardName, const QString &providerId);
    void deleteCardInfoPopup(const QString &cardName);

private:
    QString currentCardName, currentProviderId;
    bool contentsChanged;
    CardInfoDisplayWidget *infoPopup;
    QList<QMenu *> tabMenus;

public:
    Tab(TabSupervisor *_tabSupervisor, QWidget *parent = nullptr);
    const QList<QMenu *> &getTabMenus() const
    {
        return tabMenus;
    }
    TabSupervisor *getTabSupervisor() const
    {
        return tabSupervisor;
    }
    bool getContentsChanged() const
    {
        return contentsChanged;
    }
    void setContentsChanged(bool _contentsChanged)
    {
        contentsChanged = _contentsChanged;
    }
    virtual QString getTabText() const = 0;
    virtual void retranslateUi() = 0;
    virtual void closeRequest()
    {
    }
    virtual void tabActivated()
    {
    }
};

#endif
