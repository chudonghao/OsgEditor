//
// Created by chudonghao on 2020/3/17.
//

#include "MainWindow.h"

#include <cmath>
#include <random>
#include <regex>

#include <QDebug>
#include <QDragMoveEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>

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

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#if OSG_VERSION_LESS_THAN(3, 6, 0)
#include <osgQt/GraphicsWindowQt>
#else
#include <osgQOpenGL/osgQOpenGLWidget>
#endif

#include <osgCDH/Outline.h>

#include "CoordinateGeode.h"

#include "Visitor.h"
#include "TestOutLine.h"
#include "GUIEventHandler.h"

#include "NodeTreeModel.h"
#include "ShaderWindow.h"
#include "StateSetView.h"
#include "NodeView.h"
#include "ui_MainWindow.h"

#include "TestLine.h"
#include "TestCull.h"
#include "TestBulletLine.h"
#include "TestStencil.h"

#include "SetUnRefImageDataAfterApplyVisitor.h"

using namespace osg;
using namespace std;

namespace MainWindowDetail
{

class OutlineCullCallback : public NodeCallback
{
public:
    OutlineCullCallback(Node *self)
    {
        m_refMatrix = new RefMatrix;
        m_self = self;
    }
    bool setNode(vector<Node *> nodePath, Node *node)
    {
        // smallest unit is not drawable
        for (;node && node->asDrawable();)
        {
            bool found = false;
            bool replaced = false;
            for (auto iter = nodePath.rbegin(); iter != nodePath.rend(); ++iter)
            {
                // replace node second
                if (found)
                {
                    node = *iter;
                    replaced = true;
                    break;
                }
                // find node first
                if (*iter == node)
                {
                    found = true;
                }
            }
            // node in nodePath
            if (found)
            {
                // err happened
                if (!replaced) { node = nullptr; }
            }
            // node not in nodePath
            else
            {
                node = nodePath.back();
                nodePath.pop_back();
            }
        }
        if (nodePath.empty() || !node)
        {
            goto err;
        }
        // set
        m_nodePath.clear();
        for (auto node : nodePath)
        {
            m_nodePath.emplace_back(node);
        }
        m_node = node;
        // check
        {
            // avoid outline self
            ref_ptr<Node> rpSelf = m_self;
            auto iter = find_if(m_nodePath.begin(), m_nodePath.end(), [=](ref_ptr<Node> v)
            {
                return v == rpSelf;
            });
            if (iter != m_nodePath.end())
            {
                goto err;
            }
        }
        {
            // correct path
            auto iter = find_if(m_nodePath.rbegin(), m_nodePath.rend(), [=](ref_ptr<Node> v)
            {
                return v == node;
            });
            if (iter != m_nodePath.rend())
            {
                m_nodePath.erase(iter.base() - 1, m_nodePath.end());
            }
        }
        return true;

        err:
        m_nodePath.clear();
        m_node = nullptr;
        return false;
    }
    void operator()(Node *node, NodeVisitor *nv) override
    {
        if (!m_node)
        {
            return;
        }
#if OSG_VERSION_LESS_THAN(3, 6, 0)
        auto cv = dynamic_cast<osgUtil::CullVisitor *>(nv);
#else
        auto cv = nv->asCullVisitor();
#endif
        if (!cv)
        {
            traverse(node, nv);
            return;
        }
        ref_ptr<Node> rpNode = m_node;
        vector<Node *> nodePath;
        nodePath.reserve(m_nodePath.size());
        for (auto opNode : m_nodePath)
        {
            ref_ptr<Node> rpNode = opNode;
            if (!rpNode)
            {
                setNode({}, nullptr);
                traverse(node, nv);
                return;
            }
            nodePath.push_back(rpNode);
        }
        auto matrix = computeLocalToWorld(nodePath);
        m_refMatrix->set(matrix * *cv->getModelViewMatrix());
        cv->pushModelViewMatrix(m_refMatrix, Transform::RELATIVE_RF);
        m_node->accept(*cv);
        cv->popModelViewMatrix();
    }
private:
    vector<observer_ptr<Node>> m_nodePath;
    observer_ptr<Node> m_node;
    observer_ptr<Node> m_self;
    ref_ptr<RefMatrix> m_refMatrix;
};

}

using namespace MainWindowDetail;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui)
{
    DisplaySettings::instance()->setMinimumNumStencilBits(1);
    QSurfaceFormat sf = QSurfaceFormat::defaultFormat();
    sf.setDepthBufferSize(24);
    sf.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(sf);
    ui->setupUi(this);
    m_nodeTreeModel = new NodeTreeModel(this);
    ui->node_tree_view->setModel(m_nodeTreeModel);
    ui->node_tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->node_tree_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->node_tree_view->setDragEnabled(true);
    ui->node_tree_view->setAcceptDrops(true);
    ui->node_tree_view->setDropIndicatorShown(true);
    connect(ui->node_tree_view, &QTreeView::clicked, this, &MainWindow::nodeTreeViewItemClicked);
    connect(ui->node_tree_view, &QTreeView::customContextMenuRequested, this, &MainWindow::showTreeViewContexMenu);
    connect(ui->open_action, &QAction::triggered, this,
            static_cast<void (MainWindow::*)()>(&MainWindow::openOpenFileDialog));
    connect(ui->export_action, &QAction::triggered, this, &MainWindow::openSaveFileDialog);
    connect(ui->light_check_box, &QCheckBox::stateChanged, this, &MainWindow::setLightEnabled);
    connect(ui->coord_check_box, &QCheckBox::stateChanged, this, &MainWindow::setCoordEnabled);
    connect(ui->chose_shader_btn, &QPushButton::clicked, this, &MainWindow::openShaderWindow);
    setAcceptDrops(true);

#if OSG_VERSION_LESS_THAN(3, 6, 0)
    m_glWidget = new osgQt::GLWidget(this);
    new osgQt::GraphicsWindowQt(static_cast<osgQt::GLWidget *>(m_glWidget));
#else
    m_glWidget = new osgQOpenGLWidget(this);
    connect(static_cast<osgQOpenGLWidget *>(m_glWidget), &osgQOpenGLWidget::initialized, this, &MainWindow::setupViewer);
#endif
    m_glWidget->setMinimumSize(500, 500);
    m_glWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    ui->viewer_container->addWidget(m_glWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupViewer()
{
    //auto newList = string();
    //auto list = osgDB::Registry::instance()->getLibraryFilePathList();
    //for (auto &path : list)
    //{
    //    //qDebug() << path.c_str();
    //    newList += path;
    //    newList += ":";
    //}
    //newList += ;
    //osgDB::Registry::instance()->getLibraryFilePathList().push_back("/home/chudonghao/gits/HgOsgViewer/osgPlugins-3.6.4");

#if OSG_VERSION_LESS_THAN(3, 6, 0)
    m_viewer = new osgViewer::Viewer;
#else
    m_viewer = static_cast<osgQOpenGLWidget *>(m_glWidget)->getOsgViewer();
#endif

    // tread mode
    m_viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    m_viewer->addEventHandler(new osgViewer::StatsHandler);
    m_viewer->addEventHandler(new osgViewer::HelpHandler);
    m_viewer->setName("Main Viewer");

    m_camera = m_viewer->getCamera();
    m_camera->setName("Main Camera");
    m_viewer->addEventHandler(new osgGA::StateSetManipulator(m_camera->getStateSet()));
    m_camera->setViewport(0, 0, m_glWidget->width(), m_glWidget->height());
#if OSG_VERSION_LESS_THAN(3, 6, 0)
    auto gc = static_cast<osgQt::GLWidget *>(m_glWidget)->getGraphicsWindow();
    m_camera->setGraphicsContext(gc);
#endif
    m_camera->setClearStencil(0);
    m_camera->setClearColor(Vec4(100.f/255.f, 100.f/255.f, 100.f/255.f, 1.));
    m_camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    m_camera->setProjectionMatrixAsPerspective(60., 1., 0.1, 2000.);
    //m_camera->setProjectionMatrix(Matrix::identity());
    //m_camera->setViewMatrix(Matrix::lookAt(Vec3(0.,-5.,5.),Vec3(),Vec3(0.,0.,1.)));
    m_camera->setViewMatrix(Matrix::identity());
    m_camera->setComputeNearFarMode(CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

    // create root
    m_rootGroup = new Group();
    m_rootGroup->setName("Root Group");
    m_viewer->setSceneData(m_rootGroup);

    // create model group
    m_modelGroup = new Group;
    m_modelGroup->setName("Model Group");
    m_rootGroup->addChild(m_modelGroup);
    m_modelGroupStateSet = m_modelGroup->getOrCreateStateSet();
    m_modelGroupStateSet->setMode(GL_LIGHTING, StateAttribute::OFF);
    m_modelGroupStateSet->setAttributeAndModes(new Material);

    //auto t = new TestOutLine();
    //t->init();
    //m_rootGroup->addChild(t);

    //auto t = new TestStencil();
    //t->init();
    //m_rootGroup->addChild(t);

    //auto t = new TestCull();
    //m_rootGroup->addChild(t);

    //auto t = new TestLine;
    //m_rootGroup->addChild(t);

    //auto t = new TestBulletLine;
    //m_rootGroup->addChild(t);



    // create coordinate group
    m_coordGroup = new Group;
    m_coordGroup->setName("Coord Group");
    m_rootGroup->addChild(m_coordGroup);

    m_coordGroup->addChild(new CoordinateGeode());

    //{
    //string modelFilename = "cessna.osgt";
    //static ref_ptr<Node> outlineModel = osgDB::readRefNodeFile(modelFilename);

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
    //auto outline = new osgFX::Outline;
    auto outline = new osgCDH::Outline;
    outline->setName("Outline Effect");
    outline->setColor(Vec4(236.f/255, 157.f/255, 92.f/255, 1));
    outline->setWidth(4.f);
    m_rootGroup->addChild(outline);

    m_outlineGroup = new Group;
    m_outlineGroup->setName("Outline Group");
    m_outlineCullCallback = new OutlineCullCallback(m_outlineGroup);
    m_outlineGroup->setCullCallback(m_outlineCullCallback);
    outline->addChild(m_outlineGroup);

    // camera manipulator
    m_cameraManipulator = new osgGA::TrackballManipulator;
    m_cameraManipulator->setName("Camera Manipulator");
    m_cameraManipulator->setNode(m_modelGroup);
    m_viewer->setCameraManipulator(m_cameraManipulator);

    auto eventHandler = new GUIEventHandler;
    connect(eventHandler->helper(), &GUIEventHandlerQtHelper::nodeClicked, this, [=]
    {
        auto intersections = eventHandler->intersections();
        auto &nodePath = eventHandler->intersections().begin()->nodePath;
        auto node = eventHandler->intersections().begin()->drawable;
        highlightNode(nodePath, node);
    });
    m_viewer->addEventHandler(eventHandler);

    // create program
    //m_shaderProgram = new Program;
    //auto vs = new Shader(Shader::VERTEX);
    //vs->loadShaderSourceFromFile("model.vert");
    //auto fs = new Shader(Shader::FRAGMENT);
    //fs->loadShaderSourceFromFile("model.frag");
    //m_shaderProgram->addShader(vs);
    //m_shaderProgram->addShader(fs);
    //m_modelGroup->getOrCreateStateSet()->setAttributeAndModes(m_shaderProgram);

    //m_modelGroup->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,StateAttribute::OFF);
    m_modelGroup->getOrCreateStateSet()->setMode(GL_BLEND, StateAttribute::ON);
    //m_modelGroup->getOrCreateStateSet()->setAttributeAndModes(new BlendFunc(BlendFunc::SRC_ALPHA,BlendFunc::ONE_MINUS_SRC_ALPHA,BlendFunc::ZERO,BlendFunc::ONE));

    // start render loop
    connect(&m_frameTimer, SIGNAL(timeout()), this, SLOT(update()));
    m_frameTimer.setInterval(10);
    m_frameTimer.start();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
#if OSG_VERSION_LESS_THAN(3, 6, 0)
    if (m_viewer) { m_viewer->frame(); }
    else { setupViewer(); }
#endif
}

void MainWindow::openOpenFileDialog()
{
    QString type;
    QString fn = QFileDialog::getOpenFileName(this, "Open File", m_lastFileDir,
                                              "All (*.*);;"
                                              "Osg (*.osg *.osga *.osgt *.ive *.3ds);;"
                                              "Obj (*.obj);;"
                                              "Image (*.rgb *.tga *.bmp *.jpg *.dds);;",
                                              &type);
    if (fn.isEmpty())
    {
        return;
    }
    QFileInfo fi(fn);
    m_lastFileDir = fi.dir().path();
    if (type.startsWith("Osg") || type.startsWith("Obj") || type.startsWith("All"))
    {
        loadNodeFile(fn.toStdString().c_str());
    }
    else
    {
        loadImageFile(fn.toStdString().c_str());
    }
    m_cameraManipulator->home(0.1);
}

void MainWindow::openSaveFileDialog()
{
    if (!m_modelNode)
    {
        ui->statusbar->showMessage("No scene.");
    }
    QString fn = QFileDialog::getSaveFileName(this, "Save File", ".", "Osg File (*.osg)");
    if (fn.isEmpty())
    {
        return;
    }
    if (osgDB::writeNodeFile(*m_modelNode, fn.toStdString())) { ui->statusbar->showMessage("File saved.", 3000); }
    else { ui->statusbar->showMessage("Can NOT save file."); }
}

void MainWindow::nodeTreeViewItemClicked(const QModelIndex &index)
{
    auto node = m_nodeTreeModel->getNode(index);
    auto nodePath = m_nodeTreeModel->getNodePath(index);
    decltype(nodePath) completeNodePath = {m_camera, m_rootGroup, m_modelGroup};
    completeNodePath.insert(completeNodePath.end(), nodePath.begin(), nodePath.end());
    highlightNode(completeNodePath, node);
    m_cameraManipulator->setNode(node);
    m_cameraManipulator->home(0.5);
}

void MainWindow::setLightEnabled(bool enabled)
{
    if (enabled)
    {
        m_modelGroupStateSet->setMode(GL_LIGHTING, StateAttribute::ON);
    }
    else
    {
        m_modelGroupStateSet->setMode(GL_LIGHTING, StateAttribute::OFF);
    }
}

void MainWindow::setProgramEnabled(bool enabled)
{
    if (enabled)
    {

    }
}

void MainWindow::openShaderWindow()
{
    CShaderWindow *s = new CShaderWindow(this);
    s->setAttribute(Qt::WA_ShowModal);
    s->setAttribute(Qt::WA_DeleteOnClose);
    s->show();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    QWidget::dragEnterEvent(event);
    if (event->mimeData()->hasUrls())
    {
        event->accept();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        auto url = event->mimeData()->urls().front();
        openFile(url.toLocalFile().toLocal8Bit().toStdString().c_str());
    }
}

bool MainWindow::openFile(const char *file)
{
    bool success = false;
    {
        regex re_for_node(R"(.*\.(3ds|dxf|ive|osg|osga|osgt|obj|x))");
        if (regex_match(file, re_for_node))
        {
            success = loadNodeFile(file);
            goto ret;
        }

    }
    {
        regex re_for_image(R"(.*\.(tga|png|jpg|dds|bmp))");
        // node file
        // image file
        if (regex_match(file, re_for_image))
        {
            success = loadImageFile(file);
            goto ret;
        }
    }
    statusBar()->showMessage(QString("Can not open ") + file);
    ret:
    if (success) { m_cameraManipulator->home(0.1); }
    return success;
}

bool MainWindow::loadNodeFile(const char *file)
{
    using namespace osgDB;
    auto node = readNodeFile(file);
    if (node)
    {
        SetUnRefImageDataAfterApplyVisitor nv;
        node->accept(nv);
        changeRootGroup(node);
        //Visitor v;
        //node->accept(v);
        //
        //Image *img = new Image();
        //img->allocateImage(10000, 1, 1, GL_RGBA, GL_FLOAT);
        //
        //random_device rd;
        //default_random_engine re(rd());
        //uniform_real_distribution<float> d(0.f,3.14f);
        //
        //for (int i = 0; i < 10000; ++i)
        //{
        //    Vec4f *pos = reinterpret_cast<Vec4f *>(img->data(i));
        //    auto r = d(re);
        //    *pos = Vec4f(i*sin(r)*5, i*cos(r)*5, 0, 1);
        //}
        //
        //TextureBuffer *tex = new TextureBuffer(img);
        //tex->setInternalFormat(GL_RGBA32F_ARB);
        //tex->setSourceFormat(GL_RGBA);
        //tex->setResizeNonPowerOfTwoHint(false);
        //node->getOrCreateStateSet()->setTextureAttribute(3, tex);
        //node->getOrCreateStateSet()->getOrCreateUniform("u_posBuffer",Uniform::INT)->set(3);
        //node->getOrCreateStateSet()->getOrCreateUniform("u_DiffuseMap",Uniform::INT)->set(0);

        //ui->node_tree_view->reset();
        ui->statusbar->showMessage(QString::fromLocal8Bit(file) + " opened.", 3000);
        return true;
    }
    else
    {
        ui->statusbar->showMessage(QString::fromLocal8Bit(file) + " is not a node file");
    }
    return false;
}

bool MainWindow::loadImageFile(const char *file)
{
    m_modelGroup->removeChild(m_modelNode);
    auto image = osgDB::readImageFile(file);
    if (image)
    {
        m_modelGroup->removeChild(m_modelNode);
        MatrixTransform *mt = new MatrixTransform(Matrix::scale(20, 20, 20));
        mt->addChild(createGeodeForImage(image));
        m_modelNode = mt;
        m_modelGroup->addChild(m_modelNode);
        return true;
    }
    return false;
}

void MainWindow::setCoordEnabled(bool enabled)
{
    m_coordGroup->setNodeMask(enabled ? ~0u : 0u);
}

void MainWindow::showNodeDetail(Node *node)
{
    ui->node_view->setNode(node);
    showStateSetDetail(node ? node->getStateSet() : nullptr);

    auto index = m_nodeTreeModel->checkNode(node);
    auto selection = ui->node_tree_view->selectionModel();
    selection->clearSelection();
    selection->select(index, QItemSelectionModel::SelectionFlag::Select);
    //ui->node_tree_view->setSelectionModel(selection);
    ui->node_tree_view->scrollTo(index);
}

void MainWindow::showStateSetDetail(StateSet *stateSet)
{
    ui->state_set_view->setStateSet(stateSet);
}

void MainWindow::changeRootGroup(Node *root)
{
    if (!root)
    {
        m_modelGroup->removeChild(m_modelNode);
        m_modelNode = nullptr;
        m_nodeTreeModel->setNode(root);
    }
    m_modelGroup->removeChild(m_modelNode);
    m_modelNode = root;
    m_modelGroup->addChild(root);
    m_nodeTreeModel->setNode(root);
    highlightNode({root}, root);
}

void MainWindow::showTreeViewContexMenu(const QPoint &pos)
{
    return;
    QMenu menu;

    // create root node
    {
        if (!m_nodeTreeModel->hasChildren())
        {
            connect(menu.addAction("Create Root Group"), &QAction::triggered, this, &MainWindow::createRootGroup);
            goto ret;
        }
    }

    // special for every type
    {
        QModelIndex index = ui->node_tree_view->indexAt(pos);
        if (!index.isValid())
        {
            goto ret;
        }

        auto node = m_nodeTreeModel->getNode(index);
        if (!node)
        {
            goto ret;
        }
        if (ref_ptr<Geode> geode = node->asGeode())
        {
            connect(menu.addAction("Create Drawable"),&QAction::triggered, [this, geode] { openCreateDrawableWindow(geode); });
        };
    }


    ret:
    menu.exec(ui->node_tree_view->mapToGlobal(pos));
}

void MainWindow::createRootGroup()
{
    auto root = new Group;
    changeRootGroup(root);
}

void MainWindow::openCreateDrawableWindow(Geode *geode)
{

}
void MainWindow::highlightNode(const vector<Node *> &nodePath, Node *node)
{
    if (nodePath.empty()) { return; }
    for (auto node : nodePath)
    {
        qDebug() << "!" << node->getName().c_str();
    }
    m_outlineCullCallback->setNode(nodePath, node);
    showNodeDetail(node);
}
