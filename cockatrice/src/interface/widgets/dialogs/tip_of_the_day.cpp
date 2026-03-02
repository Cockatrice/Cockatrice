#include "tip_of_the_day.h"

#include <QDate>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QXmlStreamReader>
#include <utility>

#define TIPDDBMODEL_COLUMNS 3

TipOfTheDay::TipOfTheDay(QString _title, QString _content, QString _imagePath, QDate _date)
    : title(std::move(_title)), content(std::move(_content)), imagePath(std::move(_imagePath)), date(_date)
{
}

TipsOfTheDay::TipsOfTheDay(QString xmlPath, QObject *parent) : QAbstractListModel(parent)
{
    tipList = new QList<TipOfTheDay>;

    QFile xmlFile(xmlPath);

    QTextStream errorStream(stderr);
    if (!QFile::exists(xmlPath)) {
        errorStream << tr("File does not exist.\n");
        return;
    } else if (!xmlFile.open(QIODevice::ReadOnly)) {
        errorStream << tr("Failed to open file.\n");
        return;
    }

    QXmlStreamReader reader(&xmlFile);

    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::EndElement) {
            break;
        }

        auto readerName = reader.name().toString();

        if (readerName == "tip") {
            QString title, content, imagePath;
            QDate date;
            reader.readNext();
            while (!reader.atEnd()) {
                if (reader.readNext() == QXmlStreamReader::EndElement) {
                    break;
                }

                readerName = reader.name().toString();

                if (readerName == "title") {
                    title = reader.readElementText();
                } else if (readerName == "text") {
                    content = reader.readElementText();
                } else if (readerName == "image") {
                    imagePath = "theme:tips/images/" + reader.readElementText();
                } else if (readerName == "date") {
                    date = QDate::fromString(reader.readElementText(), Qt::ISODate);
                } else {
                    // unknown element, do nothing
                }
            }
            tipList->append(TipOfTheDay(title, content, imagePath, date));
        }
    }
}

TipsOfTheDay::~TipsOfTheDay()
{
    delete tipList;
}

QVariant TipsOfTheDay::data(const QModelIndex &index, int /*role*/) const
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
        case DateColumn:
            return tip.getDate();
        default:
            return QVariant();
    }
}

TipOfTheDay TipsOfTheDay::getTip(int tipId)
{
    return tipList->at(tipId);
}

int TipsOfTheDay::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return tipList->size();
}
