//
// Created by chudonghao on 2020/4/3.
//

#include "GUIEventHandler.h"

#include <iostream>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <osgViewer/Viewer>

using namespace std;

bool GUIEventHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) {
  auto view = dynamic_cast<osgViewer::View *>(&aa);
  if (!view) {
    return false;
  }

  switch (ea.getEventType()) {
    case osgGA::GUIEventAdapter::PUSH: {
      _drag = false;

      break;
    }
    case osgGA::GUIEventAdapter::DRAG: {
      _drag = true;
      break;
    }
    case osgGA::GUIEventAdapter::RELEASE: {
      if (!_drag) {
        Intersections ins;
        view->computeIntersections(ea, ins);

        if (ins.empty()) {
          cout << "empty" << endl;
        } else {
          auto &in = *ins.begin();
          cout << in.drawable->getName() << endl;
          _intersections = ins;
          emit _helper.nodeClicked();
        }
      }
      break;
    }
  }
  return false;
}
