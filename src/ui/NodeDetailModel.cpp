//
// Created by chudonghao on 2023/9/11.
//

#include "NodeDetailModel.h"

QVariant NodeDetailModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    switch (section) {
      case 0:
        return QString("Name");
      case 1:
        return QString("Value");
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex NodeDetailModel::index(int row, int column, const QModelIndex& parent) const {
  Item* item;
  if (!parent.isValid()) {
    item = _virtual_item;
  } else {
    item = static_cast<Item*>(parent.internalPointer());
  }
  if (!item) {
    return {};
  }

  auto gi = item->AsGroupItem();
  if (!gi) {
    return {};
  }

  if (row >= gi->children.size()) {
    return {};
  } else {
    return createIndex(row, column, gi->children.at(row));
  }
}

QModelIndex NodeDetailModel::parent(const QModelIndex& child) const {
  Item* item;
  if (!child.isValid()) {
    item = _virtual_item;
  } else {
    item = static_cast<Item*>(child.internalPointer());
  }
  if (!item) {
    return {};
  }

  if (!item->parent) {
    return {};
  }

  int row = item->parent->children.indexOf(item);
  if (row >= 0) {
    return createIndex(row, 0, item->parent);
  }

  return {};
}

int NodeDetailModel::rowCount(const QModelIndex& parent) const {
  if (!_virtual_item) {
    return 0;
  }

  Item* item;
  if (!parent.isValid()) {
    item = _virtual_item;
  } else {
    item = static_cast<Item*>(parent.internalPointer());
  }

  if (auto gi = item->AsGroupItem()) {
    return gi->children.size();
  }
  return 0;
}

int NodeDetailModel::columnCount(const QModelIndex& parent) const { return 2; }

QVariant NodeDetailModel::data(const QModelIndex& index, int role) const {
  if (!_virtual_item) {
    return 0;
  }

  Item* item;
  if (!index.isValid()) {
    item = _virtual_item;
  } else {
    item = static_cast<Item*>(index.internalPointer());
  }

  return index.column() == 0 ? item->NameData(role) : item->ValueData(role);
}

void NodeDetailModel::SetNode(osg::ref_ptr<osg::Node> node) {
  beginResetModel();

  delete _virtual_item;

  _virtual_item = new VirtualItem;
  _virtual_item->node = node;

  // node
  {
    auto type = new TypeItem(_virtual_item);
    type->name = "Node";

    auto name = new NamePropertyItem(type);
    name->node_item = _virtual_item;
  }

  endResetModel();
}

NodeDetailModel::Item::Item(NodeDetailModel::GroupItem* parent) {
  if (parent) {
    parent->children.append(this);
  }
  this->parent = parent;
}
