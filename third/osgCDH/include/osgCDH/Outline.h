//
// Created by chudonghao on 2020/7/7.
//

#ifndef OSGEDITOR_E6681B7E64614347A1EE6F56EC3305DC_
#define OSGEDITOR_E6681B7E64614347A1EE6F56EC3305DC_

#include <osg/Group>

namespace osg
{
class LineWidth;
class Material;
} // namespace osg

namespace osgCDH
{

class Outline : public osg::Group
{
public:
    Outline();
    const osg::Vec4 &getColor() const;
    void setColor(const osg::Vec4 &color);
    float getWidth() const;
    void setWidth(float width);
protected:
    void traverse(osg::NodeVisitor &nv) override;
private:

    osg::Vec4 m_color{1, 1, 1, 1};
    float m_width{1};

    osg::ref_ptr<osg::LineWidth> m_lineWidth;
    osg::ref_ptr<osg::Material> m_material;
    osg::ref_ptr<osg::StateSet> m_firstPass;
    osg::ref_ptr<osg::StateSet> m_secondPass;
};

} // namespace osgCDH

#endif //OSGEDITOR_E6681B7E64614347A1EE6F56EC3305DC_
