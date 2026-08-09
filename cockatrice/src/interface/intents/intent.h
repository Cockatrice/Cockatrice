#ifndef COCKATRICE_INTENT_H
#define COCKATRICE_INTENT_H

#include <QObject>

class Intent : public QObject
{
    Q_OBJECT

public:
    explicit Intent(QObject *parent = nullptr);
    ~Intent() override;

    void execute();

signals:
    void finished();
    void failed(QString reason);

protected:
    // --- Subclasses must implement these ---
    virtual bool checkPrecondition() const = 0;
    virtual void onPreconditionSatisfied() = 0;
    virtual void onPreconditionNotSatisfied() = 0;

    // Helper to chain another intent
    void runDependency(Intent *dependency);

    // Emit the outcome exactly once; ignore late signals after the intent is done.
    void emitFinished();
    void emitFailed(const QString &reason);

private:
    bool completed = false;
};

#endif // COCKATRICE_INTENT_H
