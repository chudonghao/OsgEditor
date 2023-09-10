//
// Created by chudonghao on 2020/4/13.
//

#include "TestBulletLine.h"

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/LineWidth>

using namespace std;
using namespace osg;

class TestBulletLine::UpdateCallback : public NodeCallback
{
public:
    UpdateCallback(TestBulletLine *t) : t(t) {}

    bool run(osg::Object *object, osg::Object *data) override
    {
        auto nv = data->asNodeVisitor();
        if (nv)
        {
            if (nv->getVisitorType()==NodeVisitor::UPDATE_VISITOR)
            {
                float ct = nv->getFrameStamp()->getSimulationTime();

                if (t->m_length < t->m_v*(ct - t->m_startTime))
                {
                    t->m_startTime = ct;
                    t->getOrCreateStateSet()->getOrCreateUniform("u_StartTime", Uniform::FLOAT)->set(ct);
                }
            }
        }
        return NodeCallback::run(object, data);
    }

    TestBulletLine *t;
};

TestBulletLine::TestBulletLine()
{

    auto geode = new Geode;
    addChild(geode);

    auto geo = new Geometry;
    geode->addDrawable(geo);

    {
        auto va = new Vec3Array;
        va->push_back(Vec3(0, 0, 0));
        va->push_back(Vec3(0, 0, 20));

        auto ca = new Vec3Array;
        ca->push_back(Vec3(1, 0, 0));
        ca->push_back(Vec3(0, 1, 0));

        auto na = new Vec3Array;
        na->push_back(Vec3(0, 0, 1));

        auto ta = new FloatArray();
        ta->push_back(0);
        ta->push_back(20);

        geo->setVertexArray(va);
        geo->setColorArray(ca, Array::BIND_PER_VERTEX);
        geo->setNormalArray(na, Array::BIND_OVERALL);
        geo->setTexCoordArray(0, ta, Array::BIND_PER_VERTEX);
        geo->addPrimitiveSet(new DrawArrays(GL_LINES, 0, va->size()));

    }

    {
        auto stateSet = getOrCreateStateSet();

        auto program = new Program;
        auto vs = new Shader(Shader::VERTEX);
        vs->loadShaderSourceFromFile("bullet.vert");
        auto fs = new Shader(Shader::FRAGMENT);
        fs->loadShaderSourceFromFile("bullet.frag");
        program->addShader(vs);
        program->addShader(fs);

        stateSet->setAttributeAndModes(program);
        m_v = 50.f;
        stateSet->getOrCreateUniform("u_V", Uniform::FLOAT)->set(m_v);
        stateSet->getOrCreateUniform("u_StartTime", Uniform::FLOAT)->set(0.f);
        stateSet->getOrCreateUniform("u_Size", Uniform::FLOAT)->set(10.f);
        stateSet->setAttributeAndModes(new LineWidth(10.f));
        stateSet->setMode(GL_BLEND, StateAttribute::ON);
    }

    setUpdateCallback(new UpdateCallback(this));

}
