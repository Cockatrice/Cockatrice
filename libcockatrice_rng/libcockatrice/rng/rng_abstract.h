#ifndef RNG_ABSTRACT_H
#define RNG_ABSTRACT_H

#include <QObject>
#include <QVector>

class RNG_Abstract : public QObject
{
    Q_OBJECT
public:
    explicit RNG_Abstract(QObject *parent = nullptr) : QObject(parent)
    {
    }
    virtual unsigned int rand(int min, int max) = 0;
    QVector<int> makeNumbersVector(int n, int min, int max);
    [[nodiscard]] double testRandom(const QVector<int> &numbers) const;
};

extern RNG_Abstract *rng;

#endif
