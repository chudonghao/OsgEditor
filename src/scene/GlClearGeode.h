//
// Created by chudonghao on 2020/7/3.
//

#ifndef OSGEDITOR_GLCLEARGEODE_H
#define OSGEDITOR_GLCLEARGEODE_H

#include <osg/Geode>

class GlClearGeode : public osg::Geode
{
public:
    explicit GlClearGeode(GLbitfield mask);

    void setClearMask(GLbitfield mask);

    GLbitfield getClearMask() const;

private:
    GLbitfield m_clearMask{};
};


#endif //OSGEDITOR_GLCLEARGEODE_H
