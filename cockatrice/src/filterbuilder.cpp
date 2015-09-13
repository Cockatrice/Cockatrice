#include "filterbuilder.h"

#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>

#include "cardfilter.h"

FilterBuilder::FilterBuilder(QWidget *parent)
    : QWidget(parent)
{
    filterCombo = new QComboBox;
    filterCombo->setObjectName("filterCombo");
    for (int i = 0; i < CardFilter::AttrEnd; i++)
        filterCombo->addItem(
            tr(CardFilter::attrName(static_cast<CardFilter::Attr>(i))),
            QVariant(i)
        );

    typeCombo = new QComboBox;
    typeCombo->setObjectName("typeCombo");
    for (int i = 0; i < CardFilter::TypeEnd; i++)
        typeCombo->addItem(
            tr(CardFilter::typeName(static_cast<CardFilter::Type>(i))),
            QVariant(i)
        );

    QPushButton *ok = new QPushButton(QPixmap("theme:icons/increment"), QString());
    ok->setObjectName("ok");
    ok->setMaximumSize(20, 20);

    edit = new QLineEdit;
    edit->setObjectName("edit");
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QGridLayout *layout = new QGridLayout;
    layout->setObjectName("layout");
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(typeCombo, 0, 0, 1, 2);
    layout->addWidget(filterCombo, 0, 2, 1, 2);
    layout->addWidget(edit, 1, 0, 1, 3);
    layout->addWidget(ok, 1, 3);

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
