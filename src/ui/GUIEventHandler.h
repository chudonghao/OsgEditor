//
// Created by chudonghao on 2020/4/3.
//

#ifndef OSGEDITOR_SRC_UI_GUIEVENTHANDLER_H
#define OSGEDITOR_SRC_UI_GUIEVENTHANDLER_H

#include <QObject>
#include <osgGA/GUIEventHandler>
#include <osgUtil/LineSegmentIntersector>

class GUIEventHandlerQtHelper : public QObject {
  Q_OBJECT;

 public:
  Q_SIGNAL void nodeClicked();
};

class GUIEventHandler : public osgGA::GUIEventHandler {
 public:
  using Intersections = osgUtil::LineSegmentIntersector::Intersections;

  bool handle(const osgGA::GUIEventAdapter &adapter, osgGA::GUIActionAdapter &actionAdapter) override;

  const Intersections &intersections() const { return _intersections; }

  GUIEventHandlerQtHelper *helper() { return &_helper; }

 private:
  Intersections _intersections;
  GUIEventHandlerQtHelper _helper;
  bool _drag{false};
};

#endif  // OSGEDITOR_SRC_UI_GUIEVENTHANDLER_H
