#pragma once

#include "headers.h"
#include "Utils.h"

class ContactModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ContactModel(QObject *parent = nullptr);
    explicit ContactModel(const std::shared_ptr<ContactInfos>&contact_infos, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::UserRole) const override;

    // ====================================================================== //

    void clean();
    void add_contact   (const std::shared_ptr<ContactInfo >&contact_info );
    void remove_contact(const std::shared_ptr<ContactInfo >&contact_info );
    void set_contacts  (const std::shared_ptr<ContactInfos>&contact_infos);
    int  contacts_size() const;
    // ContactInfos &contact_infos() { return m_contact_infos; }
    // 用于获取高亮的联系人信息
    std::shared_ptr<ContactInfo> get_contact(const QModelIndex &index) const;

private:
    std::shared_ptr<ContactInfos> m_contact_infos;
};
