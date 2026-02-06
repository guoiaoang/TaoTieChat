#include "ContentModel.h"

ContentModel::ContentModel(QObject *parent)
    : ContentModel(nullptr, parent) {
}

ContentModel::ContentModel(Contents contents, QObject *parent)
    : QAbstractListModel(parent), m_contents(contents) {
    // 防止未初始化
    if(m_contents == nullptr) {
        m_contents = QSharedPointer<TypeContents>(new TypeContents);
    }
}

int ContentModel::rowCount(const QModelIndex &parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }
    return m_contents->size();
}

QVariant ContentModel::data(const QModelIndex &index, int role) const {
    // 先校验范围是否合法
    if (!index.isValid() || index.row() < 0 || index.row() > m_contents->size()) {
        return QVariant();
    }
    // 获取行号与内容
    auto row = index.row();  // 获取第几条数据
    auto content = m_contents->at(row);  // 获取数据内容
    // role 转换为指定类型
    switch (role) {
    case Qt::DisplayRole:
        role = int(TypeContent::ModelRole::NickName);
        break;
    case Qt::EditRole:
        role = int(TypeContent::ModelRole::Message);
        break;
    }
    // 返回有效的内容
    if(    role > int(TypeContent::ModelRole::Begin)
        && role < int(TypeContent::ModelRole::End  )) {
        return content->data(TypeContent::ModelRole(role));
    }
    return QVariant();
}

void ContentModel::add_content(Content content) {
    int row = m_contents->size();
    // 变更必须被包裹才会触发界面的更新
    beginInsertRows(QModelIndex(), row, row);
    m_contents->append(content);
    endInsertRows(); // 插入数据后通知 View 更新
}

void ContentModel::remove_content(Content content) {
    if (m_contents->isEmpty()) {
        return;
    }

    int row = -1;
    for(; row < m_contents->size(); ++row) {
        if(m_contents->at(row)->id() == content->id()) {
            break;
        }
    }

    if(row >= 0) {
        // 变更必须被包裹才会触发界面的更新
        beginRemoveRows(QModelIndex(), row, row);
        m_contents->remove(row);
        endRemoveRows();
    }
}
