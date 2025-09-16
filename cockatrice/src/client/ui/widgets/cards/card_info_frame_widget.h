#ifndef CARDFRAME_H
#define CARDFRAME_H

#include "../../../../card/exact_card.h"
#include "card_ref.h"

#include <QPushButton>
#include <QTabWidget>

class AbstractCardItem;
class CardInfoPictureWidget;
class CardInfoTextWidget;
class QVBoxLayout;
class QSplitter;

class CardInfoFrameWidget : public QTabWidget
{
    Q_OBJECT
private:
    ExactCard exactCard;
    CardInfoPictureWidget *pic;
    CardInfoTextWidget *text;
    QPushButton *viewTransformationButton;
    bool cardTextOnly;
    QWidget *tab1, *tab2, *tab3;
    QVBoxLayout *tab1Layout, *tab2Layout, *tab3Layout;
    QSplitter *splitter;

    void setViewTransformationButtonVisibility(bool visible);
    void refreshLayout();

public:
    enum ViewMode
    {
        ImageOnlyView,
        TextOnlyView,
        ImageAndTextView
    };

    explicit CardInfoFrameWidget(QWidget *parent = nullptr);
    ExactCard getCard()
    {
        return exactCard;
    }
    void retranslateUi();

public slots:
    void setCard(const ExactCard &card);
    void setCard(const QString &cardName);
    void setCard(const CardRef &cardRef);
    void setCard(AbstractCardItem *card);
    void viewTransformation();
    void clearCard();
    void setViewMode(int mode);
};

#endif
