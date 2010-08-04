#ifndef REMOTEDECKLIST_TREEWIDGET_H
#define REMOTEDECKLIST_TREEWIDGET_H

#include <QAbstractItemModel>
#include <QDateTime>
#include <QTreeView>

class ProtocolResponse;
class AbstractClient;
class QSortFilterProxyModel;
class DeckList_File;
class DeckList_Directory;
class DeckList_TreeItem;

class RemoteDeckList_TreeModel : public QAbstractItemModel {
	Q_OBJECT
public:
	class DirectoryNode;
	class FileNode;
	class Node {
	protected:
		DirectoryNode *parent;
		QString name;
	public:
		Node(const QString &_name, DirectoryNode *_parent = 0)
			: parent(_parent), name(_name) { }
		virtual ~Node() { };
		DirectoryNode *getParent() const { return parent; }
		QString getName() const { return name; }
	};
	class DirectoryNode : public Node, public QList<Node *> {
	public:
		DirectoryNode(const QString &_name = QString(), DirectoryNode *_parent = 0);
		~DirectoryNode();
		void clearTree();
		QString getPath() const;
		DirectoryNode *getNodeByPath(QStringList path);
		FileNode *getNodeById(int id) const;
	};
	class FileNode : public Node {
	private:
		int id;
		QDateTime uploadTime;
	public:
		FileNode(const QString &_name, int _id, const QDateTime &_uploadTime, DirectoryNode *_parent = 0)
			: Node(_name, _parent), id(_id), uploadTime(_uploadTime) { }
		int getId() const { return id; }
		QDateTime getUploadTime() const { return uploadTime; }
	};
	
	template<typename T> T getNode(const QModelIndex &index) const
	{
		if (!index.isValid())
			return dynamic_cast<T>(root);
		return dynamic_cast<T>(static_cast<Node *>(index.internalPointer()));
	}
private:
	AbstractClient *client;
	DirectoryNode *root;
	
	QIcon fileIcon, dirIcon;
	
	QModelIndex nodeToIndex(Node *node) const;
signals:
	void treeRefreshed();
private slots:
	void deckListFinished(ProtocolResponse *r);
public:
	RemoteDeckList_TreeModel(AbstractClient *_client, QObject *parent = 0);
	~RemoteDeckList_TreeModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	DirectoryNode *getRoot() const { return root; }
	void addFileToTree(DeckList_File *file, DirectoryNode *parent);
	void addFolderToTree(DeckList_Directory *folder, DirectoryNode *parent);
	void addFolderToTree(const QString &name, const QList<DeckList_TreeItem *> &folderItems, DirectoryNode *parent);
	void removeNode(Node *node);
	void refreshTree();
};

class RemoteDeckList_TreeWidget : public QTreeView {
private:
	RemoteDeckList_TreeModel *treeModel;
	QSortFilterProxyModel *proxyModel;
public:
	RemoteDeckList_TreeWidget(AbstractClient *_client, QWidget *parent = 0);
	RemoteDeckList_TreeModel::Node *getNode(const QModelIndex &ind) const;
	RemoteDeckList_TreeModel::Node *getCurrentItem() const;
	RemoteDeckList_TreeModel::DirectoryNode *getNodeByPath(const QString &path) const;
	RemoteDeckList_TreeModel::FileNode *getNodeById(int id) const;
	void addFileToTree(DeckList_File *file, RemoteDeckList_TreeModel::DirectoryNode *parent);
	void addFolderToTree(DeckList_Directory *folder, RemoteDeckList_TreeModel::DirectoryNode *parent);
	void addFolderToTree(const QString &name, RemoteDeckList_TreeModel::DirectoryNode *parent);
	void removeNode(RemoteDeckList_TreeModel::Node *node);
	void refreshTree();
};

#endif
