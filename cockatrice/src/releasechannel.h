#ifndef RELEASECHANNEL_H
#define RELEASECHANNEL_H

#include <QString>
#include <QDate>
#include <QObject>
#include <QVariantMap>

class QNetworkReply;
class QNetworkAccessManager;

class Release {
    friend class StableReleaseChannel;
    friend class DevReleaseChannel;
public:
    Release() {};
    ~Release() {};
private:
    QString name, descriptionUrl, downloadUrl, commitHash;
    QDate publishDate;
    bool compatibleVersionFound = false;
protected:
    void setName(QString _name) { name = _name; }
    void setDescriptionUrl(QString _descriptionUrl) { descriptionUrl = _descriptionUrl; }
    void setDownloadUrl(QString _downloadUrl) { downloadUrl = _downloadUrl; compatibleVersionFound = true; }
    void setCommitHash(QString _commitHash) { commitHash = _commitHash; }
    void setPublishDate(QDate _publishDate) { publishDate = _publishDate; }
public:
    QString getName() const { return name; }
    QString getDescriptionUrl() const { return descriptionUrl; }
    QString getDownloadUrl() const { return downloadUrl; }
    QString getCommitHash() const { return commitHash; }
    QDate getPublishDate() const { return publishDate; }
    bool isCompatibleVersionFound() const { return compatibleVersionFound; }
};

class ReleaseChannel: public QObject {
    Q_OBJECT
public:
    ReleaseChannel();
    ~ReleaseChannel();
protected:
    // shared by all instances
    static int sharedIndex;
    int index;
    QNetworkAccessManager *netMan;
    QNetworkReply *response;
    Release * lastRelease;
protected:
    static bool downloadMatchesCurrentOS(const QString &fileName);
    virtual QString getReleaseChannelUrl() const = 0;
public:
    int getIndex() const { return index; }
    Release * getLastRelease() { return lastRelease; }
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

class StableReleaseChannel: public ReleaseChannel {
    Q_OBJECT
public:
    StableReleaseChannel() {};
    ~StableReleaseChannel() {};
    virtual QString getManualDownloadUrl() const;
    virtual QString getName() const;
protected:
    virtual QString getReleaseChannelUrl() const;
protected slots:
    virtual void releaseListFinished();
    void tagListFinished();
    virtual void fileListFinished();
};

class DevReleaseChannel: public ReleaseChannel {
    Q_OBJECT
public:
    DevReleaseChannel() {};
    ~DevReleaseChannel() {};
    virtual QString getManualDownloadUrl() const;
    virtual QString getName() const;
protected:
    virtual QString getReleaseChannelUrl() const;
protected slots:
    virtual void releaseListFinished();
    virtual void fileListFinished();
};

#endif