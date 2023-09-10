//
// Created by chudonghao on 2020/4/26.
//

#include "TestStencil.h"
#include <osg/StateSet>
#include <osg/Program>
#include <osg/Shader>
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/Geode>
#include <osg/Stencil>
#include <osg/Depth>
#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osgUtil/CullVisitor>
#include <iostream>

using namespace osg;
using namespace osgUtil;

class TestStencil::CullCallback : public osg::NodeCallback
{
public:
    bool run(osg::Object *object, osg::Object *data) override
    {
        TestStencil *o = static_cast<TestStencil *>(object);
        CullVisitor *cv = dynamic_cast<CullVisitor *>(data);
        if (cv)
        {
            // TODO cull
            //bool cull = cv->isCulled(o->getBound());
            //cv->pushStateSet(o->m_globalStateSet);

            cv->pushStateSet(o->m_firstPass);
            o->traverse(*cv);
            cv->popStateSet();
            cv->pushStateSet(o->m_secondPass);
            o->traverse(*cv);
            cv->popStateSet();
            //cv->pushStateSet(o->m_thirdPass);
            //o->traverse(*cv);
            //cv->popStateSet();

            //o->traverse(*cv);

            //cv->popStateSet();
        }
        return false;
    }
};

TestStencil::TestStencil()
{
    setCullCallback(new CullCallback);
    {
        m_firstPass = new StateSet();
        auto s = new Stencil();
        s->setFunction(Stencil::ALWAYS, 1, ~0u);
        s->setOperation(Stencil::ZERO, Stencil::ZERO, Stencil::ZERO);
        m_firstPass->setAttributeAndModes(s);
        auto d = new Depth(Depth::LESS, 0., 1.f, false);
        m_firstPass->setAttributeAndModes(d);
        auto c = new ColorMask(false, false, false, false);
        m_firstPass->setAttributeAndModes(c);
    }

    {
        m_secondPass = new StateSet();

        auto *s = new osg::Stencil;
        s->setFunction(osg::Stencil::LESS, 2, ~0u);
        s->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::INCR);
        m_secondPass->setAttributeAndModes(s);
    }

    m_globalStateSet = getOrCreateStateSet();
    m_globalStateSet->setMode(GL_BLEND, StateAttribute::ON);
}

void TestStencil::init()
{
    {
        auto gd = new Geode();
        addChild(gd);

        //auto sd = new ShapeDrawable(new Sphere(Vec3(), 20));
        //sd->setColor(Vec4(0,0.6,0,1));
        //gd->addDrawable(sd);
        auto geo = createTexturedQuadGeometry(Vec3(-5, -5, 0), Vec3(10, 0, 0), Vec3(0., 10, 0));
        auto ca = new Vec4Array;
        ca->push_back(Vec4(0.5, 0, 0, 0.5));
        geo->setColorArray(ca, Array::BIND_OVERALL);
        gd->addDrawable(geo);
    }
    {
        auto mt = new MatrixTransform(Matrix::translate(5, 5, 1));
        auto gd = new Geode();
        mt->addChild(gd);
        addChild(mt);

        //auto sd = new ShapeDrawable(new Sphere(Vec3(), 20));
        //gd->addDrawable(sd);

        auto geo = createTexturedQuadGeometry(Vec3(-5, -5, 0), Vec3(10, 0, 0), Vec3(0., 10, 0));
        auto ca = new Vec4Array;
        ca->push_back(Vec4(0.5, 0, 0, 0.5));
        geo->setColorArray(ca, Array::BIND_OVERALL);
        gd->addDrawable(geo);
    }
}
