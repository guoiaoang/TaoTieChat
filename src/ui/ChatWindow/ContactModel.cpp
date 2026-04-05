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
    if (!index.isValid() || index.row() < 0 || index.row() >= m_contacts->size()) {
        return QVariant();
    }
    // 获取行号与内容
    auto row = index.row();  // 获取第几条数据
    auto contact = AccountsManager::contact(m_contacts, row);  // 获取数据内容
    // role 转换为指定类型
    switch (role) {
    case Qt::DisplayRole:
        role = int(ContactData::Role::Nickname);
        break;
    // 联系人页面不启用对联系人信息的修改，需要在弹窗中修改
    // case Qt::EditRole:
    //     role = int(ContactData::Role::Message);
    //     break;
    }
    // 返回有效的内容
    if(    role > int(ContactData::Role::Begin)
        && role < int(ContactData::Role::End  )) {
        return ContactManager(contact).value(ContactData::Role(role));
    }
    return QVariant();
}

void ContactModel::clean() {
    beginResetModel();
    // m_contacts.clear();  // 清除的是智能指针，而不是指针指向的对象，会导致 `m_contacts == nullptr`
    m_contacts->clear();
    endResetModel();
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

    QString account_id = ContactManager(contact).get_account_id();
    int row = -1;
    for(int i = 0; i < m_contacts->size(); ++i) {
        if(ContactManager(m_contacts->at(i)).get_account_id() == account_id) {
            row = i;
            break;
        }
    }

    if(row >= 0) {
        // 变更必须被包裹才会触发界面的更新
        beginRemoveRows(QModelIndex(), row, row);
        m_contacts->removeAt(row);
        endRemoveRows();
    }
}

int ContactModel::contacts_size() const {
    return m_contacts->size();
}

Contact ContactModel::contact(const QModelIndex &index) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_contacts->size()) {
        return nullptr;
    }
    return AccountsManager::contact(m_contacts, index.row());
}
