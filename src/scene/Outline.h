//
// Created by chudonghao on 2020/7/7.
//

#ifndef OSGEDITOR_E6681B7E64614347A1EE6F56EC3305DC_
#define OSGEDITOR_E6681B7E64614347A1EE6F56EC3305DC_

#include <osg/Group>

namespace osg {
class LineWidth;
class Material;
}  // namespace osg

class Outline : public osg::Group {
 public:
  Outline();
  const osg::Vec4 &GetColor() const;
  void SetColor(const osg::Vec4 &color);
  float GetWidth() const;
  void SetWidth(float width);

 protected:
  void traverse(osg::NodeVisitor &nv) override;

 private:
  osg::Vec4 _color{1, 1, 1, 1};
  float _width{1};

  osg::ref_ptr<osg::LineWidth> _line_width;
  osg::ref_ptr<osg::Material> _material;
  osg::ref_ptr<osg::StateSet> _first_pass;
  osg::ref_ptr<osg::StateSet> _second_pass;
};

#endif  // OSGEDITOR_E6681B7E64614347A1EE6F56EC3305DC_
