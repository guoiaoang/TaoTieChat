#include "Accounts.h"

Accounts& Accounts::instance() {
    static Accounts s_accounts;
    return s_accounts;
}

Accounts::Accounts() {
}

Accounts::~Accounts() {
}

void Accounts::set_current_account(const std::shared_ptr<ContactInfo >& contact_info ) {
    m_current_account.reset();
    m_current_account = contact_info;
    emit signal_account_changed(m_current_account);
}

void Accounts::set_contact_list(const std::shared_ptr<ContactInfos>& contact_infos) {
    m_contact_infos.reset();
    m_contact_infos = contact_infos;
    emit signal_contact_list_changed(m_contact_infos);
}

std::shared_ptr<ContactInfo > Accounts::get_current_account() const {
    return m_current_account;
}

std::shared_ptr<ContactInfos> Accounts::get_contact_list() const {
    return m_contact_infos;
}

std::shared_ptr<AuthRequest > Accounts::get_auth_request() const {
    std::shared_ptr<AuthRequest> auth_request = std::make_shared<AuthRequest>();
    if(m_current_account) {
        if(m_current_account->hasBaseInfo()) {
            auto& baseInfo = m_current_account->baseInfo();
            auto& accountId = baseInfo.accountId();
            auto& userId    = baseInfo.userId   ();
            // 没有 accountId 才用 userId 代替
            if(!accountId.isEmpty()) {
                auth_request->setAccountId(accountId);
            }
            else {
                auth_request->setUserId(userId);
            }
        }
        if(m_current_account->hasSelfInfo()) {
            auto& selfInfo = m_current_account->selfInfo();
            auto& password = selfInfo.password();
            auto& token    = selfInfo.token   ();
            // 优先使用 token 认证
            if(!token.isEmpty()) {
                auth_request->setToken(token);
            }
            else {
                auth_request->setPassword(password);
            }
        }
    }
    return auth_request;
}

bool Accounts::is_self(const std::shared_ptr<ContactInfo >& contact_info) const {
    if(contact_info && m_current_account && contact_info->baseInfo().accountId() == m_current_account->baseInfo().accountId()) {
        return true;
    }
    return false;
}

bool Accounts::is_contact(const std::shared_ptr<ContactInfo >& contact_info) const {
    if(m_current_account && contact_info) {
        for(const auto& contact : m_contact_infos->contactInfos()) {
            if(contact.baseInfo().accountId() == contact_info->baseInfo().accountId()) {
                return true;
            }
        }
    }
    return false;
}
