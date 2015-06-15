#include "filterbuilder.h"

#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>

#include "cardfilter.h"

FilterBuilder::FilterBuilder(QWidget *parent)
    : QFrame(parent)
{
    int i;

    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *addFilter = new QHBoxLayout;

    filterCombo = new QComboBox;
    for (i = 0; i < CardFilter::AttrEnd; i++)
        filterCombo->addItem(
            tr(CardFilter::attrName(static_cast<CardFilter::Attr>(i))),
            QVariant(i)
        );

    typeCombo = new QComboBox;
    for (i = 0; i < CardFilter::TypeEnd; i++)
        typeCombo->addItem(
            tr(CardFilter::typeName(static_cast<CardFilter::Type>(i))),
            QVariant(i)
        );

    QPushButton *ok = new QPushButton("+");
    ok->setMaximumSize(20, 20);

    addFilter->addWidget(ok);
    addFilter->addWidget(typeCombo);
    addFilter->addWidget(filterCombo, Qt::AlignLeft);

    edit = new QLineEdit;
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addLayout(addFilter);
    layout->addWidget(edit);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    layout->setAlignment(Qt::AlignTop);
    setLayout(layout);

    connect(edit, SIGNAL(returnPressed()), this, SLOT(emit_add()));
    connect(ok, SIGNAL(released()), this, SLOT(emit_add()));
    fltr = NULL;
}

FilterBuilder::~FilterBuilder()
{
    destroyFilter();
}

void FilterBuilder::destroyFilter()
{
    if (fltr)
        delete fltr;
}

static int comboCurrentIntData(const QComboBox *combo) 
{
    return combo->itemData(combo->currentIndex()).toInt();
}

void FilterBuilder::emit_add()
{
    QString txt;

    txt = edit->text();
    if (txt.length() < 1)
        return;

    destroyFilter();
    fltr = new CardFilter(txt,
                    static_cast<CardFilter::Type>(comboCurrentIntData(typeCombo)),
                    static_cast<CardFilter::Attr>(comboCurrentIntData(filterCombo)));
    emit add(fltr);
    edit->clear();
}
