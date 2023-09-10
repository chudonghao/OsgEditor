//
// Created by chudonghao on 2020/3/19.
//

#ifndef OSGEDITOR_SRC_UI_STATESETVIEW_H
#define OSGEDITOR_SRC_UI_STATESETVIEW_H

#include <QWidget>
#include <osg/observer_ptr>

namespace osg
{
class StateSet;
class Texture;
class TexGen;
}

namespace Ui
{
class StateSetView;
}

class TextureListModel;
class StateSetView : public QWidget
{
Q_OBJECT;
public:
    explicit StateSetView(QWidget *parent = nullptr);

    ~StateSetView() override;

    void setStateSet(osg::StateSet *stateSet);

private:
    using Ui = ::Ui::StateSetView;
    Ui *ui{};
    osg::observer_ptr<osg::StateSet> m_stateSet;
    osg::observer_ptr<osg::Texture> m_texture;
    osg::observer_ptr<osg::TexGen> m_texgen;
    TextureListModel *m_textureListModel;
    void reset();
    void setupMaterialGroupBox();
};



#endif //OSGEDITOR_SRC_UI_STATESETVIEW_H
