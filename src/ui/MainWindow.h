//
// Created by chudonghao on 2020/3/17.
//

#ifndef OSGEDITOR_SRC_UI_MAINWINDOW_H
#define OSGEDITOR_SRC_UI_MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QOpenGLWidget>
#include <osg/Version>
#include <osg/ref_ptr>
#include <osg/Shader>

namespace osg
{
class Camera;
class Node;
class StateSet;
class Group;
class Program;
class Geode;
} // namespace osg

namespace osgGA
{
class CameraManipulator;
} // namespace osgGA

namespace osgViewer
{
class Viewer;
} // namespace osgViewer

namespace Ui
{
class MainWindow;
}

class NodeTreeModel;

namespace MainWindowDetail {
class OutlineCullCallback;
}// namespace MainWindowDetail
class MainWindow : public QMainWindow
{
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
    Q_SLOT void openOpenFileDialog();
    Q_SLOT void openSaveFileDialog();
    Q_SLOT void nodeTreeViewItemClicked(const QModelIndex &index);
    Q_SLOT void setLightEnabled(bool);
    Q_SLOT void setCoordEnabled(bool);
    Q_SLOT void setProgramEnabled(bool);
    Q_SLOT void openShaderWindow();
    Q_SLOT void showNodeDetail(osg::Node *node);
    Q_SLOT void showStateSetDetail(osg::StateSet *stateSet);
    Q_SLOT void showTreeViewContexMenu(const QPoint &pos);
    Q_SLOT void createRootGroup();
    void highlightNode(const std::vector<osg::Node*> &, osg::Node *node);
    void changeRootGroup(osg::Node *root);
    void openCreateDrawableWindow(osg::Geode *geode);
private:
    using Ui = ::Ui::MainWindow;
    Ui *ui;
#if OSG_VERSION_LESS_THAN(3,6,0)
    QGLWidget *m_glWidget;
#else
    QOpenGLWidget *m_glWidget;
#endif
    osg::ref_ptr<osgViewer::Viewer> m_viewer;
    osg::ref_ptr<osg::Camera> m_camera;
    osg::ref_ptr<osg::Group> m_rootGroup;
    osg::ref_ptr<osg::Group> m_modelGroup;
    osg::ref_ptr<osg::Group> m_outlineGroup;
    osg::ref_ptr<osg::Node> m_modelNode;
    osg::ref_ptr<osg::Group> m_coordGroup;
    osg::ref_ptr<osg::StateSet> m_modelGroupStateSet;
    osg::ref_ptr<osg::Program> m_shaderProgram;
    osg::ref_ptr<osgGA::CameraManipulator> m_cameraManipulator;
    osg::ref_ptr<MainWindowDetail::OutlineCullCallback> m_outlineCullCallback;
    QTimer m_frameTimer;
    NodeTreeModel *m_nodeTreeModel;
    QString m_lastFileDir{"."};
    void setupViewer();
    bool openFile(const char*file);
    bool loadNodeFile(const char*file);
    bool loadImageFile(const char*file);
};

#endif //OSGEDITOR_SRC_UI_MAINWINDOW_H
