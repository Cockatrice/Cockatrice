#ifndef DLG_TIPOFDAY_H
#define DLG_TIPOFDAY_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;
class TipsOfTheDay;

class DlgTipOfTheDay : public QDialog
{
    Q_OBJECT
public:
    DlgTipOfTheDay(QWidget *parent = 0);
signals:
    void newTipRequested(int tipId);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    unsigned int currentTip;
    TipsOfTheDay *tipDatabase;
    QLabel *title, *tipTextContent, *imageLabel;
    QCheckBox *showTipsOnStartupCheck;
    QPixmap *image;

private slots:
    void nextClicked();
    void previousClicked();
    void updateTip(int tipId);
};

#endif
