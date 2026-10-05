/**
 * @file dlg_turn_order.h
 * @ingroup GameDialogs
 */
//! \todo Document this file.

#ifndef DLG_TURN_ORDER_H
#define DLG_TURN_ORDER_H

#include <QDialog>
#include <QStringList>

class QListWidget;
class QPushButton;

class DlgTurnOrder : public QDialog
{
    Q_OBJECT
private:
    QListWidget *listWidget;
    QPushButton *moveUpButton;
    QPushButton *moveDownButton;
    QPushButton *randomizeButton;
    bool randomizeRequested;

    void moveSelected(int delta);
    void updateButtons();

private slots:
    void actMoveUp();
    void actMoveDown();
    void actRandomize();
    void actOk();
    void selectionChanged();

public:
    explicit DlgTurnOrder(const QStringList &_playerNames, QWidget *parent = nullptr);

    void retranslateUi();

    [[nodiscard]] bool randomize() const
    {
        return randomizeRequested;
    }
    [[nodiscard]] QStringList order() const;
};

#endif
