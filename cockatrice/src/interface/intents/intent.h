#ifndef COCKATRICE_INTENT_H
#define COCKATRICE_INTENT_H

#include <QObject>

class Intent : public QObject
{
    Q_OBJECT

public:
    explicit Intent(QObject *parent = nullptr) : QObject(parent)
    {
    }
    virtual ~Intent() = default;

    void execute()
    {
        if (checkPrecondition()) {
            onPreconditionSatisfied();
        } else {
            onPreconditionNotSatisfied();
        }
    }

signals:
    void finished();
    void failed(QString reason);

protected:
    // --- Subclasses must implement these ---
    virtual bool checkPrecondition() const = 0;
    virtual void onPreconditionSatisfied() = 0;
    virtual void onPreconditionNotSatisfied() = 0;

    // Helper to chain another intent
    void runDependency(Intent *dependency)
    {
        connect(dependency, &Intent::finished, this, [this]() {
            // Re-check after dependency finishes
            this->execute();
        });

        connect(dependency, &Intent::failed, this, &Intent::failed);

        dependency->execute();
    }
};

#endif // COCKATRICE_INTENT_H
