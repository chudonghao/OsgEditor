//
// Created by chudonghao on 2023/9/11.
//

#ifndef INC_2023_9_11_EEFE9599A6CA423388DB7136E1365CE8_H_
#define INC_2023_9_11_EEFE9599A6CA423388DB7136E1365CE8_H_

#include <QAbstractItemModel>
#include <osg/Node>

class NodeDetailModel : public QAbstractItemModel {
 public:
  using QAbstractItemModel::QAbstractItemModel;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  QModelIndex index(int row, int column, const QModelIndex& parent) const override;

  QModelIndex parent(const QModelIndex& child) const override;

  int rowCount(const QModelIndex& parent) const override;

  int columnCount(const QModelIndex& parent) const override;

  QVariant data(const QModelIndex& index, int role) const override;

  void SetNode(osg::ref_ptr<osg::Node> node);

 private:
  struct TypeItem;
  struct PropertyItem;
  struct GroupItem;

  struct Item {
    GroupItem* parent{};
    Item(GroupItem* parent);
    virtual ~Item() {}
    virtual QVariant NameData(int role) { return {}; }
    virtual QVariant ValueData(int role) { return {}; }
    virtual TypeItem* AsTypeItem() { return nullptr; }
    virtual PropertyItem* AsPropertyItem() { return nullptr; }
    virtual GroupItem* AsGroupItem() { return nullptr; }
  };

  struct GroupItem : Item {
    QList<Item*> children;

    using Item::Item;
    virtual ~GroupItem() { qDeleteAll(children); }
    GroupItem* AsGroupItem() override { return this; }
  };

  struct VirtualItem : GroupItem {
    osg::ref_ptr<osg::Node> node;

    explicit VirtualItem() : GroupItem(nullptr) {}
  };

  struct TypeItem : GroupItem {
    QString name;

    using GroupItem::GroupItem;
    TypeItem* AsTypeItem() override { return this; }

    QVariant NameData(int role) override {
      if (role == Qt::DisplayRole) {
        return name;
      }
      return {};
    }
  };

  struct PropertyItem : Item {
    QString name;

    using Item::Item;
    PropertyItem* AsPropertyItem() override { return this; }
    QVariant NameData(int role) override {
      if (role == Qt::DisplayRole) {
        return name;
      }
      return {};
    }
  };

  struct NamePropertyItem : PropertyItem {
    VirtualItem* node_item{};

    NamePropertyItem(GroupItem* parent) : PropertyItem(parent) { name = "Name"; }
    PropertyItem* AsPropertyItem() override { return this; }
    QVariant ValueData(int role) override {
      if (role == Qt::DisplayRole) {
        return QString::fromLocal8Bit(node_item->node->getName().c_str());
      }
      return {};
    }
  };

  VirtualItem* _virtual_item{};
};

#endif  // INC_2023_9_11_EEFE9599A6CA423388DB7136E1365CE8_H_
