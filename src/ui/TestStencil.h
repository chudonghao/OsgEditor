//
// Created by chudonghao on 2020/4/26.
//

#ifndef OSGEDITOR_SRC_UI_TESTSTENCIL_H
#define OSGEDITOR_SRC_UI_TESTSTENCIL_H

#include <osg/Group>

class TestStencil : public osg::Group
{
public:
    TestStencil();
    void init();
private:
    class CullCallback;
    osg::ref_ptr<osg::StateSet> m_globalStateSet;
    osg::ref_ptr<osg::StateSet> m_firstPass;
    osg::ref_ptr<osg::StateSet> m_secondPass;
};

#endif //OSGEDITOR_SRC_UI_TESTSTENCIL_H
