//
// Created by chudonghao on 2020/7/3.
//

#include "GlClearGeode.h"

#include <osg/Drawable>
#include <osg/State>

using namespace osg;


namespace
{

class DrawCallback : public osg::Drawable::DrawCallback
{
public:
    DrawCallback(GLbitfield &clearMask) : m_clearMask(clearMask) {}

    void drawImplementation(osg::RenderInfo &info, const osg::Drawable *drawable) const override
    {
        //auto &state = *info.getState();
        glClear(m_clearMask);
    }

private:
    GLbitfield &m_clearMask;
};

} // namespace

GlClearGeode::GlClearGeode(GLbitfield mask)
{
    m_clearMask = mask;
    auto drawable = new Drawable;
    drawable->setUseDisplayList(false);
    drawable->setUseVertexArrayObject(false);
    drawable->setUseVertexBufferObjects(false);
    drawable->setDrawCallback(new DrawCallback(m_clearMask));
    Geode::addDrawable(drawable);
}

void GlClearGeode::setClearMask(GLbitfield mask)
{
    m_clearMask = mask;
}

GLbitfield GlClearGeode::getClearMask() const
{
    return m_clearMask;
}
