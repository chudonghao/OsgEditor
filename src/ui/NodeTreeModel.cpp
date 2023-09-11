//
// Created by chudonghao on 2020/3/17.
//

#include "NodeTreeModel.h"

#include <QDebug>
#include <QMimeData>
#include <QStack>
#include <cassert>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Node>

using namespace osg;

NodeTreeModel::MyItem::MyItem(MyItem *parent, osg::Node *node) {
  this->parent = parent;
  this->node = node;
  if (parent) {
    parent->children.push_back(this);
  }
}

NodeTreeModel::MyItem::~MyItem() { qDeleteAll(children); }

std::pair<NodeTreeModel::MyItem *, int> NodeTreeModel::MyItem::find(osg::Node *node) {
  if (node == this->node) {
    return {this, -1};
  } else {
    int count = 0;
    for (const auto &child : children) {
      MyItem *item;
      int index;
      std::tie(item, index) = child->find(node);
      if (item) {
        return {item, index != -1 ? index : count};
      }
      ++count;
    }
  }
  return {nullptr, -1};
}

NodeTreeModel::NodeTreeModel(QObject *parent) : QAbstractItemModel(parent) { _virtual_item = new MyItem(nullptr, nullptr); }

NodeTreeModel::~NodeTreeModel() { delete _virtual_item; }

QModelIndex NodeTreeModel::index(int row, int column, const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  MyItem *parent_item = nullptr;
  if (!parent.isValid()) {
    parent_item = _virtual_item;
  } else {
    parent_item = static_cast<MyItem *>(parent.internalPointer());
  }

  MyItem *childItem = row < parent_item->children.count() ? parent_item->children.at(row) : nullptr;
  if (childItem) {
    return createIndex(row, column, childItem);
  } else {
    return QModelIndex();
  }
}

QModelIndex NodeTreeModel::parent(const QModelIndex &child) const {
  if (!child.isValid()) {
    return QModelIndex();
  }
  MyItem *childItem = static_cast<MyItem *>(child.internalPointer());
  if (childItem == _root_item) {
    return QModelIndex();
  }

  MyItem *parent_item = childItem->parent;
  if (parent_item == _root_item) {
    return createIndex(0, 0, _root_item);
  }

  MyItem *grandparentItem = parent_item->parent;
  auto grandparentGroup = grandparentItem->node->asGroup();
  auto index = grandparentGroup->getChildIndex(parent_item->node);
  if (index >= 0) {
    return createIndex(index, 0, parent_item);
  }

  return QModelIndex();
}

Qt::ItemFlags NodeTreeModel::flags(const QModelIndex &index) const {
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  if (!index.isValid()) {
    return flags;
  }

  MyItem *item = static_cast<MyItem *>(index.internalPointer());

  if (item != _root_item) {
    flags |= Qt::ItemFlag::ItemIsDragEnabled;
  }

  if (item->node->asGroup()) {
    flags |= Qt::ItemFlag::ItemIsDropEnabled;
  }

  return flags;
}

int NodeTreeModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    MyItem *item = static_cast<MyItem *>(parent.internalPointer());
    return item->children.count();
  }
  return _virtual_item->children.count();
}

int NodeTreeModel::columnCount(const QModelIndex &parent) const { return 4; }

QVariant NodeTreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || role != Qt::DisplayRole) {
    return QVariant();
  }
  MyItem *item = static_cast<MyItem *>(index.internalPointer());
  auto node = item->node;
  if (node) {
    switch (index.column()) {
      case 0:
        return QVariant(QString(node->getName().c_str()));
      case 1: {
        return QString(node->className());
      }
      case 2:
        return QVariant(node->getStateSet() != nullptr);
      case 3: {
        auto geo = node->asGeometry();
        if (geo) {
          return geo->getNumPrimitiveSets();
        }
        return QVariant(0);
      }
    }
  }
  return QVariant();
}

void NodeTreeModel::SetNode(osg::Node *node) {
  beginResetModel();
  delete _virtual_item;
  _root_item = nullptr;
  _virtual_item = new MyItem(nullptr, nullptr);
  if (!node) {
    goto err;
  }
  {
    struct Arg {
      MyItem *item;
      int cid;
      int depth;
    };
    QStack<Arg> args;
    _root_item = new MyItem(_virtual_item, node);
    auto group = node->asGroup();
    if (group && group->getNumChildren()) {
      args.push(Arg{_root_item, 0, 0});
    }

    for (; !args.empty();) {
      // every child
      auto &arg = args.top();
      auto *item = arg.item;
      auto cid = arg.cid;
      auto *g = item->node->asGroup();
      auto *node = g->getChild(cid);
      // 1. create MyItem for it
      // 2. add MyItem to its parent
      auto child = new MyItem(item, node);
      // complete once
      args.pop();
      // save next iter
      ++cid;
      if (cid < g->getNumChildren()) {
        args.push(Arg{item, cid, arg.depth});
      }
      // sub
      g = node->asGroup();
      if (g && g->getNumChildren()) {
        args.push(Arg{child, 0, arg.depth + 1});
      }
    }
  }
err:
  endResetModel();
}

osg::Node *NodeTreeModel::GetNode(const QModelIndex &index) {
  auto index0 = index.parent().child(index.row(), 0);
  MyItem *item = static_cast<MyItem *>(index0.internalPointer());
  if (item == nullptr) {
    item = _root_item;
  }
  return item ? item->node : nullptr;
}

std::vector<osg::Node *> NodeTreeModel::GetNodePath(const QModelIndex &index) {
  std::vector<osg::Node *> res;
  QModelIndex i = index;
  for (; i.isValid(); i = i.parent()) {
    MyItem *item = static_cast<MyItem *>(i.internalPointer());
    res.push_back(item->node);
  }
  std::reverse(res.begin(), res.end());
  return res;
}

QVariant NodeTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    switch (section) {
      case 0:
        return QString("Name");
      case 1:
        return QString("Type");
      case 2:
        return QString("Has StateSet");
      case 3:
        return QString("Primitive Set Num");
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex NodeTreeModel::CheckNode(osg::Node *node) {
  QModelIndex index;
  if (!node) {
    return index;
  }
  MyItem *item;
  int row;
  std::tie(item, row) = _virtual_item->find(node);
  if (item) {
    index = createIndex(row, 0, item);
  }
  return index;
}

// bool NodeTreeModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//     qDebug() << 2 << row;
//     auto child = parent.child(row, count);
//     MyItem *parent_item = static_cast<MyItem *>(parent.internalPointer());
//     MyItem *childItem = static_cast<MyItem *>(child.internalPointer());
//     ref_ptr<Node> parentNode = getNode(parent);
//     ref_ptr<Node> childNode = getNode(child);
//     if (!parentNode || !childNode)
//     {
//         return false;
//     }
//     bool removed = false;
//     beginRemoveRows(parent, row, row);
//     if (auto childDrawable = childNode->asDrawable())
//     {
//         auto parentGeode = parentNode->asGeode();
//         // TODO
//         return false;
//     }
//     else
//     {
//         auto parentGroup = parentNode->asGroup();
//
//         removed = parentGroup->removeChild(row, 1);
//         if (removed)
//         {
//             auto item = parent_item->children.takeAt(row);
//             delete item;
//         }
//     }
//     endRemoveRows();
//     qDebug() << "remove: " << removed;
//     return removed;
// }

Qt::DropActions NodeTreeModel::supportedDragActions() const { return Qt::MoveAction; }

Qt::DropActions NodeTreeModel::supportedDropActions() const { return Qt::MoveAction; }

// bool NodeTreeModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//     qDebug() << 1 << row;
//     return QAbstractItemModel::insertRows(row, count, parent);
// }

QMimeData *NodeTreeModel::mimeData(const QModelIndexList &indexes) const {
  qDebug() << 0;
  QMimeData *data = new QMimeData;
  data->setProperty("index", indexes[0]);
  return data;
}

bool NodeTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
  qDebug() << 1 << parent.row();
  auto childIndex = data->property("index").toModelIndex();
  auto oldParentIndex = childIndex.parent();
  auto &newParentIndex = parent;
  if (!childIndex.isValid() || !oldParentIndex.isValid()) {
    return false;
  }

  // get model data
  auto childItem = (MyItem *)childIndex.internalPointer();
  auto oldParentItem = (MyItem *)childIndex.parent().internalPointer();
  auto newParentItem = (MyItem *)parent.internalPointer();
  assert(childItem);
  assert(oldParentItem);
  if (!newParentItem) {
    return false;
  }

  if (oldParentItem == newParentItem) {
    // TODO
    return false;
  }

  // get scene node;
  auto childNode = childItem->node;
  auto oldParentGroup = oldParentItem->node->asGroup();
  auto newParentGroup = newParentItem->node->asGroup();
  assert(oldParentGroup && newParentGroup);

  ////////////////////////////////
  /// remove
  bool removed = false;
  beginRemoveRows(oldParentIndex, row, row);
  // remove from scene
  removed = oldParentGroup->removeChild(childIndex.row());
  if (removed) {
    // remove from model
    oldParentItem->children.takeAt(childIndex.row());
  }
  endRemoveRows();

  ////////////////////////////////
  /// add
  auto index = row >= 0 ? row : newParentItem->children.size();
  beginInsertRows(newParentIndex, index, index);
  if (removed) {
    // add to scene
    newParentGroup->insertChild(index, childNode);
    // add to model
    newParentItem->children.insert(index, childItem);
    childItem->parent = newParentItem;
  }
  endInsertRows();

  return false;
}

bool NodeTreeModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const { return true; }
