//
// Created by chudonghao on 2020/3/19.
//

#include "StateSetView.h"
#include <osg/StateSet>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/Material>
#include <QDebug>
#include "TextureListModel.h"
#include "OsgDataEdit.h"
#include "ui_StateSetView.h"

using namespace osg;

Q_DECLARE_METATYPE(Texture::WrapParameter);
Q_DECLARE_METATYPE(Texture::FilterParameter);

namespace
{
int getWrapIndex(Texture::WrapMode m)
{
    switch (m)
    {
        case Texture::CLAMP:
            return 1;
        case Texture::CLAMP_TO_EDGE:
            return 2;
        case Texture::CLAMP_TO_BORDER:
            return 3;
        case Texture::REPEAT:
            return 4;
        case Texture::MIRROR:
            return 5;
    }
    return 0;
};

Texture::WrapMode getWrapByIndex(int index)
{
    static std::map<int, Texture::WrapMode> idWrapMap = {
            {getWrapIndex(Texture::CLAMP),           Texture::CLAMP},
            {getWrapIndex(Texture::CLAMP_TO_EDGE),   Texture::CLAMP_TO_EDGE},
            {getWrapIndex(Texture::CLAMP_TO_BORDER), Texture::CLAMP_TO_BORDER},
            {getWrapIndex(Texture::REPEAT),          Texture::REPEAT},
            {getWrapIndex(Texture::MIRROR),          Texture::MIRROR},
    };
    auto iter = idWrapMap.find(index);
    if (iter != idWrapMap.end())
    {
        return iter->second;
    }
    return Texture::CLAMP;
}

int getFilterIndex(Texture::FilterMode m)
{
    switch (m)
    {
        case Texture::LINEAR:
            return 1;
        case Texture::LINEAR_MIPMAP_LINEAR:
            return 2;
        case Texture::LINEAR_MIPMAP_NEAREST:
            return 3;
        case Texture::NEAREST:
            return 4;
        case Texture::NEAREST_MIPMAP_LINEAR:
            return 5;
        case Texture::NEAREST_MIPMAP_NEAREST:
            return 6;
    }
    return 0;
};

Texture::FilterMode getFilterByIndex(int index)
{
    static std::map<int, Texture::FilterMode> idFilterMap = {
            {getFilterIndex(Texture::LINEAR),                 Texture::LINEAR},
            {getFilterIndex(Texture::LINEAR_MIPMAP_LINEAR),   Texture::LINEAR_MIPMAP_LINEAR},
            {getFilterIndex(Texture::LINEAR_MIPMAP_NEAREST),  Texture::LINEAR_MIPMAP_NEAREST},
            {getFilterIndex(Texture::NEAREST),                Texture::NEAREST},
            {getFilterIndex(Texture::NEAREST_MIPMAP_LINEAR),  Texture::NEAREST_MIPMAP_LINEAR},
            {getFilterIndex(Texture::NEAREST_MIPMAP_NEAREST), Texture::NEAREST_MIPMAP_NEAREST},
    };
    auto iter = idFilterMap.find(index);
    if (iter != idFilterMap.end())
    {
        return iter->second;
    }
    return Texture::LINEAR;
}

int getTexGenModeIndex(TexGen::Mode m)
{
    switch (m)
    {
        case TexGen::OBJECT_LINEAR:
            return 1;
        case TexGen::EYE_LINEAR:
            return 2;
        case TexGen::SPHERE_MAP:
            return 3;
        case TexGen::NORMAL_MAP:
            return 4;
        case TexGen::REFLECTION_MAP:
            return 5;
    }
    return 0;
};

TexGen::Mode getTexGenModeByIndex(int index)
{
    static std::map<int, TexGen::Mode> idModeMap = {
            {getTexGenModeIndex(TexGen::OBJECT_LINEAR), TexGen::OBJECT_LINEAR},
            {getTexGenModeIndex(TexGen::EYE_LINEAR), TexGen::EYE_LINEAR},
            {getTexGenModeIndex(TexGen::SPHERE_MAP), TexGen::SPHERE_MAP},
            {getTexGenModeIndex(TexGen::NORMAL_MAP), TexGen::NORMAL_MAP},
            {getTexGenModeIndex(TexGen::REFLECTION_MAP), TexGen::REFLECTION_MAP},
    };
    auto iter = idModeMap.find(index);
    if (iter != idModeMap.end())
    {
        return iter->second;
    }
    return TexGen::OBJECT_LINEAR;
};

}
StateSetView::StateSetView(QWidget *parent) : QWidget(parent), ui(new Ui)
{
    ui->setupUi(this);
    //ui->tex_file_label->setMaximumWidth(150);
    //ui->tex_file_label->setAlignment(Qt::AlignLeft);
    _texture_list_model = new TextureListModel(this);
    //ui->texture_list_view->setModel(m_textureListModel);
    ui->texture_tree_view->setModel(_texture_list_model);
    connect(ui->texture_tree_view, &QTreeView::clicked, this, [=](const QModelIndex &index)
    {
        QString tex_name;
        QString file;
        int wrap_s_index = 0;
        int wrap_t_index = 0;
        int wrap_r_index = 0;
        int min_filter_index = 0;
        int mag_filter_index = 0;
        int mode_index = 0;
        auto texture = _texture_list_model->GetTexture(index);
        auto texgen = _texture_list_model->GetTexGen(index);
        _texture = texture;
        _texgen = texgen;
        if (texture)
        {
            tex_name = texture->getName().c_str();
            auto wrap_s = texture->getWrap(Texture::WRAP_S);
            auto wrap_t = texture->getWrap(Texture::WRAP_T);
            auto wrap_r = texture->getWrap(Texture::WRAP_R);
            auto min_filter = texture->getFilter(Texture::MIN_FILTER);
            auto mag_filter = texture->getFilter(Texture::MAG_FILTER);
            auto texture2D = dynamic_pointer_cast<Texture2D>(texture);
            if (texture2D)
            {
                auto img = texture2D->getImage();
                if (img)
                {
                    file = img->getFileName().c_str();
                }
                wrap_s_index = getWrapIndex(wrap_s);
                wrap_t_index = getWrapIndex(wrap_t);
                wrap_r_index = getWrapIndex(wrap_r);
                min_filter_index = getFilterIndex(min_filter);
                mag_filter_index = getFilterIndex(mag_filter);
            }
        }
        else if (texgen)
        {
            mode_index = getTexGenModeIndex(texgen->getMode());
        }
        if (texture)
        {
            ui->texture_stacked->setCurrentWidget(ui->texture);
        }
        if (texgen)
        {
            ui->texture_stacked->setCurrentWidget(ui->texgen);
        }
        ui->texture_name_label->setText(tex_name);
        ui->texture_file_label->setText(file);
        ui->wrap_s_combo_box->setCurrentIndex(wrap_s_index);
        ui->wrap_t_combo_box->setCurrentIndex(wrap_t_index);
        ui->wrap_r_combo_box->setCurrentIndex(wrap_r_index);
        ui->min_filter_combo_box->setCurrentIndex(min_filter_index);
        ui->mag_filter_combo_box->setCurrentIndex(mag_filter_index);
        ui->mode_combo_box->setCurrentIndex(mode_index);
    });

    ui->wrap_s_combo_box->setProperty("wrap", QVariant::fromValue(Texture::WRAP_S));
    ui->wrap_t_combo_box->setProperty("wrap", QVariant::fromValue(Texture::WRAP_T));
    ui->wrap_r_combo_box->setProperty("wrap", QVariant::fromValue(Texture::WRAP_R));
    auto updateWrapFunc = [=](int index)
    {
        auto wrap = QObject::sender()->property("wrap").value<Texture::WrapParameter>();
        ref_ptr<Texture> rpTexture = _texture;
        if (rpTexture)
        {
            rpTexture->setWrap(wrap, getWrapByIndex(index));
        }
    };
    connect(ui->wrap_s_combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, updateWrapFunc);
    connect(ui->wrap_t_combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, updateWrapFunc);
    connect(ui->wrap_r_combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, updateWrapFunc);

    ui->min_filter_combo_box->setProperty("filter", QVariant::fromValue(Texture::MIN_FILTER));
    ui->mag_filter_combo_box->setProperty("filter", QVariant::fromValue(Texture::MAG_FILTER));
    auto updateFilterFunc = [=](int index)
    {
        auto filter = QObject::sender()->property("filter").value<Texture::FilterParameter>();
        ref_ptr<Texture> rpTexture = _texture;
        if (rpTexture)
        {
            rpTexture->setFilter(filter, getFilterByIndex(index));
        }
    };
    connect(ui->min_filter_combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, updateFilterFunc);
    connect(ui->mag_filter_combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, updateFilterFunc);

    auto updateTexGenMode = [=](int index)
    {
        ref_ptr<TexGen> rpTexGen = _texgen;
        if (rpTexGen)
        {
            rpTexGen->setMode(getTexGenModeByIndex(index));
        }
    };
    connect(ui->mode_combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, updateTexGenMode);

    SetupMaterialGroupBox();
}

StateSetView::~StateSetView()
{
    delete ui;
}

void StateSetView::setStateSet(osg::StateSet *stateSet)
{
    ref_ptr<StateSet> rpStateSet = _state_set;
    if (rpStateSet == stateSet) { return; }
    _state_set = stateSet;
    Reset();
}

#if __cplusplus >= 201703L

using AmbientValueType = typename std::decay<decltype(std::declval<Material>().getAmbient(Material::FRONT))>::type;
using DiffuseValueType = typename std::decay<decltype(std::declval<Material>().getDiffuse(Material::FRONT))>::type;
using EmissionValueType = typename std::decay<decltype(std::declval<Material>().getEmission(Material::FRONT))>::type;
using SpecularValueType = typename std::decay<decltype(std::declval<Material>().getSpecular(Material::FRONT))>::type;
using ShininessValueType = typename std::decay<decltype(std::declval<Material>().getShininess(Material::FRONT))>::type;
using AmbientEdit = DecodeOsgDataEditType<AmbientValueType>::OsgDataEdit;
using DiffuseEdit = DecodeOsgDataEditType<DiffuseValueType>::OsgDataEdit;
using EmissionEdit = DecodeOsgDataEditType<EmissionValueType>::OsgDataEdit;
using SpecularEdit = DecodeOsgDataEditType<SpecularValueType>::OsgDataEdit;
using ShininessEdit = DecodeOsgDataEditType<ShininessValueType>::OsgDataEdit;

void StateSetView::reset()
{
    m_textureListModel->setStateSet(m_stateSet);
    ref_ptr<StateSet> stateSet = m_stateSet;
    ui->name->setText(stateSet ? stateSet->getName().c_str() : "");
    auto *material = dynamic_cast<Material *>(stateSet->getAttribute(StateAttribute::Type::MATERIAL));
    if (material)
    {
        static_cast<AmbientEdit *>(ui->ambient_front_spin_box)->setValue(material->getAmbient(Material::FRONT));
        static_cast<DiffuseEdit *>(ui->diffuse_front_spin_box)->setValue(material->getDiffuse(Material::FRONT));
        static_cast<EmissionEdit *>(ui->emission_front_spin_box)->setValue(material->getEmission(Material::FRONT));
        static_cast<SpecularEdit *>(ui->specular_front_spin_box)->setValue(material->getSpecular(Material::FRONT));
        static_cast<ShininessEdit *>(ui->shininess_front_spin_box)->setValue(material->getShininess(Material::FRONT));
    }
    else
    {
        static_cast<AmbientEdit *>(ui->ambient_front_spin_box)->setValue({});
        static_cast<DiffuseEdit *>(ui->diffuse_front_spin_box)->setValue({});
        static_cast<EmissionEdit *>(ui->emission_front_spin_box)->setValue({});
        static_cast<SpecularEdit *>(ui->specular_front_spin_box)->setValue({});
        static_cast<ShininessEdit *>(ui->shininess_front_spin_box)->setValue({});
    }
}

void StateSetView::setupMaterialGroupBox()
{
    auto setup = [this](QWidget *&spinBox, auto *newSpinBox)
    {
        ui->material_tab->layout()->replaceWidget(spinBox, newSpinBox);
        spinBox->deleteLater();
        spinBox = newSpinBox;
    };

    setup(ui->ambient_front_spin_box, new AmbientEdit(ui->material_tab));
    setup(ui->diffuse_front_spin_box, new DiffuseEdit(ui->material_tab));
    setup(ui->emission_front_spin_box, new EmissionEdit(ui->material_tab));
    setup(ui->specular_front_spin_box, new SpecularEdit(ui->material_tab));
    setup(ui->shininess_front_spin_box, new ShininessEdit(ui->material_tab));
}

#else //__cplusplus >= 201703L

void StateSetView::Reset()
{
    ref_ptr<StateSet> rpStateSet = _state_set;
    _texture_list_model->SetStateSet(rpStateSet);
    _texture = nullptr;
    _texgen = nullptr;
    if (rpStateSet)
    {
        ui->name->setText(rpStateSet->getName().c_str());
    }
    else
    {
        ui->name->setText("");
    }
}

void StateSetView::SetupMaterialGroupBox()
{

}

#endif //__cplusplus >= 201703L


