//
// Created by chudonghao on 2020/3/17.
//

#ifndef OSGEDITOR_SRC_UI_MAINWINDOW_H
#define OSGEDITOR_SRC_UI_MAINWINDOW_H

#include <QMainWindow>
#include <QOpenGLWidget>
#include <QTimer>
#include <osg/Shader>
#include <osg/Version>
#include <osg/ref_ptr>

namespace osg {
class Camera;
class Node;
class StateSet;
class Group;
class Program;
class Geode;
}  // namespace osg

namespace osgGA {
class CameraManipulator;
}  // namespace osgGA

namespace osgViewer {
class Viewer;
}  // namespace osgViewer

namespace Ui {
class MainWindow;
}

class NodeTreeModel;

namespace MainWindowDetail {
class OutlineCullCallback;
}  // namespace MainWindowDetail
class MainWindow : public QMainWindow {
  Q_OBJECT;

 public:
  explicit MainWindow(QWidget *parent = nullptr);

  ~MainWindow() override;

 protected:
  void paintEvent(QPaintEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  void dropEvent(QDropEvent *event) override;

 private:
  Q_SLOT void on_open_action_triggered();
  Q_SLOT void on_export_action_triggered();
  Q_SLOT void on_node_tree_view_clicked(const QModelIndex &index);
  Q_SLOT void on_light_check_box_stateChanged(int);
  Q_SLOT void on_coord_check_box_stateChanged(int);
  Q_SLOT void on_node_tree_view_customContextMenuRequested(const QPoint &pos);
  Q_SLOT void ShowNodeDetail(osg::Node *node);
  Q_SLOT void ShowStateSetDetail(osg::StateSet *stateSet);
  Q_SLOT void CreateRootGroup();
  void HighlightNode(const std::vector<osg::Node *> &, osg::Node *node);
  void ChangeRootGroup(osg::Node *root);
  void OpenCreateDrawableWindow(osg::Geode *geode);

 private:
  using Ui = ::Ui::MainWindow;
  Ui *ui;
  QOpenGLWidget *_gl_widget;
  osg::ref_ptr<osgViewer::Viewer> _viewer;
  osg::ref_ptr<osg::Camera> _camera;
  osg::ref_ptr<osg::Group> _root_group;
  osg::ref_ptr<osg::Group> _model_group;
  osg::ref_ptr<osg::Group> _outline_group;
  osg::ref_ptr<osg::Node> _model_node;
  osg::ref_ptr<osg::Group> _coord_group;
  osg::ref_ptr<osg::StateSet> _model_group_state_set;
  osg::ref_ptr<osg::Program> _shader_program;
  osg::ref_ptr<osgGA::CameraManipulator> _camera_manipulator;
  osg::ref_ptr<MainWindowDetail::OutlineCullCallback> _outline_cull_callback;
  QTimer _frame_timer;
  NodeTreeModel *_node_tree_model;
  QString _last_file_dir{"."};
  void SetupViewer();
  bool OpenFile(const char *file);
  bool LoadNodeFile(const char *file);
  bool LoadImageFile(const char *file);
};

#endif  // OSGEDITOR_SRC_UI_MAINWINDOW_H
