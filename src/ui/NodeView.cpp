//
// Created by chudonghao on 2020/7/5.
//

#include "NodeView.h"

#include <osg/Node>

#include "OsgDataEdit.h"
using MatrixEdit = DecodeOsgDataEditType<osg::Matrix>::OsgDataEdit;

#include "ui_NodeView.h"

using namespace osg;

NodeView::NodeView(QWidget *parent) : QWidget(parent) {
  ui = new Ui;
  ui->setupUi(this);
}

NodeView::~NodeView() { delete ui; }

void NodeView::SetNode(osg::Node *node) {
  ref_ptr<Node> rpNode = _node;
  if (node == rpNode) {
    return;
  }
  _node = node;
  // reset
  if (!node) {
    ui->name_label->setText("");
    ui->type_label->setText("");
    return;
  }
  ui->name_label->setText(node->getName().empty() ? QString::fromUtf8("无名") : node->getName().c_str());
  ui->type_label->setText(node->className());
}
