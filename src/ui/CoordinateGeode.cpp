//
// Created by chudonghao on 2020/7/8.
//

#include "CoordinateGeode.h"
#include <iostream>
#include <osg/Version>
#include <osg/Vec3>
#include <osg/VertexAttribDivisor>
#include <osg/Program>
#include <osg/LineWidth>
#include <osg/NodeCallback>
#include <osg/Geometry>
#include <osgUtil/CullVisitor>

using namespace osg;

CoordinateGeode::CoordinateGeode()
{
    auto stateSet = new StateSet;
    stateSet->setMode(GL_BLEND, StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, StateAttribute::OFF);
    setStateSet(stateSet);

    m_axisMatrix = new RefMatrix();
    m_xNetMatrix = new RefMatrix();
    m_yNetMatrix = new RefMatrix();
    setupAxisGeometry();
    setupNetGeometry();
}
void CoordinateGeode::traverse(NodeVisitor &nv)
{
#if OSG_VERSION_LESS_THAN(3, 6, 0)
    auto cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);
#else
    auto cv = nv.asCullVisitor();
#endif
    if (cv)
    {
        auto eye = cv->getEyeLocal();
        auto dir = cv->getLookVectorLocal();
        auto x0 = eye.x();
        auto y0 = eye.y();
        auto z0 = eye.z();
        auto m = dir.x();
        auto n = dir.y();
        auto p = dir.z();
        auto x = (-z0) / p * m + x0;
        auto y = (-z0) / p * n + y0;

        if (std::isnan(x)) { x = 0; }
        if (std::isnan(y)) { y = 0; }

        Vec3 intersection = {x, y, 0};
        auto distance = (eye - intersection).length();
        if (distance > 100.)
        {
            dir.normalize();
            intersection = eye + dir * 100;
            intersection.z() = 0;
        }
        float pixelSize = 2 / std::max(cv->pixelSize(intersection, 1), std::numeric_limits<float>::epsilon());

        float a0 = std::log10(pixelSize) + 2.f;
        float a1 = std::floor(a0);
        float a2 = a0 - a1;

        float step = std::pow(10, a1);
        float scale = std::max(1000.f, 1000.f * step);
        Matrix netScale = Matrix::scale(scale, step, 0);
        m_NgAlphaUniform->set(a2);
        //m_NgDrawArrays->setNumInstances(1000);

        m_xNetMatrix->set(netScale * *cv->getModelViewMatrix());
        cv->pushModelViewMatrix(m_xNetMatrix, osg::Transform::RELATIVE_RF);
        m_NetGeometry->accept(nv);
        cv->popModelViewMatrix();

        m_yNetMatrix->set(netScale * Matrix::rotate(PI_2, Vec3(0, 0, 1)) * *cv->getModelViewMatrix());
        cv->pushModelViewMatrix(m_yNetMatrix, osg::Transform::RELATIVE_RF);
        m_NetGeometry->accept(nv);
        cv->popModelViewMatrix();

        m_axisMatrix->set(Matrix::scale(Vec3{scale, scale, scale}) * *cv->getModelViewMatrix());
        //std::cout << netCenter.x() << ' ' << netCenter.y() << ' ' << netCenter.z() << std::endl;
        cv->pushModelViewMatrix(m_axisMatrix, Transform::RELATIVE_RF);
        m_axisGeometry->accept(nv);
        cv->popModelViewMatrix();
    }
}
void CoordinateGeode::setupAxisGeometry()
{
    m_axisGeometry = new Geometry;

    auto va = new Vec3Array();
    va->push_back({-1, 0, 0});
    va->push_back({1, 0, 0});
    va->push_back({0, -1, 0});
    va->push_back({0, 1, 0});
    va->push_back({0, 0, -1});
    va->push_back({0, 0, 1});
    m_axisGeometry->setVertexArray(va);

    auto ca = new Vec3Array();
    ca->push_back({1, 0, 0});
    ca->push_back({1, 0, 0});
    ca->push_back({0, 1, 0});
    ca->push_back({0, 1, 0});
    ca->push_back({0, 0, 1});
    ca->push_back({0, 0, 1});
    m_axisGeometry->setColorArray(ca, Array::BIND_PER_VERTEX);

    auto na = new Vec3Array();
    na->push_back({0, 0, 1});
    m_axisGeometry->setNormalArray(na, Array::BIND_OVERALL);

    auto ps = new DrawArrays(GL_LINES, 0, va->size());
    m_axisGeometry->addPrimitiveSet(ps);
    m_axisGeometry->setCullingActive(false);
}
void CoordinateGeode::setupNetGeometry()
{
    static auto vss = "#version 330 compatibility\n"
                      "uniform bool u_MainNet;\n"
                      "uniform int u_NumInstance;\n"
                      "uniform float u_Alpha;\n"
                      "out float v_ColorFactor;\n"
                      "void main(){\n"
                      "    if(gl_InstanceID == 0){ gl_Position = vec4(0); return; }"
                      "    vec4 pos = gl_Vertex;\n"
                      "    int s = int(mod(gl_InstanceID, 2));\n"
                      "    int netID = gl_InstanceID / 2;\n"
                      "    if(s != 0){ netID += 1; }\n"
                      "    else { netID = -netID; }\n"
                      "    if(mod(netID, 10) == 0){ v_ColorFactor = 0; }\n"
                      "    else{ v_ColorFactor = u_Alpha; }\n"
                      "    pos.y += netID;\n"
                      "    gl_Position = gl_ModelViewProjectionMatrix * pos;\n"
                      "}";
    static auto fss = "#version 330 compatibility\n"
                      "in float v_ColorFactor;\n"
                      "void main(){\n"
                      "    gl_FragColor.rgb = vec3(1);\n"
                      "    gl_FragColor.a = 1 - v_ColorFactor;\n"
                      "}";
    auto vs = new Shader(Shader::VERTEX, vss);
    auto fs = new Shader(Shader::FRAGMENT, fss);
    auto program = new Program;
    program->addShader(vs);
    program->addShader(fs);

    auto stateSet = new StateSet;
    stateSet->setAttribute(program);

    m_NgAlphaUniform = stateSet->getOrCreateUniform("u_Alpha", Uniform::FLOAT);
    m_NgNumInstanceUniform = stateSet->getOrCreateUniform("u_NumInstance", Uniform::INT);
    m_NgNumInstanceUniform->set(20000);

    m_NetGeometry = new Geometry;
    m_NetGeometry->setStateSet(stateSet);

    auto va = new Vec3Array();
    va->push_back({-1, 0, 0});
    va->push_back({1, 0, 0});
    m_NetGeometry->setVertexArray(va);

    auto ca = new Vec3Array();
    ca->push_back({1, 0, 0});
    ca->push_back({1, 0, 0});
    m_NetGeometry->setColorArray(ca, Array::BIND_PER_VERTEX);

    auto na = new Vec3Array();
    na->push_back({0, 0, 1});
    m_NetGeometry->setNormalArray(na, Array::BIND_OVERALL);

    m_NgDrawArrays = new DrawArrays(GL_LINES, 0, va->size(), 2000);
    m_NetGeometry->addPrimitiveSet(m_NgDrawArrays);

    m_NetGeometry->setUseDisplayList(false);
    m_NetGeometry->setUseVertexBufferObjects(true);
}
