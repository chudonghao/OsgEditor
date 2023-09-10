//
// Created by chudonghao on 2020/7/8.
//

#ifndef OSGEDITOR_E0FD3C9491774CE484AF011DB4D47222_
#define OSGEDITOR_E0FD3C9491774CE484AF011DB4D47222_

#include <osg/Geode>

namespace osg
{
class Geometry;
} // namespace osg

class CoordinateGeode : public osg::Geode{
public:
    CoordinateGeode();
    void traverse(osg::NodeVisitor &nv) override;
private:
    void setupAxisGeometry();
    void setupNetGeometry();

    osg::ref_ptr<osg::Geometry> m_axisGeometry;

    osg::ref_ptr<osg::Geometry> m_NetGeometry;
    osg::ref_ptr<osg::DrawArrays> m_NgDrawArrays;
    osg::ref_ptr<osg::Uniform> m_NgAlphaUniform;
    osg::ref_ptr<osg::Uniform> m_NgNumInstanceUniform;

    osg::ref_ptr<osg::RefMatrix> m_axisMatrix;
    osg::ref_ptr<osg::RefMatrix> m_xNetMatrix;
    osg::ref_ptr<osg::RefMatrix> m_yNetMatrix;
};


#endif //OSGEDITOR_E0FD3C9491774CE484AF011DB4D47222_
