//
// Created by chudonghao on 2020/7/5.
//

#ifndef OSGEDITOR_SRC_UI_NODEVIEW_H
#define OSGEDITOR_SRC_UI_NODEVIEW_H

#include <QWidget>
#include <osg/observer_ptr>

namespace osg
{
class Node;
}

namespace Ui
{
class NodeView;
}

class NodeView : public QWidget
{
Q_OBJECT
public:
    explicit NodeView(QWidget *parent = nullptr);

    ~NodeView() override;

    void setNode(osg::Node *node);

private:
    using Super = QWidget;
    using Self = NodeView;
    using Ui = ::Ui::NodeView;
    Ui *ui{};
    osg::observer_ptr<osg::Node> m_node;
};


#endif //OSGEDITOR_SRC_UI_NODEVIEW_H
