//
// Created by chudonghao on 2020/3/17.
//

#ifndef OSGEDITOR_SRC_UI_NODETREEMODEL_H
#define OSGEDITOR_SRC_UI_NODETREEMODEL_H

#include <QAbstractItemModel>
#include <osg/ref_ptr>

namespace osg { class Node; }

class NodeTreeModel : public QAbstractItemModel
{
Q_OBJECT;
public:
    NodeTreeModel(QObject *parent);

    virtual ~NodeTreeModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent) const override;

    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::DropActions supportedDragActions() const override;

    Qt::DropActions supportedDropActions() const override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    void setNode(osg::Node *node);

    osg::Node *getNode(const QModelIndex &index);

    std::vector<osg::Node *> getNodePath(const QModelIndex &index);

    QModelIndex checkNode(osg::Node *node);

private:
    struct MyItem
    {
        MyItem(MyItem *parent, osg::Node *node);

        ~MyItem();

        std::pair<MyItem *, int> find(osg::Node *node);

        osg::ref_ptr<osg::Node> node;

        MyItem *parent{};
        QList<MyItem *> children;
    };

    MyItem *m_virtualItem{};
    MyItem *m_rootItem{};
};

#endif //OSGEDITOR_SRC_UI_NODETREEMODEL_H
