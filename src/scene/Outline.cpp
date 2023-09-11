//
// Created by chudonghao on 2020/7/7.
//

#include "Outline.h"

#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/PolygonMode>
#include <osg/StateSet>
#include <osg/Stencil>
#include <osg/Texture2D>
#include <osg/Version>
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

Outline::Outline() {
  auto program = new Program;
  auto vs = new Shader(Shader::VERTEX);
  vs->setShaderSource(vs2);
  auto fs = new Shader(Shader::FRAGMENT);
  fs->setShaderSource(fs2);
  program->addShader(vs);
  program->addShader(fs);
  {
    _first_pass = new StateSet();
    // 模板测试
    auto stencil = new Stencil();
    stencil->setFunction(Stencil::NEVER, 1, ~0u);
    stencil->setOperation(Stencil::REPLACE, Stencil::REPLACE, Stencil::REPLACE);
    _first_pass->setAttributeAndModes(stencil);
    // 深度测试
    auto depth = new Depth(Depth::LESS, 0., 1.f, false);
    _first_pass->setAttributeAndModes(depth);
    // More
  }

  {
    const size_t Override_On = StateAttribute::OVERRIDE | StateAttribute::ON;
    const size_t Override_Off = StateAttribute::OVERRIDE | StateAttribute::OFF;

    _second_pass = new StateSet();
    // 模板测试
    auto *stencil = new Stencil;
    stencil->setFunction(Stencil::EQUAL, 0, ~0u);
    stencil->setOperation(Stencil::KEEP, Stencil::KEEP, Stencil::KEEP);
    _second_pass->setAttributeAndModes(stencil);
    // 线框模式
    PolygonMode *polyMode = new PolygonMode;
    polyMode->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::LINE);
    _second_pass->setAttributeAndModes(polyMode, Override_On);
    // 线宽
    _line_width = new LineWidth;
    _line_width->setWidth(_width);
    _second_pass->setAttributeAndModes(_line_width, Override_On);
    // 材质
    _material = new Material;
    _material->setColorMode(Material::OFF);
    _material->setAmbient(Material::FRONT_AND_BACK, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    _material->setDiffuse(Material::FRONT_AND_BACK, _color);
    _material->setSpecular(Material::FRONT_AND_BACK, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    _material->setEmission(Material::FRONT_AND_BACK, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    _second_pass->setAttributeAndModes(_material, Override_On);

    // 关闭Alpha融合
    _second_pass->setMode(GL_BLEND, Override_Off);
    // 关闭深度测试
    _second_pass->setMode(GL_DEPTH_TEST, Override_Off);
    // 关闭纹理
    _second_pass->setTextureMode(0, GL_TEXTURE_1D, Override_Off);
    _second_pass->setTextureMode(0, GL_TEXTURE_2D, Override_Off);
    _second_pass->setTextureMode(0, GL_TEXTURE_3D, Override_Off);
    // 着色器
    _second_pass->setAttributeAndModes(program, Override_On);
  }
}

const Vec4 &Outline::GetColor() const { return _color; }

void Outline::SetColor(const osg::Vec4 &color) {
  _color = color;
  _material->setDiffuse(Material::FRONT_AND_BACK, color);
}

float Outline::GetWidth() const { return _width; }

void Outline::SetWidth(float width) {
  _width = width;
  _line_width->setWidth(width);
}

void Outline::traverse(NodeVisitor &nv) {
  auto cv = nv.asCullVisitor();
  if (cv) {
    cv->pushStateSet(_first_pass);
    Group::traverse(nv);
    cv->popStateSet();
    cv->pushStateSet(_second_pass);
    Group::traverse(nv);
    cv->popStateSet();
  }
}
