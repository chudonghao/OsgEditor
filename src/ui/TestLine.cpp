//
// Created by chudonghao on 2020/4/7.
//

#include "TestLine.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Vec3>
#include <osg/Shape>
#include <osg/LineWidth>
#include <osg/Hint>
#include <osg/Point>
#include <osg/BlendFunc>
#include <algorithm>
#include <osg/Depth>

using namespace std;
using namespace osg;

TestLine::TestLine()
{
    const float rand = 3.1415*5/180;

    Geometry *geometry = new Geometry;
    auto va = new Vec3Array;
    auto ca = new Vec4Array;
    auto na = new Vec3Array;
    for (int i = 0; i < 30; ++i)
    {
        va->push_back(Vec3(cos(rand*i)*30, sin(rand*i)*30, i));
        float a = std::min(1.f, (float) i/15);
        ca->push_back(Vec4(0, 1, 1, a));
    }
    for (int i = 0; i < 30; ++i)
    {
        va->push_back(Vec3(cos(rand*i)*30, sin(rand*i)*30, i));
        float a = std::min(1.f, (float) (30 - i)/5);
        ca->push_back(Vec4(0, 1, 1, a));
    }
    for (int i = 0; i < va->size() - 1; ++i)
    {
        na->push_back(va->at(i + 1) - va->at(i));
    }
    na->push_back(Vec3(1, 0, 0));

    geometry->setVertexArray(va);

    geometry->setNormalArray(na, Array::BIND_PER_VERTEX);

    geometry->setColorArray(ca, Array::BIND_PER_VERTEX);

    geometry->addPrimitiveSet(new DrawArrays(GL_LINE_STRIP, 0, va->size()));

    Geode *geode = new Geode;
    geode->addDrawable(geometry);

    // group add
    addChild(geode);

    // group stateSet
    auto stateSet = getOrCreateStateSet();
    stateSet->setMode(GL_BLEND, StateAttribute::ON);
    stateSet->setAttributeAndModes(new Depth(Depth::LESS, 0, 1, false));

    auto program = new Program;
    auto vs = new Shader(Shader::VERTEX);
    vs->loadShaderSourceFromFile("line.vert");
    auto gs = new Shader(Shader::GEOMETRY);
    gs->loadShaderSourceFromFile("line.geom");
    auto fs = new Shader(Shader::FRAGMENT);
    fs->loadShaderSourceFromFile("line.frag");
    program->addShader(vs);
    program->addShader(gs);
    program->addShader(fs);

    stateSet->setAttributeAndModes(program);
    stateSet->getOrCreateUniform("u_Width",Uniform::FLOAT)->set(10.f);
}
