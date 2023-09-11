//
// Created by chudonghao on 2020/3/19.
//

#ifndef OSGEDITOR_SRC_UI_STATESETVIEW_H
#define OSGEDITOR_SRC_UI_STATESETVIEW_H

#include <QWidget>
#include <osg/observer_ptr>

namespace osg {
class StateSet;
class Texture;
class TexGen;
}  // namespace osg

namespace Ui {
class StateSetView;
}

class TextureListModel;
class StateSetView : public QWidget {
  Q_OBJECT;

 public:
  explicit StateSetView(QWidget *parent = nullptr);

  ~StateSetView() override;

  void setStateSet(osg::StateSet *stateSet);

 private:
  using Ui = ::Ui::StateSetView;
  Ui *ui{};
  osg::observer_ptr<osg::StateSet> _state_set;
  osg::observer_ptr<osg::Texture> _texture;
  osg::observer_ptr<osg::TexGen> _texgen;
  TextureListModel *_texture_list_model;
  void Reset();
  void SetupMaterialGroupBox();
};

#endif  // OSGEDITOR_SRC_UI_STATESETVIEW_H
