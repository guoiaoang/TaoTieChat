#include "Accounts.h"

Accounts& Accounts::instance() {
    static Accounts s_accounts;
    return s_accounts;
}

Accounts::~Accounts() {
}

void Accounts::set_current_account(const Contact&account) {
    m_current_account = account;
    emit signal_account_changed(m_current_account);
}

Contact Accounts::get_current_account() {
    return m_current_account;
}

void Accounts::set_contact_list(const Contacts&contact) {
    m_contacts = contact;
    emit signal_contact_list_changed(m_contacts);
}

Contacts Accounts::get_contact_list() {
    return m_contacts;
}

bool Accounts::is_contact(const Contact&contact) {
    if(contact && m_contacts) {
        ;m_contacts;
    }
    return false;
}
