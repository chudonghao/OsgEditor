//
// Created by chudonghao on 2020/3/25.
//

#ifndef OSGEDITOR_SRC_UI_VISITOR_H
#define OSGEDITOR_SRC_UI_VISITOR_H

#include <osg/NodeVisitor>

class Visitor : public osg::NodeVisitor
{
public:
    Visitor();
    void apply(osg::Geometry &geo) override;
};



#endif //OSGEDITOR_SRC_UI_VISITOR_H
