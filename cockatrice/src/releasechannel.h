#ifndef RELEASECHANNEL_H
#define RELEASECHANNEL_H

#include <QDate>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <utility>

class QNetworkReply;
class QNetworkAccessManager;

class Release
{
    friend class StableReleaseChannel;
    friend class BetaReleaseChannel;

public:
    Release() = default;
    ~Release() = default;

private:
    QString name, descriptionUrl, downloadUrl, commitHash;
    QDate publishDate;
    bool compatibleVersionFound = false;

protected:
    void setName(QString _name)
    {
        name = std::move(_name);
    }
    void setDescriptionUrl(QString _descriptionUrl)
    {
        descriptionUrl = std::move(_descriptionUrl);
    }
    void setDownloadUrl(QString _downloadUrl)
    {
        downloadUrl = std::move(_downloadUrl);
        compatibleVersionFound = true;
    }
    void setCommitHash(QString _commitHash)
    {
        commitHash = std::move(_commitHash);
    }
    void setPublishDate(QDate _publishDate)
    {
        publishDate = _publishDate;
    }

public:
    QString getName() const
    {
        return name;
    }
    QString getDescriptionUrl() const
    {
        return descriptionUrl;
    }
    QString getDownloadUrl() const
    {
        return downloadUrl;
    }
    QString getCommitHash() const
    {
        return commitHash;
    }
    QDate getPublishDate() const
    {
        return publishDate;
    }
    bool isCompatibleVersionFound() const
    {
        return compatibleVersionFound;
    }
};

class ReleaseChannel : public QObject
{
    Q_OBJECT
public:
    ReleaseChannel();
    ~ReleaseChannel() override;

protected:
    // shared by all instances
    static int sharedIndex;
    int index;
    QNetworkAccessManager *netMan;
    QNetworkReply *response;
    Release *lastRelease;

protected:
    static bool downloadMatchesCurrentOS(const QString &fileName);
    virtual QString getReleaseChannelUrl() const = 0;

public:
    int getIndex() const
    {
        return index;
    }
    Release *getLastRelease()
    {
        return lastRelease;
    }
    virtual QString getManualDownloadUrl() const = 0;
    virtual QString getName() const = 0;
    void checkForUpdates();
signals:
    void finishedCheck(bool needToUpdate, bool isCompatible, Release *release);
    void error(QString errorString);
protected slots:
    virtual void releaseListFinished() = 0;
    virtual void fileListFinished() = 0;
};

class StableReleaseChannel : public ReleaseChannel
{
    Q_OBJECT
public:
    StableReleaseChannel() = default;
    ~StableReleaseChannel() override = default;

    QString getManualDownloadUrl() const override;

    QString getName() const override;

protected:
    QString getReleaseChannelUrl() const override;
protected slots:

    void releaseListFinished() override;
    void tagListFinished();

    void fileListFinished() override;
};

class BetaReleaseChannel : public ReleaseChannel
{
    Q_OBJECT
public:
    BetaReleaseChannel() = default;
    ~BetaReleaseChannel() override = default;

    QString getManualDownloadUrl() const override;

    QString getName() const override;

protected:
    QString getReleaseChannelUrl() const override;
protected slots:

    void releaseListFinished() override;

    void fileListFinished() override;
};

#endif