#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"

class ContactModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ContactModel(QObject *parent = nullptr);
    explicit ContactModel(Contacts contacts, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // ====================================================================== //

    void clean();
    void add_contact(Contact contact);
    void remove_contact(Contact contact);
    int  contacts_size() const;
    // Contacts &contacts() { return m_contacts; }
    // 用于获取高亮的联系人信息
    Contact contact(const QModelIndex &index) const;

private:
    Contacts m_contacts;
};
