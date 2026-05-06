#include "Accounts.h"

Accounts& Accounts::instance() {
    static Accounts s_accounts;
    return s_accounts;
}

Accounts::Accounts()
: m_current_account(ContactManager ::create())
, m_contacts       (ContactsManager::create()) {
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

bool Accounts::is_self(const Contact&contact) {
    ContactManager cm_contact(contact          );
    ContactManager cm_self   (m_current_account);
    if(!cm_contact.get_account_id().isEmpty() && cm_contact.get_account_id() == cm_self.get_account_id()) {
        return true;
    }
    return false;
}

bool Accounts::is_contact(const Contact&contact) {
    ContactManager cm_contact(contact);
    if(cm_contact.get_account_id().isEmpty()) {
        return false;
    }
    for(const auto& c : *get_contact_list()) {
        ContactManager cm(c);
        if(!cm.get_account_id().isEmpty() && cm.get_account_id() == cm_contact.get_account_id()) {
            return true;
        }
    }
    return false;
}
