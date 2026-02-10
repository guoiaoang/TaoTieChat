#include "ContactModel.h"

ContactModel::ContactModel(QObject *parent)
    : ContactModel(nullptr, parent) {
}

ContactModel::ContactModel(Contacts contacts, QObject *parent)
    : QAbstractListModel(parent), m_contacts(contacts) {
    // 防止未初始化
    if(m_contacts == nullptr) {
        m_contacts = QSharedPointer<ContactsData>(new ContactsData);
    }
}

int ContactModel::rowCount(const QModelIndex &parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }
    return m_contacts->size();
}

QVariant ContactModel::data(const QModelIndex &index, int role) const {
    // 先校验范围是否合法
    if (!index.isValid() || index.row() < 0 || index.row() > m_contacts->size()) {
        return QVariant();
    }
    // 获取行号与内容
    auto row = index.row();  // 获取第几条数据
    auto contact = m_contacts->at(row);  // 获取数据内容
    // role 转换为指定类型
    switch (role) {
    case Qt::DisplayRole:
        role = int(ContactData::Role::NickName);
        break;
    case Qt::EditRole:
        role = int(ContactData::Role::Message);
        break;
    }
    // 返回有效的内容
    if(    role > int(ContactData::Role::Begin)
        && role < int(ContactData::Role::End  )) {
        return ContactManager(contact).value(ContactData::Role(role));
    }
    return QVariant();
}

void ContactModel::add_contact(Contact contact) {
    int row = m_contacts->size();
    // 变更必须被包裹才会触发界面的更新
    beginInsertRows(QModelIndex(), row, row);
    m_contacts->append(contact);
    endInsertRows(); // 插入数据后通知 View 更新
}

void ContactModel::remove_contact(Contact contact) {
    if (m_contacts->isEmpty()) {
        return;
    }

    int row = -1;
    for(; row < m_contacts->size(); ++row) {
        if(ContactManager(m_contacts->at(row)).id() == ContactManager(contact).id()) {
            break;
        }
    }

    if(row >= 0) {
        // 变更必须被包裹才会触发界面的更新
        beginRemoveRows(QModelIndex(), row, row);
        m_contacts->remove(row);
        endRemoveRows();
    }
}

Contact ContactModel::contact(const QModelIndex &index) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_contacts->size()) {
        return nullptr;
    }
    return m_contacts->at(index.row());
}
