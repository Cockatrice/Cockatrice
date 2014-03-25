#include <QObject>
#include <QEvent>

class KeySignals : public QObject {
    Q_OBJECT

private:
    bool filterDeleteOn;
    bool filterLROn;

signals:
    void onEnter();
    void onCtrlEnter();
    void onLeft();
    void onCtrlLeft();
    void onRight();
    void onCtrlRight();
    void onDelete();

protected:
    virtual bool eventFilter(QObject *, QEvent *event);

public:
    KeySignals()
        : filterDeleteOn(true)
        , filterLROn(true)
        {}
    void filterDelete(bool on) { filterDeleteOn = on; }
    void filterLeftRight(bool on) { filterLROn = on; }
};
