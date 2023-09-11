//
// Created by chudonghao on 2020/3/19.
//

#ifndef OSGEDITOR_SRC_UI_TEXTURELISTMODEL_H
#define OSGEDITOR_SRC_UI_TEXTURELISTMODEL_H

#include <QAbstractItemModel>
#include <osg/observer_ptr>
#include <osg/ref_ptr>

namespace osg {
class StateSet;
class Texture;
class TexGen;
class StateAttribute;
}  // namespace osg
class TextureListModel : public QAbstractItemModel {
 public:
  TextureListModel(QObject *parent);

  QModelIndex index(int row, int column, const QModelIndex &parent) const override;

  QModelIndex parent(const QModelIndex &child) const override;

  int rowCount(const QModelIndex &parent) const override;

  int columnCount(const QModelIndex &parent) const override;

  QVariant data(const QModelIndex &index, int role) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  void SetStateSet(const osg::ref_ptr<osg::StateSet> &state_set);

  osg::ref_ptr<osg::Texture> GetTexture(const QModelIndex &index);

  osg::ref_ptr<osg::TexGen> GetTexGen(const QModelIndex &index);

 private:
  osg::observer_ptr<osg::StateSet> m_stateSet;

  enum EDepth {
    DEPTH_ZERO,  // fake root
    DEPTH_ONE,   // att
    DEPTH_TWO,   // tex
  };

  struct MyItem {
    MyItem *parent{};
    EDepth depth{};
    osg::StateAttribute *attribute{};
    int row{0};
    std::vector<MyItem *> children;
    QString type;
    QString unit;
  };
  MyItem *_root_item;
  std::list<MyItem *> _items;
};

#endif  // OSGEDITOR_SRC_UI_TEXTURELISTMODEL_H
