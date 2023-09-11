//
// Created by chudonghao on 2020/3/19.
//

#include "TextureListModel.h"
#include <QDebug>
#include <osg/StateSet>
#include <osg/Texture>
#include <osg/TexGen>

using namespace osg;
TextureListModel::TextureListModel(QObject *parent) : QAbstractItemModel(parent)
{
  _root_item = new MyItem;
  _root_item->parent = nullptr;
  _root_item->depth = DEPTH_ZERO;
}

QModelIndex TextureListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    MyItem *parentItem;

    if (!parent.isValid())
    {
        parentItem = _root_item;
    }
    else
    {
        parentItem = static_cast<MyItem *>(parent.internalPointer());
    }

    MyItem *childItem = parentItem->children[row];
    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex TextureListModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
    {
        return QModelIndex();
    }

    MyItem *childItem = static_cast<MyItem *>(child.internalPointer());
    MyItem *parentItem = childItem->parent;

    if (parentItem == nullptr || parentItem == _root_item)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row, 0, parentItem);
}

int TextureListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        MyItem *item = static_cast<MyItem *>(parent.internalPointer());
        return item->children.size();
    }
    return _root_item->children.size();
}

int TextureListModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant TextureListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
    {
        return QVariant();
    }
    if (!m_stateSet)
    {
        return QVariant();
    }
    MyItem *item = static_cast<MyItem *>(index.internalPointer());
    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
        {
            return QString::number(item->row);
        }
        else if (index.column() == 1)
        {
            return item->type;
        }
    }
    return QVariant();
}

void TextureListModel::SetStateSet(const osg::ref_ptr<osg::StateSet> &state_set)
{
    ref_ptr<StateSet> rpStateSet = m_stateSet;
    if (state_set == rpStateSet)
    {
        return;
    }
    m_stateSet = state_set;
    rpStateSet = m_stateSet;
    beginResetModel();
    qDeleteAll(_items);
    _items.clear();
    _root_item->children.clear();
    if (rpStateSet)
    {
        auto list = rpStateSet->getTextureAttributeList();
        int i = 0;
        for (auto &texAtt : list)
        {
            MyItem *item1 = new MyItem;

            // save
            _items.push_back(item1);
            // parent
            _root_item->children.push_back(item1);
            item1->parent = _root_item;

            item1->depth = DEPTH_ONE;
            item1->row = i;
            int j = 0;
            for (auto &tex : texAtt)
            {
                auto item2 = new MyItem;

                // save
                _items.push_back(item2);
                // parent
                item1->children.push_back(item2);
                item2->parent = item1;

                item2->depth = DEPTH_TWO;
                item2->row = j;
                item2->attribute = tex.second.first.get();

                item2->type = tex.second.first->className();
                ++j;
            }
            ++i;
        }
    }
    endResetModel();
}

osg::ref_ptr<osg::Texture> TextureListModel::GetTexture(const QModelIndex &index)
{
    ref_ptr<StateSet> rpStateSet = m_stateSet;
    if (!rpStateSet || !index.isValid())
    {
        return nullptr;
    }
    MyItem *item = static_cast<MyItem *>(index.internalPointer());
    if (!item->attribute)
    {
        return nullptr;
    }
    return item->attribute->asTexture();
}

osg::ref_ptr<osg::TexGen> TextureListModel::GetTexGen(const QModelIndex &index)
{
    ref_ptr<StateSet> rpStateSet = m_stateSet;
    if (!rpStateSet || !index.isValid())
    {
        return nullptr;
    }
    MyItem *item = static_cast<MyItem *>(index.internalPointer());
    return dynamic_cast<TexGen *>(item->attribute);
}

QVariant TextureListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0:
                return QString("ID");
            case 1:
                return QString::fromUtf8("类型");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

