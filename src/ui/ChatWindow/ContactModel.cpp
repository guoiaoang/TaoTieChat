#include "ContactModel.h"

ContactModel::ContactModel(QObject *parent)
    : ContactModel(std::make_shared<ContactInfos>(), parent) {
}

ContactModel::ContactModel(const std::shared_ptr<ContactInfos>&contact_infos, QObject *parent)
    : QAbstractListModel(parent), m_contact_infos(contact_infos ? contact_infos : std::make_shared<ContactInfos>()) {
}

int ContactModel::rowCount(const QModelIndex &parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }
    return m_contact_infos ? m_contact_infos->contactInfos().size() : 0;
}

// 从内部变量中获取数据
QVariant ContactModel::data(const QModelIndex &index, int role) const {
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

void ContactModel::clean() {
    beginResetModel();
    if(!m_contact_infos) {
        m_contact_infos = std::make_shared<ContactInfos>();
    }
    m_contact_infos->setContactInfos(QList<ContactInfo>());
    endResetModel();
}

void ContactModel::add_contact(const std::shared_ptr<ContactInfo>&contact_info) {
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

void ContactModel::remove_contact(const std::shared_ptr<ContactInfo>&contact_info) {
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

void ContactModel::set_contacts(const std::shared_ptr<ContactInfos>&contact_infos) {
    beginResetModel();
    m_contact_infos = contact_infos ? contact_infos : std::make_shared<ContactInfos>();
    endResetModel();
}

int ContactModel::contacts_size() const {
    return m_contact_infos ? m_contact_infos->contactInfos().size() : 0;
}

std::shared_ptr<ContactInfo> ContactModel::get_contact(const QModelIndex &index) const {
    if(!m_contact_infos) {
        return nullptr;
    }

    const QList<ContactInfo> &contact_infos = m_contact_infos->contactInfos();
    if(!index.isValid() || index.row() < 0 || index.row() >= contact_infos.size()) {
        return nullptr;
    }
    return std::make_shared<ContactInfo>(contact_infos.at(index.row()));
}
