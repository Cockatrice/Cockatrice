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
    explicit DlgTipOfTheDay(QWidget *parent = nullptr);
    ~DlgTipOfTheDay();
    bool successfulInit;
signals:
    void newTipRequested(int tipId);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    unsigned int currentTip;
    TipsOfTheDay *tipDatabase;
    QLabel *title, *tipTextContent, *imageLabel, *tipNumber, *date;
    QCheckBox *showTipsOnStartupCheck;
    QPixmap *image;

private slots:
    void nextClicked();
    void previousClicked();
    void updateTip(int tipId);
};

#endif
