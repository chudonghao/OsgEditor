//
// Created by chudonghao on 2020/3/17.
//

#include "MainWindow.h"

#include <scene/Outline.h>

#include <QDebug>
#include <QDragMoveEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <cmath>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/TextureBuffer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgFX/Outline>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgQOpenGL/osgQOpenGLWidget>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <random>
#include <regex>

#include "CoordinateGeode.h"
#include "GUIEventHandler.h"
#include "NodeDetailModel.h"
#include "NodeTreeModel.h"
#include "SetUnRefImageDataAfterApplyVisitor.h"
#include "StateSetView.h"
#include "ui_MainWindow.h"

using namespace osg;
using namespace std;

namespace MainWindowDetail {

class OutlineCullCallback : public NodeCallback {
 public:
  OutlineCullCallback(Node *self) {
    _ref_matrix = new RefMatrix;
    _self = self;
  }
  bool setNode(vector<Node *> node_path, Node *node) {
    // smallest unit is not drawable
    for (; node && node->asDrawable();) {
      bool found = false;
      bool replaced = false;
      for (auto iter = node_path.rbegin(); iter != node_path.rend(); ++iter) {
        // replace node second
        if (found) {
          node = *iter;
          replaced = true;
          break;
        }
        // find node first
        if (*iter == node) {
          found = true;
        }
      }
      // node in nodePath
      if (found) {
        // err happened
        if (!replaced) {
          node = nullptr;
        }
      }
      // node not in nodePath
      else {
        node = node_path.back();
        node_path.pop_back();
      }
    }
    if (node_path.empty() || !node) {
      goto err;
    }
    // set
    _node_path.clear();
    for (auto node : node_path) {
      _node_path.emplace_back(node);
    }
    _node = node;
    // check
    {
      // avoid outline self
      ref_ptr<Node> rpSelf = _self;
      auto iter = find_if(_node_path.begin(), _node_path.end(), [=](ref_ptr<Node> v) { return v == rpSelf; });
      if (iter != _node_path.end()) {
        goto err;
      }
    }
    {
      // correct path
      auto iter = find_if(_node_path.rbegin(), _node_path.rend(), [=](ref_ptr<Node> v) { return v == node; });
      if (iter != _node_path.rend()) {
        _node_path.erase(iter.base() - 1, _node_path.end());
      }
    }
    return true;

  err:
    _node_path.clear();
    _node = nullptr;
    return false;
  }
  void operator()(Node *node, NodeVisitor *nv) override {
    if (!_node) {
      return;
    }

    auto cv = nv->asCullVisitor();

    if (!cv) {
      traverse(node, nv);
      return;
    }
    ref_ptr<Node> rpNode = _node;
    vector<Node *> nodePath;
    nodePath.reserve(_node_path.size());
    for (auto opNode : _node_path) {
      ref_ptr<Node> rpNode = opNode;
      if (!rpNode) {
        setNode({}, nullptr);
        traverse(node, nv);
        return;
      }
      nodePath.push_back(rpNode);
    }
    auto matrix = computeLocalToWorld(nodePath);
    _ref_matrix->set(matrix * *cv->getModelViewMatrix());
    cv->pushModelViewMatrix(_ref_matrix, Transform::RELATIVE_RF);
    _node->accept(*cv);
    cv->popModelViewMatrix();
  }

 private:
  vector<observer_ptr<Node>> _node_path;
  observer_ptr<Node> _node;
  observer_ptr<Node> _self;
  ref_ptr<RefMatrix> _ref_matrix;
};

}  // namespace MainWindowDetail

using namespace MainWindowDetail;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui) {
  DisplaySettings::instance()->setMinimumNumStencilBits(1);
  QSurfaceFormat sf = QSurfaceFormat::defaultFormat();
  sf.setDepthBufferSize(24);
  sf.setStencilBufferSize(8);
  QSurfaceFormat::setDefaultFormat(sf);
  ui->setupUi(this);
  _node_tree_model = new NodeTreeModel(this);
  _node_detail_model = new NodeDetailModel(this);
  ui->node_tree_view->setModel(_node_tree_model);
  ui->node_tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->node_tree_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  ui->node_tree_view->setDragEnabled(true);
  ui->node_tree_view->setAcceptDrops(true);
  ui->node_tree_view->setDropIndicatorShown(true);
  ui->node_detail_view->setModel(_node_detail_model);
  setAcceptDrops(true);

  _gl_widget = new osgQOpenGLWidget(this);
  connect(static_cast<osgQOpenGLWidget *>(_gl_widget), &osgQOpenGLWidget::initialized, this, &MainWindow::SetupViewer);

  _gl_widget->setMinimumSize(500, 500);
  _gl_widget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  ui->viewer_container->addWidget(_gl_widget);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::SetupViewer() {
  _viewer = static_cast<osgQOpenGLWidget *>(_gl_widget)->getOsgViewer();

  // tread mode
  _viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
  _viewer->addEventHandler(new osgViewer::StatsHandler);
  _viewer->addEventHandler(new osgViewer::HelpHandler);
  _viewer->setName("Main Viewer");

  _camera = _viewer->getCamera();
  _camera->setName("Main Camera");
  _viewer->addEventHandler(new osgGA::StateSetManipulator(_camera->getStateSet()));
  _camera->setViewport(0, 0, _gl_widget->width(), _gl_widget->height());

  _camera->setClearStencil(0);
  _camera->setClearColor(Vec4(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 1.));
  _camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  _camera->setProjectionMatrixAsPerspective(60., 1., 0.1, 2000.);
  // m_camera->setProjectionMatrix(Matrix::identity());
  // m_camera->setViewMatrix(Matrix::lookAt(Vec3(0.,-5.,5.),Vec3(),Vec3(0.,0.,1.)));
  _camera->setViewMatrix(Matrix::identity());
  _camera->setComputeNearFarMode(CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

  // create root
  _root_group = new Group();
  _root_group->setName("Root Group");
  _viewer->setSceneData(_root_group);

  // create model group
  _model_group = new Group;
  _model_group->setName("Model Group");
  _root_group->addChild(_model_group);
  _model_group_state_set = _model_group->getOrCreateStateSet();
  _model_group_state_set->setMode(GL_LIGHTING, StateAttribute::OFF);
  _model_group_state_set->setAttributeAndModes(new Material);

  // auto t = new TestOutLine();
  // t->init();
  // m_rootGroup->addChild(t);

  // auto t = new TestStencil();
  // t->init();
  // m_rootGroup->addChild(t);

  // auto t = new TestCull();
  // m_rootGroup->addChild(t);

  // auto t = new TestLine;
  // m_rootGroup->addChild(t);

  // auto t = new TestBulletLine;
  // m_rootGroup->addChild(t);

  // create coordinate group
  _coord_group = new Group;
  _coord_group->setName("Coord Group");
  _root_group->addChild(_coord_group);

  _coord_group->addChild(new CoordinateGeode());

  //{
  // string modelFilename = "cessna.osgt";
  // static ref_ptr<Node> outlineModel = osgDB::readRefNodeFile(modelFilename);

  //  m_rootGroup->addChild(outlineModel);
  //
  //    // create outline effect
  //    ref_ptr<osgFX::Outline> outline = new osgFX::Outline;
  //    m_rootGroup->addChild(outline);
  //
  //    outline->setWidth(8);
  //    outline->setColor(Vec4(1,1,0,1));
  //    outline->addChild(outlineModel);
  //}

  // create outline group
  // auto outline = new osgFX::Outline;
  auto outline = new Outline;
  outline->setName("Outline Effect");
  outline->SetColor(Vec4(236.f / 255, 157.f / 255, 92.f / 255, 1));
  outline->SetWidth(4.f);
  _root_group->addChild(outline);

  _outline_group = new Group;
  _outline_group->setName("Outline Group");
  _outline_cull_callback = new OutlineCullCallback(_outline_group);
  _outline_group->setCullCallback(_outline_cull_callback);
  outline->addChild(_outline_group);

  // camera manipulator
  _camera_manipulator = new osgGA::TrackballManipulator;
  _camera_manipulator->setName("Camera Manipulator");
  _camera_manipulator->setNode(_model_group);
  _viewer->setCameraManipulator(_camera_manipulator);

  auto eventHandler = new GUIEventHandler;
  connect(eventHandler->helper(), &GUIEventHandlerQtHelper::nodeClicked, this, [=] {
    auto intersections = eventHandler->intersections();
    auto &nodePath = eventHandler->intersections().begin()->nodePath;
    auto node = eventHandler->intersections().begin()->drawable;
    HighlightNode(nodePath, node);
  });
  _viewer->addEventHandler(eventHandler);

  // create program
  // m_shaderProgram = new Program;
  // auto vs = new Shader(Shader::VERTEX);
  // vs->loadShaderSourceFromFile("model.vert");
  // auto fs = new Shader(Shader::FRAGMENT);
  // fs->loadShaderSourceFromFile("model.frag");
  // m_shaderProgram->addShader(vs);
  // m_shaderProgram->addShader(fs);
  // m_modelGroup->getOrCreateStateSet()->setAttributeAndModes(m_shaderProgram);

  // m_modelGroup->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,StateAttribute::OFF);
  _model_group->getOrCreateStateSet()->setMode(GL_BLEND, StateAttribute::ON);
  // m_modelGroup->getOrCreateStateSet()->setAttributeAndModes(new
  // BlendFunc(BlendFunc::SRC_ALPHA,BlendFunc::ONE_MINUS_SRC_ALPHA,BlendFunc::ZERO,BlendFunc::ONE));

  // start render loop
  connect(&_frame_timer, SIGNAL(timeout()), this, SLOT(update()));
  _frame_timer.setInterval(10);
  _frame_timer.start();
}

void MainWindow::paintEvent(QPaintEvent *event) {}

void MainWindow::on_open_action_triggered() {
  QString type;
  QString fn = QFileDialog::getOpenFileName(this, "Open File", _last_file_dir,
                                            "All (*.*);;"
                                            "Osg (*.osg *.osga *.osgt *.ive *.3ds);;"
                                            "Obj (*.obj);;"
                                            "Image (*.rgb *.tga *.bmp *.jpg *.dds);;",
                                            &type);
  if (fn.isEmpty()) {
    return;
  }
  QFileInfo fi(fn);
  _last_file_dir = fi.dir().path();
  if (type.startsWith("Osg") || type.startsWith("Obj") || type.startsWith("All")) {
    LoadNodeFile(fn.toStdString().c_str());
  } else {
    LoadImageFile(fn.toStdString().c_str());
  }
  _camera_manipulator->home(0.1);
}

void MainWindow::on_export_action_triggered() {
  if (!_model_node) {
    ui->statusbar->showMessage("No scene.");
  }
  QString fn = QFileDialog::getSaveFileName(this, "Save File", ".", "Osg File (*.osg)");
  if (fn.isEmpty()) {
    return;
  }
  if (osgDB::writeNodeFile(*_model_node, fn.toStdString())) {
    ui->statusbar->showMessage("File saved.", 3000);
  } else {
    ui->statusbar->showMessage("Can NOT save file.");
  }
}

void MainWindow::on_node_tree_view_clicked(const QModelIndex &index) {
  auto node = _node_tree_model->GetNode(index);
  auto nodePath = _node_tree_model->GetNodePath(index);
  decltype(nodePath) completeNodePath = {_camera, _root_group, _model_group};
  completeNodePath.insert(completeNodePath.end(), nodePath.begin(), nodePath.end());
  HighlightNode(completeNodePath, node);
  _camera_manipulator->setNode(node);
  _camera_manipulator->home(0.5);
}

void MainWindow::on_light_check_box_stateChanged(int enabled) {
  if (enabled) {
    _model_group_state_set->setMode(GL_LIGHTING, StateAttribute::ON);
  } else {
    _model_group_state_set->setMode(GL_LIGHTING, StateAttribute::OFF);
  }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  QWidget::dragEnterEvent(event);
  if (event->mimeData()->hasUrls()) {
    event->accept();
  }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) { event->accept(); }

void MainWindow::dropEvent(QDropEvent *event) {
  if (event->mimeData()->hasUrls()) {
    auto url = event->mimeData()->urls().front();
    OpenFile(url.toLocalFile().toLocal8Bit().toStdString().c_str());
  }
}

bool MainWindow::OpenFile(const char *file) {
  bool success = false;
  {
    regex re_for_node(R"(.*\.(3ds|dxf|ive|osg|osga|osgt|obj|x))");
    if (regex_match(file, re_for_node)) {
      success = LoadNodeFile(file);
      goto ret;
    }
  }
  {
    regex re_for_image(R"(.*\.(tga|png|jpg|dds|bmp))");
    // node file
    // image file
    if (regex_match(file, re_for_image)) {
      success = LoadImageFile(file);
      goto ret;
    }
  }
  statusBar()->showMessage(QString("Can not open ") + file);
ret:
  if (success) {
    _camera_manipulator->home(0.1);
  }
  return success;
}

bool MainWindow::LoadNodeFile(const char *file) {
  using namespace osgDB;
  auto node = readNodeFile(file);
  if (node) {
    SetUnRefImageDataAfterApplyVisitor nv;
    node->accept(nv);
    ChangeRootGroup(node);

    ui->statusbar->showMessage(QString::fromLocal8Bit(file) + " opened.", 3000);
    return true;
  } else {
    ui->statusbar->showMessage(QString::fromLocal8Bit(file) + " is not a node file");
  }
  return false;
}

bool MainWindow::LoadImageFile(const char *file) {
  _model_group->removeChild(_model_node);
  auto image = osgDB::readImageFile(file);
  if (image) {
    _model_group->removeChild(_model_node);
    MatrixTransform *mt = new MatrixTransform(Matrix::scale(20, 20, 20));
    mt->addChild(createGeodeForImage(image));
    _model_node = mt;
    _model_group->addChild(_model_node);
    return true;
  }
  return false;
}

void MainWindow::on_coord_check_box_stateChanged(int enabled) { _coord_group->setNodeMask(enabled ? ~0u : 0u); }

void MainWindow::ShowNodeDetail(osg::Node *node) {
  _node_detail_model->SetNode(node);
  ui->node_detail_view->expandAll();

  ShowStateSetDetail(node ? node->getStateSet() : nullptr);

  auto index = _node_tree_model->CheckNode(node);
  auto selection = ui->node_tree_view->selectionModel();
  selection->clearSelection();
  selection->select(index, QItemSelectionModel::SelectionFlag::Select);
  // ui->node_tree_view->setSelectionModel(selection);
  ui->node_tree_view->scrollTo(index);
}

void MainWindow::ShowStateSetDetail(osg::StateSet *stateSet) { ui->state_set_view->setStateSet(stateSet); }

void MainWindow::ChangeRootGroup(osg::Node *root) {
  if (!root) {
    _model_group->removeChild(_model_node);
    _model_node = nullptr;
    _node_tree_model->SetNode(root);
  }
  _model_group->removeChild(_model_node);
  _model_node = root;
  _model_group->addChild(root);
  _node_tree_model->SetNode(root);
  HighlightNode({root}, root);
}

void MainWindow::on_node_tree_view_customContextMenuRequested(const QPoint &pos) {
  return;
  QMenu menu;

  // create root node
  {
    if (!_node_tree_model->hasChildren()) {
      connect(menu.addAction("Create Root Group"), &QAction::triggered, this, &MainWindow::CreateRootGroup);
      goto ret;
    }
  }

  // special for every type
  {
    QModelIndex index = ui->node_tree_view->indexAt(pos);
    if (!index.isValid()) {
      goto ret;
    }

    auto node = _node_tree_model->GetNode(index);
    if (!node) {
      goto ret;
    }
    if (ref_ptr<Geode> geode = node->asGeode()) {
      connect(menu.addAction("Create Drawable"), &QAction::triggered, [this, geode] { OpenCreateDrawableWindow(geode); });
    };
  }

ret:
  menu.exec(ui->node_tree_view->mapToGlobal(pos));
}

void MainWindow::CreateRootGroup() {
  auto root = new Group;
  ChangeRootGroup(root);
}

void MainWindow::OpenCreateDrawableWindow(osg::Geode *geode) {}

void MainWindow::HighlightNode(const std::vector<osg::Node *> &node_path, osg::Node *node) {
  if (node_path.empty()) {
    return;
  }
  for (auto node : node_path) {
    qDebug() << "!" << node->getName().c_str();
  }
  _outline_cull_callback->setNode(node_path, node);
  ShowNodeDetail(node);
}
