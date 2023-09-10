//
// Created by chudonghao on 2020/3/30.
//

#include "TestOutLine.h"
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

static const char *vs1 =
    "#version 330 compatibility\n"
    "\n"
    "uniform mat4 u_scale;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * u_scale * gl_Vertex;\n"
    "}";
static const char *fs1 =
    "#version 330 compatibility\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(0,0,0,1);\n"
    "}";
static const char *vs2 =
    "#version 330 compatibility\n"
    "\n"
    "void main() {\n"
    "    gl_Position = ftransform();\n"
    "}";
static const char *fs2 =
    "#version 330 compatibility\n"
    "\n"
    "void main() {\n"
    "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}";

using namespace osg;
using namespace osgUtil;

class TestOutLine::CullCallback : public osg::NodeCallback
{
public:
    bool run(osg::Object *object, osg::Object *data) override
    {
        TestOutLine *o = static_cast<TestOutLine *>(object);
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

            o->traverse(*cv);

            //cv->popStateSet();
        }
        return false;
    }
};

TestOutLine::TestOutLine()
{
    setCullCallback(new CullCallback);
    m_globalStateSet = new StateSet();
    {
        m_firstPass = new StateSet();
        auto s = new Stencil();
        s->setFunction(Stencil::ALWAYS, 1, ~0u);
        s->setOperation(Stencil::KEEP, Stencil::KEEP, Stencil::REPLACE);
        m_firstPass->setAttributeAndModes(s);
        auto d = new Depth(Depth::LESS, 0., 1.f, false);
        m_firstPass->setAttributeAndModes(d);
        auto c = new ColorMask(false, false, false, false);
        m_firstPass->setAttributeAndModes(c);

        //m_firstPass->getOrCreateUniform("u_scale", Uniform::FLOAT_MAT4)->set(Matrix::scale(0.9, 0.9, 0.9));
        //auto program = new Program;
        //auto vs = new Shader(Shader::VERTEX);
        //vs->setShaderSource(vs1);
        //auto fs = new Shader(Shader::FRAGMENT);
        //fs->setShaderSource(fs1);
        //program->addShader(vs);
        //program->addShader(fs);
        //m_firstPass->setAttributeAndModes(program);
    }

    {
        m_secondPass = new StateSet();

        auto *s = new osg::Stencil;
        s->setFunction(osg::Stencil::EQUAL, 0, ~0u);
        s->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
        m_secondPass->setAttributeAndModes(s);

        size_t Override_On = StateAttribute::OVERRIDE | StateAttribute::ON;
        size_t Override_Off = StateAttribute::OVERRIDE | StateAttribute::OFF;

        // draw back-facing polygon lines
        osg::PolygonMode *polyMode = new osg::PolygonMode;
        polyMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        m_secondPass->setAttributeAndModes(polyMode, Override_On);

        // outline width
        auto _lineWidth = new osg::LineWidth;
        _lineWidth->setWidth(5.f);
        m_secondPass->setAttributeAndModes(_lineWidth, Override_On);

        // outline color/material
        auto _material = new osg::Material;
        _material->setColorMode(osg::Material::OFF);
        const osg::Material::Face face = osg::Material::FRONT_AND_BACK;
        _material->setAmbient(face, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        _material->setDiffuse(face, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        _material->setSpecular(face, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        _material->setEmission(face, Vec4(1, 0, 0, 1));
        m_secondPass->setAttributeAndModes(_material, Override_On);

        // disable modes
        m_secondPass->setMode(GL_BLEND, Override_Off);
        //state->setMode(GL_DEPTH_TEST, Override_Off);
        m_secondPass->setTextureMode(0, GL_TEXTURE_1D, Override_Off);
        m_secondPass->setTextureMode(0, GL_TEXTURE_2D, Override_Off);
        m_secondPass->setTextureMode(0, GL_TEXTURE_3D, Override_Off);

        auto program = new Program;
        auto vs = new Shader(Shader::VERTEX);
        vs->setShaderSource(vs2);
        auto fs = new Shader(Shader::FRAGMENT);
        fs->setShaderSource(fs2);
        program->addShader(vs);
        program->addShader(fs);
        m_secondPass->setAttributeAndModes(program);
    }

    auto stateSet = getOrCreateStateSet();
    {
        //osg::Stencil *s = new osg::Stencil;
        //s->setFunction(osg::Stencil::EQUAL, 1, ~0u);
        //s->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
        //stateSet->setAttributeAndModes(s);

        //auto program = new Program;
        //auto vs = new Shader(Shader::VERTEX);
        //vs->setShaderSource(vs3);
        //auto fs = new Shader(Shader::FRAGMENT);
        //fs->setShaderSource(fs3);
        //program->addShader(vs);
        //program->addShader(fs);
        //stateSet->setAttributeAndModes(program);
    }
}

void TestOutLine::init()
{
    {
        auto gd = new Geode();
        addChild(gd);

        //auto sd = new ShapeDrawable(new Sphere(Vec3(), 20));
        //sd->setColor(Vec4(0,0.6,0,1));
        //gd->addDrawable(sd);

        gd->addDrawable(createTexturedQuadGeometry(Vec3(-5, -5, 0), Vec3(10, 0, 0), Vec3(0., 10, 0)));
    }
    {
        auto mt = new MatrixTransform(Matrix::translate(5, 5, 1));
        auto gd = new Geode();
        mt->addChild(gd);
        addChild(mt);

        //auto sd = new ShapeDrawable(new Sphere(Vec3(), 20));
        //gd->addDrawable(sd);

        gd->addDrawable(createTexturedQuadGeometry(Vec3(-5, -5, 0), Vec3(10, 0, 0), Vec3(0., 10, 0)));
    }
}
