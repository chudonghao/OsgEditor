//
// Created by chudonghao on 2020/4/3.
//

#ifndef OSGEDITOR_SRC_UI_GUIEVENTHANDLER_H
#define OSGEDITOR_SRC_UI_GUIEVENTHANDLER_H

#include <osgGA/GUIEventHandler>
#include <osgUtil/LineSegmentIntersector>
#include <QObject>

class GUIEventHandlerQtHelper : public QObject
{
Q_OBJECT;
public:
    Q_SIGNAL void nodeClicked();
};

class GUIEventHandler : public osgGA::GUIEventHandler
{
public:
    using Intersections = osgUtil::LineSegmentIntersector::Intersections;

    bool handle(const osgGA::GUIEventAdapter &adapter, osgGA::GUIActionAdapter &actionAdapter) override;

    const Intersections &intersections() const { return m_intersections; }

    GUIEventHandlerQtHelper *helper() { return &m_helper; }

private:
    Intersections m_intersections;
    GUIEventHandlerQtHelper m_helper;
    bool m_drag{false};
};

#endif //OSGEDITOR_SRC_UI_GUIEVENTHANDLER_H
