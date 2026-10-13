#include "Accounts.h"
#include "Database.h"

Accounts& Accounts::instance() {
    static Accounts s_accounts;
    return s_accounts;
}

Accounts::Accounts() {
}

Accounts::~Accounts() {
}

// 用户登录与用户信息更新都会触发该函数，更新用户信息时需要保留登录信息
void Accounts::set_current_account(const std::shared_ptr<ContactData::ItemData>& contact_data) {
    if (m_current_account == contact_data) {
        qWarning().noquote().nospace() << "Warnning : set_current_account is same.";
        return;
    }

    contact_data->is_me = true;  // 自身资料标记

    // // 同一账号才复制鉴权信息
    // if (m_current_account->account_id == contact_data->account_id) {
    //     // 如果传入信息没有用户 ID 则复制用户 ID
    //     if (!m_current_account->user_id.isEmpty() || contact_data->user_id.isEmpty()) {
    //         contact_data->user_id    = m_current_account->user_id;
    //     }
    //     // 如果传入信息没有鉴权信息则复制鉴权信息
    //     if (!m_current_account->password.isEmpty() || !m_current_account->token.isEmpty()) {
    //         contact_data->password = m_current_account->password;
    //         contact_data->token    = m_current_account->token;
    //     }
    // }

    emit signal_account_changing(m_current_account, contact_data);
    m_current_account.reset();
    m_current_account = contact_data;
    emit signal_account_changed(contact_data);
}

void Accounts::set_contact_list(const QVector<std::shared_ptr<ContactData::ItemData>>& contact_datas) {
    if (m_contact_datas == contact_datas) {
        qWarning().noquote().nospace() << "Warnning : set_contact_list is same.";
        // return;
    }
    emit signal_contact_list_changing(m_contact_datas, contact_datas);
    m_contact_datas = contact_datas;
    emit signal_contact_list_changed(contact_datas);
}

std::shared_ptr<ContactData::ItemData> Accounts::get_current_account() const {
    return m_current_account;
}

QVector<std::shared_ptr<ContactData::ItemData>> Accounts::get_contact_list() const {
    return m_contact_datas;
}

std::shared_ptr<AuthRequest > Accounts::get_auth_request() const {
    std::shared_ptr<AuthRequest> auth_request = std::make_shared<AuthRequest>();
    do {
        const auto& account_data = get_current_account();
        if (!account_data) { break; }
        if ( account_data->account_id.isEmpty()) { break; }

        auto auth_opt = get_auth_opt(account_data->account_id);
        if (!auth_opt) { break; }

        auto& account_id = auth_opt->account_id;
        auto& user_id    = auth_opt->user_id   ;
        // 没有 accountId 才用 userId 代替
        if (!account_id.isEmpty()) {
            auth_request->setAccountId(account_id);
        }
        else {
            auth_request->setUserId(user_id);
        }
        auto& password = auth_opt->password;
        auto& token    = auth_opt->token   ;
        // 优先使用 token 认证
        if (!token.isEmpty()) {
            auth_request->setToken(token);
        }
        else {
            auth_request->setPassword(password);
        }
    } while (false);
    return auth_request;
}

bool Accounts::is_me(const QString& account_id) const {
    if (m_current_account && !account_id.isEmpty() && account_id == m_current_account->account_id) {
        return true;
    }
    return false;
}

bool Accounts::is_friend(const QString& account_id) const {
    if (m_current_account && !account_id.isEmpty()) {
        for (const auto& contact : m_contact_datas) {
            if (contact && contact->account_id == account_id) {
                return true;
            }
        }
    }
    return false;
}

// 记住密码及自动登陆选项
void Accounts::set_account_auth_opt(const std::shared_ptr<AuthOpt>& auth_opt) {
    if (!auth_opt) { return; }
    const QString&account_id = auth_opt->account_id;
    m_map_account_id_auth_opt[account_id] = auth_opt;
}

// 记住密码及自动登陆选项
std::shared_ptr<AuthOpt> Accounts::get_auth_opt(const QString&account_id) const {
    if (!m_map_account_id_auth_opt.contains(account_id)) { return nullptr; }
    return m_map_account_id_auth_opt.at(account_id);
}

bool Accounts::set_contact(const std::shared_ptr<ContactData::ItemData>& contact_data) {
    if (!contact_data) { return false; }
    const QString&account_id = contact_data->account_id;
    const auto&from_contact_data = m_map_all_contacts[account_id];
    emit signal_contact_info_changing(from_contact_data, contact_data);
    m_map_all_contacts[account_id] = contact_data;
    emit signal_contact_info_changed(contact_data);
    return true;
}

std::shared_ptr<ContactData::ItemData> Accounts::get_contact(const QString&account_id) const {
    // 自己
    if (m_current_account && m_current_account->account_id == account_id) {
        return m_current_account;
    }
    // 缓存
    if (m_map_all_contacts.contains(account_id)) {
        return m_map_all_contacts.at(account_id);
    }
    // 好友
    else if (!m_contact_datas.isEmpty()) {
        for(auto& contact_data : m_contact_datas) {
            if (contact_data && contact_data->account_id == account_id) {
                return contact_data;
            }
        }
    }
    return nullptr;
}

// // 好友申请信息
// void Accounts::set_request_messages(const QString&receiver_account_id, const QString&sender_account_id, const ContactRequestData::ItemData&messages) {
//     m_map_contact_requests[receiver_account_id][sender_account_id] = messages;
// }
// 
// ContactRequestData::ItemData Accounts::get_request_messages(const QString&receiver_account_id, const QString&sender_account_id) const {
//     ContactRequestData::ItemData messages;
//     do {
//         if (!m_map_contact_requests.contains(receiver_account_id)) { break; }
//         const auto&item_sender = m_map_contact_requests.at(receiver_account_id);
//         if (!item_sender.contains(sender_account_id)) { break; }
//         messages = item_sender.at(sender_account_id);
//     } while (false);
//     return messages;
// }
// 
// void Accounts::set_request_messages(const QString&sender_account_id, const ContactRequestData::ItemData&messages) {
//     // 获取 account_id
//     const auto&current_account = get_current_account();
//     if (!current_account) { return; }
//     const QString&receiver_account_id = current_account->account_id;
//     if ( receiver_account_id.isEmpty()) { return; }
//     // 保存消息
//     m_map_contact_requests[receiver_account_id][sender_account_id] = messages;
// }
// 
// ContactRequestData::ItemData Accounts::get_request_messages(const QString&sender_account_id) const {
//     ContactRequestData::ItemData messages;
//     do {
//         // 获取 account_id
//         const auto&current_account = get_current_account();
//         if (!current_account) { return; }
//         const QString&receiver_account_id = current_account->account_id;
//         if ( receiver_account_id.isEmpty()) { return; }
//         // 获取消息
//         if (!m_map_contact_requests.contains(receiver_account_id)) { break; }
//         const auto&item_sender = m_map_contact_requests.at(receiver_account_id);
//         if (!item_sender.contains(sender_account_id)) { break; }
//         messages = item_sender.at(sender_account_id);
//     } while (false);
//     return messages;
// }
