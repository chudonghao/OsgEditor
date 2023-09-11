//
// Created by chudonghao on 2020/7/8.
//

#ifndef OSGEDITOR_E0FD3C9491774CE484AF011DB4D47222_
#define OSGEDITOR_E0FD3C9491774CE484AF011DB4D47222_

#include <osg/Geode>

namespace osg {
class Geometry;
}  // namespace osg

class CoordinateGeode : public osg::Geode {
 public:
  CoordinateGeode();
  void traverse(osg::NodeVisitor &nv) override;

 private:
  void SetupAxisGeometry();
  void SetupNetGeometry();

  osg::ref_ptr<osg::Geometry> _axis_geometry;

  osg::ref_ptr<osg::Geometry> _net_geometry;
  osg::ref_ptr<osg::DrawArrays> _ng_draw_arrays;
  osg::ref_ptr<osg::Uniform> _ng_alpha_uniform;
  osg::ref_ptr<osg::Uniform> _ng_num_instance_uniform;

  osg::ref_ptr<osg::RefMatrix> _axis_matrix;
  osg::ref_ptr<osg::RefMatrix> _x_net_matrix;
  osg::ref_ptr<osg::RefMatrix> _y_net_matrix;
};

#endif  // OSGEDITOR_E0FD3C9491774CE484AF011DB4D47222_
