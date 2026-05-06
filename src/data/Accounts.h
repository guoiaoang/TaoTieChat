#pragma once

#include "Headers.h"
#include "data.h"
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
    // 设置当前账号信息
    void     set_current_account(const Contact&account);
    Contact  get_current_account();
    // 设置好友列表
    void     set_contact_list(const Contacts&contact);
    Contacts get_contact_list();
    // 是否是联系人  // 可添加自己
    bool     is_self   (const Contact&contact);
    bool     is_contact(const Contact&contact);

signals:
    void signal_account_changed(const Contact&account);
    void signal_contact_list_changed(const Contacts&contacts);
    // void signal_contact_changed(const Contact&contact);

private:
    Contact  m_current_account = nullptr;
    Contacts m_contacts        = nullptr;
};

