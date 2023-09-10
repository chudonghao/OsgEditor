//
// Created by chudonghao on 2020/3/25.
//

#include "Visitor.h"
#include <osg/Geometry>

using namespace osg;

Visitor::Visitor() : NodeVisitor(TRAVERSE_ALL_CHILDREN) {

}

void Visitor::apply(osg::Geometry &geo)
{
    geo.setUseVertexBufferObjects(true);
    geo.setUseDisplayList(false);
    geo.setInitialBound(BoundingBox(-10000,-10000,-1,10000,10000,1));
    auto p = geo.getPrimitiveSet(0);
    if(p)
    {
        p->setNumInstances(10000);
    }
}


