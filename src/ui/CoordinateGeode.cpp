//
// Created by chudonghao on 2020/7/8.
//

#include "CoordinateGeode.h"

#include <iostream>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/NodeCallback>
#include <osg/Program>
#include <osg/Vec3>
#include <osg/Version>
#include <osg/VertexAttribDivisor>
#include <osgUtil/CullVisitor>

using namespace osg;

CoordinateGeode::CoordinateGeode() {
  auto state_set = new StateSet;
  state_set->setMode(GL_BLEND, StateAttribute::ON);
  state_set->setMode(GL_LIGHTING, StateAttribute::OFF);
  setStateSet(state_set);

  _axis_matrix = new RefMatrix();
  _x_net_matrix = new RefMatrix();
  _y_net_matrix = new RefMatrix();
  SetupAxisGeometry();
  SetupNetGeometry();
}
void CoordinateGeode::traverse(NodeVisitor &nv) {
  auto cv = nv.asCullVisitor();
  if (cv) {
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

    if (std::isnan(x)) {
      x = 0;
    }
    if (std::isnan(y)) {
      y = 0;
    }

    Vec3 intersection = {x, y, 0};
    auto distance = (eye - intersection).length();
    if (distance > 100.) {
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
    _ng_alpha_uniform->set(a2);
    // m_NgDrawArrays->setNumInstances(1000);

    _x_net_matrix->set(netScale * *cv->getModelViewMatrix());
    cv->pushModelViewMatrix(_x_net_matrix, osg::Transform::RELATIVE_RF);
    _net_geometry->accept(nv);
    cv->popModelViewMatrix();

    _y_net_matrix->set(netScale * Matrix::rotate(PI_2, Vec3(0, 0, 1)) * *cv->getModelViewMatrix());
    cv->pushModelViewMatrix(_y_net_matrix, osg::Transform::RELATIVE_RF);
    _net_geometry->accept(nv);
    cv->popModelViewMatrix();

    _axis_matrix->set(Matrix::scale(Vec3{scale, scale, scale}) * *cv->getModelViewMatrix());
    // std::cout << netCenter.x() << ' ' << netCenter.y() << ' ' << netCenter.z() << std::endl;
    cv->pushModelViewMatrix(_axis_matrix, Transform::RELATIVE_RF);
    _axis_geometry->accept(nv);
    cv->popModelViewMatrix();
  }
}
void CoordinateGeode::SetupAxisGeometry() {
  _axis_geometry = new Geometry;

  auto va = new Vec3Array();
  va->push_back({-1, 0, 0});
  va->push_back({1, 0, 0});
  va->push_back({0, -1, 0});
  va->push_back({0, 1, 0});
  va->push_back({0, 0, -1});
  va->push_back({0, 0, 1});
  _axis_geometry->setVertexArray(va);

  auto ca = new Vec3Array();
  ca->push_back({1, 0, 0});
  ca->push_back({1, 0, 0});
  ca->push_back({0, 1, 0});
  ca->push_back({0, 1, 0});
  ca->push_back({0, 0, 1});
  ca->push_back({0, 0, 1});
  _axis_geometry->setColorArray(ca, Array::BIND_PER_VERTEX);

  auto na = new Vec3Array();
  na->push_back({0, 0, 1});
  _axis_geometry->setNormalArray(na, Array::BIND_OVERALL);

  auto ps = new DrawArrays(GL_LINES, 0, va->size());
  _axis_geometry->addPrimitiveSet(ps);
  _axis_geometry->setCullingActive(false);
}
void CoordinateGeode::SetupNetGeometry() {
  static auto vss =
      "#version 330 compatibility\n"
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
  static auto fss =
      "#version 330 compatibility\n"
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

  auto state_set = new StateSet;
  state_set->setAttribute(program);

  _ng_alpha_uniform = state_set->getOrCreateUniform("u_Alpha", Uniform::FLOAT);
  _ng_num_instance_uniform = state_set->getOrCreateUniform("u_NumInstance", Uniform::INT);
  _ng_num_instance_uniform->set(20000);

  _net_geometry = new Geometry;
  _net_geometry->setStateSet(state_set);

  auto va = new Vec3Array();
  va->push_back({-1, 0, 0});
  va->push_back({1, 0, 0});
  _net_geometry->setVertexArray(va);

  auto ca = new Vec3Array();
  ca->push_back({1, 0, 0});
  ca->push_back({1, 0, 0});
  _net_geometry->setColorArray(ca, Array::BIND_PER_VERTEX);

  auto na = new Vec3Array();
  na->push_back({0, 0, 1});
  _net_geometry->setNormalArray(na, Array::BIND_OVERALL);

  _ng_draw_arrays = new DrawArrays(GL_LINES, 0, va->size(), 2000);
  _net_geometry->addPrimitiveSet(_ng_draw_arrays);

  _net_geometry->setUseDisplayList(false);
  _net_geometry->setUseVertexBufferObjects(true);
}
