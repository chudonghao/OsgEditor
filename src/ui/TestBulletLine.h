//
// Created by chudonghao on 2020/4/13.
//

#ifndef OSGEDITOR_SRC_UI_TESTBULLETLINE_H
#define OSGEDITOR_SRC_UI_TESTBULLETLINE_H

#include <osg/Group>
class TestBulletLine : public osg::Group
{
public:
    TestBulletLine();
    class UpdateCallback;
private:
    float m_startTime{0.f};
    float m_v{5.f};
    float m_length{20.f};
};


#endif //OSGEDITOR_SRC_UI_TESTBULLETLINE_H
