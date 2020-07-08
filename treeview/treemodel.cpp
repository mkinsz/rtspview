#include "treeitem.h"
#include "treemodel.h"
#include <QStringList>
#include <QIcon>
#include <QFile>
#include <QDomDocument>
#include <QMimeData>

static QList<TreeData> parse(QString file_name)
{
    QList<TreeData> dataList;

    if (file_name.isEmpty())
        return dataList;

    QFile file(file_name);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return dataList;

    QDomDocument document;
    QString error;
    int row = 0, column = 0;
    if (!document.setContent(&file, false, &error, &row, &column))
        return dataList;

    if (document.isNull())
        return dataList;

    QDomElement root = document.documentElement();
    QDomNodeList list = root.childNodes();
    int count = list.count();
    for (int i = 0; i < count; ++i)
    {
        QDomNode dom_node = list.item(i);
        QDomElement element = dom_node.toElement();

        //获取子节点，数目为，包括：toolexename、tooltip、toolicon、website
        QDomNodeList child_list = element.childNodes();
        int child_count = child_list.count();

        TreeData data;

        for (int j = 0; j < child_count; ++j)
        {
            QDomNode child_dom_node = child_list.item(j);
            QDomElement child_element = child_dom_node.toElement();
            QString child_tag_name = child_element.tagName();
            QString child__tag_value = child_element.text();

            if (QString::compare(child_tag_name, "parentID") == 0)
            {
                data.parentId = child__tag_value.toInt();
            }
            else if (QString::compare(child_tag_name, "ID") == 0)
            {
                data.id = child__tag_value.toInt();
            }
            else if (QString::compare(child_tag_name, "text") == 0)
            {
                data.text = child__tag_value;
            }
            else if (QString::compare(child_tag_name, "address") == 0)
            {
                data.address = child__tag_value;
            }
            else if (QString::compare(child_tag_name, "tooltip") == 0)
            {
                data.tooltip = child__tag_value;
            }
            else if (QString::compare(child_tag_name, "toolicon") == 0)
            {
                data.toolicon = child__tag_value;
            }
        }
        dataList.append(data);
    }

    return dataList;
}

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Title";
    rootItem = new TreeItem(rootData);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

void TreeModel::setXML(QString xmlFile)
{
    dataList = parse(xmlFile);
    setupModelData(rootItem);
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem *>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());

    TreeData toolUtil;
    for (int i = 0; i < dataList.count(); ++i)
    {
        toolUtil = dataList.at(i);
        if (toolUtil.id == item->data(0)) break;
    }

    switch(role) {
    case Qt::DisplayRole: return toolUtil.text;
    case Qt::ToolTipRole: return toolUtil.tooltip;
    case Qt::DecorationRole: return QIcon(":/resources/" + toolUtil.toolicon);
    case Qt::UserRole: return toolUtil.address;
    default: return QVariant();
    }

//    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem *>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    return parentItem->childCount();
}

TreeItem *TreeModel::item(TreeItem *item, TreeData data)
{
    TreeItem *treeItem = nullptr;
    if (!item)
        return treeItem;

    int parentId = data.parentId;

    if (item->data(0) == parentId)
    {
        treeItem = item;
    }
    else
    {
        for (int j = 0; j < item->childCount(); ++j)
        {
            TreeItem *childItem = item->child(j);
            TreeItem *item2 = this->item(childItem, data);
            if (item2)
            {
                treeItem = item2;
                break;
            }
        }
    }

    return treeItem;
}

void TreeModel::setupModelData(TreeItem *parent)
{
    QList<TreeItem *> parents;
    parents << parent;

    for (int i = 0; i < dataList.count(); ++i)
    {
        TreeData tool = dataList.at(i);
        QList<QVariant> columnData;
        columnData << tool.id;

        for (int j = 0; j < parents.count(); ++j)
        {
            TreeItem *item = this->item(parents.at(j), tool);
            if (item)
                item->appendChild(new TreeItem(columnData, item));
            else
                parents.last()->appendChild(new TreeItem(columnData, parents.last()));
        }
    }
}

QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() <= 0) return 0;

    QMimeData *mdata = QAbstractItemModel::mimeData(indexes);

    mdata->setData("drag", "鼠标拖拽");
    mdata->setData("name", data(indexes.at(0), Qt::DisplayRole).toByteArray());
    mdata->setData("address", data(indexes.at(0), Qt::UserRole).toByteArray());
    return mdata;
}

bool TreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(data)
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    //    setItem(m_rowCount, 0, new QStandardItem(QString::fromLocal8Bit(data->data("name"))));
    //    setItem(m_rowCount, 1, new QStandardItem(QString::fromLocal8Bit(data->data("info"))));
    return true;
}

QStringList TreeModel::mimeTypes() const
{
    QStringList types;
    types << "drag";
    return types;
}
