#ifndef TIP_OF_DAY_H
#define TIP_OF_DAY_H

#include <QAbstractListModel>

class TipOfTheDay : public QObject
{
    Q_OBJECT
public:
    explicit TipOfTheDay(QString _title, QString _content, QString _imagePath);
    TipOfTheDay(const TipOfTheDay &other);
    ~TipOfTheDay() override = default;
    QString getTitle() const
    {
        return title;
    }
    QString getContent() const
    {
        return content;
    }
    QString getImagePath() const
    {
        return imagePath;
    }

private:
    QString title, content, imagePath;
};

class TipsOfTheDay : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Columns
    {
        TitleColumn,
        ContentColumn,
        ImagePathColumn,
    };

    explicit TipsOfTheDay(QString xmlPath, QObject *parent = nullptr);
    ~TipsOfTheDay() override;
    TipOfTheDay getTip(int tipId);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<TipOfTheDay> *tipList;
};

#endif
