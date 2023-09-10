//
// Created by chudonghao on 2020/7/5.
//

#include "NodeView.h"

#include <osg/Node>

#if __cplusplus >= 201703L
#include "OsgDataEdit.h"
using MatrixEdit = DecodeOsgDataEditType<osg::Matrix>::OsgDataEdit;
#else
#include <QWidget>
using MatrixEdit = QWidget;
#endif

#include "ui_NodeView.h"

using namespace osg;

NodeView::NodeView(QWidget *parent) : QWidget(parent)
{
    ui = new Ui;
    ui->setupUi(this);
}

NodeView::~NodeView()
{
    delete ui;
}

void NodeView::setNode(osg::Node *node)
{
    ref_ptr<Node> rpNode = m_node;
    if (node == rpNode)
    {
        return;
    }
    m_node = node;
    // reset
    if (!node)
    {
        ui->name_label->setText("");
        ui->type_label->setText("");
        return;
    }
    ui->name_label->setText(node->getName().empty() ? QString::fromUtf8("无名") : node->getName().c_str());
    ui->type_label->setText(node->className());
}
