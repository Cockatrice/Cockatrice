#ifndef TAB_H
#define TAB_H

#include "card_ref.h"

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
    void showCardInfoPopup(const QPoint &pos, const CardRef &cardRef);
    void deleteCardInfoPopup(const QString &cardName);

private:
    CardRef currentCard;
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
     * Nicely asks to close the tab.
     * Override this method to do checks or ask for confirmation before closing the tab.
     * If you need to force close the tab, just call close() instead.
     *
     * @return True if the tab is successfully closed.
     */
    virtual bool closeRequest();

    virtual void tabActivated()
    {
    }
};

#endif
