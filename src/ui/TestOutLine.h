//
// Created by chudonghao on 2020/3/30.
//

#ifndef OSGEDITOR_SRC_UI_TESTOUTLINE_H
#define OSGEDITOR_SRC_UI_TESTOUTLINE_H

#include <osg/Group>

class TestOutLine : public osg::Group
{
public:
    TestOutLine();
    void init();
private:
    class CullCallback;
    osg::ref_ptr<osg::StateSet> m_globalStateSet;
    osg::ref_ptr<osg::StateSet> m_firstPass;
    osg::ref_ptr<osg::StateSet> m_secondPass;
};


#endif //OSGEDITOR_SRC_UI_TESTOUTLINE_H
