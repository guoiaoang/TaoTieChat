#include "ListModel.h"

ListModel::ListModel(QObject *parent)
    : ListModel(std::make_shared<ContactInfos>(), parent) {
}

ListModel::ListModel(const std::shared_ptr<ContactInfos>&contact_infos, QObject *parent)
    : QAbstractListModel(parent), m_contact_infos(contact_infos ? contact_infos : std::make_shared<ContactInfos>()) {
}

int ListModel::rowCount(const QModelIndex &parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }
    return m_contact_infos ? m_contact_infos->contactInfos().size() : 0;
}

// 从内部变量中获取数据
QVariant ListModel::data(const QModelIndex &index, int role) const {
    if(!m_contact_infos) {
        return QVariant();
    }

    const QList<ContactInfo> &contact_infos = m_contact_infos->contactInfos();
    if(!index.isValid() || index.row() < 0 || index.row() >= contact_infos.size()) {
        return QVariant();
    }

    if(role == Qt::UserRole) {
        // 传出指针有利于判断类型转换结果是否正确
        // 双击显示条目时获取的数据类型要与这里的类型一致
        return QVariant::fromValue(std::make_shared<ContactInfo>(contact_infos.at(index.row())));
    }

    return QVariant();
}

void ListModel::clean() {
    beginResetModel();
    if(!m_contact_infos) {
        m_contact_infos = std::make_shared<ContactInfos>();
    }
    m_contact_infos->setContactInfos(QList<ContactInfo>());
    endResetModel();
}

void ListModel::add_contact(const std::shared_ptr<ContactInfo>&contact_info) {
    if(!contact_info) {
        return;
    }
    if(!m_contact_infos) {
        m_contact_infos = std::make_shared<ContactInfos>();
    }

    QList<ContactInfo> contact_infos = m_contact_infos->contactInfos();
    if(contact_infos.contains(*contact_info)) {
        return;
    }
    int row = contact_infos.size();
    beginInsertRows(QModelIndex(), row, row);
    contact_infos.push_back(*contact_info);
    m_contact_infos->setContactInfos(std::move(contact_infos));
    endInsertRows();
}

void ListModel::remove_contact(const std::shared_ptr<ContactInfo>&contact_info) {
    if(!contact_info || !m_contact_infos) {
        return;
    }

    QList<ContactInfo> contact_infos = m_contact_infos->contactInfos();
    for(int i = 0; i < contact_infos.size(); ++i) {
        if(contact_infos.at(i) == *contact_info) {
            beginRemoveRows(QModelIndex(), i, i);
            contact_infos.removeAt(i);
            m_contact_infos->setContactInfos(std::move(contact_infos));
            endRemoveRows();
            return;
        }
    }
}

void ListModel::set_contacts(const std::shared_ptr<ContactInfos>&contact_infos) {
    beginResetModel();
    m_contact_infos = contact_infos ? contact_infos : std::make_shared<ContactInfos>();
    endResetModel();
}

int ListModel::contacts_size() const {
    return m_contact_infos ? m_contact_infos->contactInfos().size() : 0;
}

std::shared_ptr<ContactInfo> ListModel::get_contact(const QModelIndex &index) const {
    if(!m_contact_infos) {
        return nullptr;
    }

    const QList<ContactInfo> &contact_infos = m_contact_infos->contactInfos();
    if(!index.isValid() || index.row() < 0 || index.row() >= contact_infos.size()) {
        return nullptr;
    }
    return std::make_shared<ContactInfo>(contact_infos.at(index.row()));
}
