//
// Created by chudonghao on 2020/7/7.
//

#include "osgCDH/Outline.h"

#include <osg/Version>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/PolygonMode>
#include <osg/StateSet>
#include <osg/Stencil>
#include <osg/Texture2D>

#include <osgUtil/CullVisitor>

using namespace osg;

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
        "    gl_FragColor = gl_FrontMaterial.diffuse;\n"
        "}";

osgCDH::Outline::Outline()
{
    auto program = new Program;
    auto vs = new Shader(Shader::VERTEX);
    vs->setShaderSource(vs2);
    auto fs = new Shader(Shader::FRAGMENT);
    fs->setShaderSource(fs2);
    program->addShader(vs);
    program->addShader(fs);
    {
        m_firstPass = new StateSet();
        // 模板测试
        auto stencil = new Stencil();
        stencil->setFunction(Stencil::NEVER, 1, ~0u);
        stencil->setOperation(Stencil::REPLACE, Stencil::REPLACE, Stencil::REPLACE);
        m_firstPass->setAttributeAndModes(stencil);
        // 深度测试
        auto depth = new Depth(Depth::LESS, 0., 1.f, false);
        m_firstPass->setAttributeAndModes(depth);
        // More
    }

    {
        const size_t Override_On = StateAttribute::OVERRIDE | StateAttribute::ON;
        const size_t Override_Off = StateAttribute::OVERRIDE | StateAttribute::OFF;

        m_secondPass = new StateSet();
        // 模板测试
        auto *stencil = new Stencil;
        stencil->setFunction(Stencil::EQUAL, 0, ~0u);
        stencil->setOperation(Stencil::KEEP, Stencil::KEEP, Stencil::KEEP);
        m_secondPass->setAttributeAndModes(stencil);
        // 线框模式
        PolygonMode *polyMode = new PolygonMode;
        polyMode->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::LINE);
        m_secondPass->setAttributeAndModes(polyMode, Override_On);
        // 线宽
        m_lineWidth = new LineWidth;
        m_lineWidth->setWidth(m_width);
        m_secondPass->setAttributeAndModes(m_lineWidth, Override_On);
        // 材质
        m_material = new Material;
        m_material->setColorMode(Material::OFF);
        m_material->setAmbient(Material::FRONT_AND_BACK, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        m_material->setDiffuse(Material::FRONT_AND_BACK, m_color);
        m_material->setSpecular(Material::FRONT_AND_BACK, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        m_material->setEmission(Material::FRONT_AND_BACK, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        m_secondPass->setAttributeAndModes(m_material, Override_On);

        // 关闭Alpha融合
        m_secondPass->setMode(GL_BLEND, Override_Off);
        // 关闭深度测试
        m_secondPass->setMode(GL_DEPTH_TEST, Override_Off);
        // 关闭纹理
        m_secondPass->setTextureMode(0, GL_TEXTURE_1D, Override_Off);
        m_secondPass->setTextureMode(0, GL_TEXTURE_2D, Override_Off);
        m_secondPass->setTextureMode(0, GL_TEXTURE_3D, Override_Off);
        // 着色器
        m_secondPass->setAttributeAndModes(program, Override_On);
    }
}
const Vec4 &osgCDH::Outline::getColor() const
{
    return m_color;
}
void osgCDH::Outline::setColor(const Vec4 &color)
{
    m_color = color;
    m_material->setDiffuse(Material::FRONT_AND_BACK, color);
}
float osgCDH::Outline::getWidth() const
{
    return m_width;
}
void osgCDH::Outline::setWidth(float width)
{
    m_width = width;
    m_lineWidth->setWidth(width);
}
void osgCDH::Outline::traverse(NodeVisitor &nv)
{
#if OSG_VERSION_LESS_THAN(3, 6, 0)
    auto cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);
#else
    auto cv = nv.asCullVisitor();
#endif
    if(cv)
    {
        cv->pushStateSet(m_firstPass);
        Group::traverse(nv);
        cv->popStateSet();
        cv->pushStateSet(m_secondPass);
        Group::traverse(nv);
        cv->popStateSet();
    }
}
