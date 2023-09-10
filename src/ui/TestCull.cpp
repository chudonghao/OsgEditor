//
// Created by chudonghao on 2020/4/10.
//

#include "TestCull.h"
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/NodeCallback>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgUtil/CullVisitor>
#include <iostream>

using namespace osg;
using namespace std;

class MyCallback : public osg::NodeCallback
{
public:
    bool run(osg::Object *object, osg::Object *data) override
    {
        osgUtil::CullVisitor *nv = dynamic_cast<osgUtil::CullVisitor *>(data);
        Group *g = dynamic_cast<Group *>(object);

        cout << "------------------------------------\n";
        cout << g << ' ';
        cout << g->getBound().center().x() << ' ';
        cout << g->getBound().center().y() << ' ';
        cout << nv->isCulled(g->getBound());
        cout << '\n';

        return false;
    }
};

TestCull::TestCull()
{
    for (int i = 0; i < 10; ++i)
    {
        MatrixTransform *mt = new MatrixTransform(Matrix::translate(Vec3(i*1000,0,0)));
        addChild(mt);

        mt->setCullCallback(new MyCallback);

        auto geode = new Geode;
        mt->addChild(geode);

        auto sd = new ShapeDrawable(new Sphere(Vec3(),200));
        geode->addDrawable(sd);
    }
}
