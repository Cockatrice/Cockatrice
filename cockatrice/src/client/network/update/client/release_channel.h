/**
 * @file release_channel.h
 * @ingroup ClientUpdate
 * @brief TODO: Document this.
 */

#ifndef RELEASECHANNEL_H
#define RELEASECHANNEL_H

#include <QDate>
#include <QLoggingCategory>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <utility>

inline Q_LOGGING_CATEGORY(ReleaseChannelLog, "release_channel");

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
    [[nodiscard]] QString getName() const
    {
        return name;
    }
    [[nodiscard]] QString getDescriptionUrl() const
    {
        return descriptionUrl;
    }
    [[nodiscard]] QString getDownloadUrl() const
    {
        return downloadUrl;
    }
    [[nodiscard]] QString getCommitHash() const
    {
        return commitHash;
    }
    [[nodiscard]] QDate getPublishDate() const
    {
        return publishDate;
    }
    [[nodiscard]] bool isCompatibleVersionFound() const
    {
        return compatibleVersionFound;
    }
};

class ReleaseChannel : public QObject
{
    Q_OBJECT
public:
    explicit ReleaseChannel();
    ~ReleaseChannel() override;

protected:
    QNetworkAccessManager *netMan;
    QNetworkReply *response;
    Release *lastRelease;

protected:
    static bool downloadMatchesCurrentOS(const QString &fileName);
    [[nodiscard]] virtual QString getReleaseChannelUrl() const = 0;

public:
    Release *getLastRelease()
    {
        return lastRelease;
    }
    [[nodiscard]] virtual QString getManualDownloadUrl() const = 0;
    [[nodiscard]] virtual QString getName() const = 0;
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
    explicit StableReleaseChannel() = default;
    ~StableReleaseChannel() override = default;

    [[nodiscard]] QString getManualDownloadUrl() const override;

    [[nodiscard]] QString getName() const override;

protected:
    [[nodiscard]] QString getReleaseChannelUrl() const override;
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

    [[nodiscard]] QString getManualDownloadUrl() const override;

    [[nodiscard]] QString getName() const override;

protected:
    [[nodiscard]] QString getReleaseChannelUrl() const override;
protected slots:

    void releaseListFinished() override;

    void fileListFinished() override;
};

#endif