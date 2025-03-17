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
    explicit Tab(TabSupervisor *_tabSupervisor);
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
    /**
     * Sends a request to close the tab.
     * Signals for cleanup should be emitted from this method instead of the destructor.
     *
     * @param forced whether this close request was initiated by the user or forced by the server.
     */
    virtual void closeRequest(bool forced = false);
    virtual void tabActivated()
    {
    }
};

#endif
