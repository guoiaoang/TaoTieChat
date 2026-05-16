#pragma once

#include "Headers.h"
#include "Utils.h"

class Accounts : public QObject{
    Q_OBJECT
public:
    static Accounts& instance();
private:
    Accounts();
    Accounts(const Accounts&) = default;
    Accounts& operator=(const Accounts&) = default;
public:
    ~Accounts();

public:
    // 设置当前账号信息及设置好友列表
    void set_current_account(const std::shared_ptr<ContactInfo >& contact_info );
    void set_contact_list   (const std::shared_ptr<ContactInfos>& contact_infos);
    std::shared_ptr<ContactInfo > get_current_account() const;
    std::shared_ptr<ContactInfos> get_contact_list   () const;
    std::shared_ptr<AuthRequest > get_auth_request   () const;
    // 是否是联系人  // 可添加自己
    bool is_self   (const std::shared_ptr<ContactInfo >& contact_info) const;
    bool is_contact(const std::shared_ptr<ContactInfo >& contact_info) const;

signals:
    void signal_account_changed     (const std::shared_ptr<ContactInfo >& contact_info );
    void signal_contact_list_changed(const std::shared_ptr<ContactInfos>& contact_infos);
    // void signal_contact_changed  (const std::shared_ptr<ContactInfo >& contact_info );

private:
    std::shared_ptr<ContactInfo>  m_current_account = std::make_shared<ContactInfo >();
    std::shared_ptr<ContactInfos> m_contact_infos   = std::make_shared<ContactInfos>();
};

