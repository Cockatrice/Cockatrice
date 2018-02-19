#include <QXmlStreamReader>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include "tip_of_the_day.h"

#define TIPDDBMODEL_COLUMNS 3

TipOfTheDay::TipOfTheDay(QString _title, QString _content, QString _imagePath) {
    title = QString(_title);
    content = QString(_content);
    imagePath = QString(_imagePath);
}

TipOfTheDay::TipOfTheDay(const TipOfTheDay &other) {
    title = other.title;
    content = other.content;
    imagePath = other.imagePath;
}

TipOfTheDay::~TipOfTheDay() {
}


TipsOfTheDay::TipsOfTheDay(QString xmlPath, QObject *parent) : QAbstractListModel(parent)
{
    tipList = new QList<TipOfTheDay>;

    QFile xmlFile(xmlPath);

    QTextStream errorStream(stderr);
    if (!QFile::exists(xmlPath))
    {
        errorStream << tr("File does not exist.\n");
        return;
    }
    else if (!xmlFile.open(QIODevice::ReadOnly)) {
        errorStream << tr("Failed to open file.\n");
        return;
    }

    QXmlStreamReader reader(&xmlFile);
    
    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.error())
        {
            errorStream << tr("Error: %1 at line %2, column %3.\n").arg(
                reader.errorString(),
                QString::number(reader.lineNumber()),
                QString::number(reader.columnNumber()));
            return;
        }
        else
        {
            // process here
        }
    }
}

TipsOfTheDay::~TipsOfTheDay()
{
    delete tipList;
}

QVariant TipsOfTheDay::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= tipList->size() || index.column() >= TIPDDBMODEL_COLUMNS)
        return QVariant();

    TipOfTheDay tip = tipList->at(index.row());
    switch (index.column()) {
    case TitleColumn:
        return tip.getTitle();
    case ContentColumn:
        return tip.getContent();
    case ImagePathColumn:
        return tip.getImagePath();
    default:
        return QVariant();
    }
}

TipOfTheDay TipsOfTheDay::getTip(int tipId) {
    QString title = QString("Title: Tip #" + QString::number(tipId));
    QString content = QString(tr("This is the data part of the tip.\nI hope you found it helpful!\n\nThanks for taking the time to read it!"));
    QString imagePath = QString("d:/test_img.png");
    return TipOfTheDay(title, content, imagePath);
}

int TipsOfTheDay::rowCount(const QModelIndex & /*parent*/) const
{
    return tipList->size();
}


