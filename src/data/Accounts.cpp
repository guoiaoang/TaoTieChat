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

void Accounts::set_current_account(const std::shared_ptr<ContactInfo>& contact_info) {
    // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
    // qDebug().noquote().nospace() << "old     account contactInfo: " << Utils::Transform::to_string(m_current_account);
    // qDebug().noquote().nospace() << "To      account contactInfo: " << Utils::Transform::to_string(contact_info);
    if (m_current_account == contact_info) {
        qWarning().noquote().nospace() << "Warnning : set_current_account is same.";
        return;
    }
    // 补充构建
    auto&old_contact_info = m_current_account;
    auto to_contact_info  = !contact_info ? std::make_shared<ContactInfo>() : std::make_shared<ContactInfo>(*contact_info.get());
    // qDebug().noquote().nospace() << "to      account contactInfo: " << Utils::Transform::to_string(to_contact_info);
    if (!old_contact_info) { old_contact_info = std::make_shared<ContactInfo>(); }
    if (!to_contact_info ) { to_contact_info  = std::make_shared<ContactInfo>(); }
    if (!old_contact_info->hasSelfInfo()) { old_contact_info->setSelfInfo(ContactInfoSelf{}); }
    if (!to_contact_info->hasSelfInfo ()) { to_contact_info ->setSelfInfo(ContactInfoSelf{}); }
    auto&old_base_info = old_contact_info->baseInfo();
    auto&to_base_info  = to_contact_info ->baseInfo();
    auto&old_self      = old_contact_info->selfInfo();
    auto&to_self       = to_contact_info ->selfInfo();
    old_self.setModifySelfInfo(true);
    to_self .setModifySelfInfo(true);
    // 判断是否是同一账号
    auto old_account_id = old_base_info.accountId();
    auto to_account_id  = to_base_info .accountId();
    if (old_account_id == to_account_id && !old_account_id.isEmpty() && !to_account_id.isEmpty()) {
        // 需要复制登陆信息
        to_self.setUserId          (!old_self.userId          ().isEmpty() ? old_self.userId          () : to_self.userId          ());
        to_self.setPassword        (!old_self.password        ().isEmpty() ? old_self.password        () : to_self.password        ());
        to_self.setToken           (!old_self.token           ().isEmpty() ? old_self.token           () : to_self.token           ());
        to_self.setSecurityQuestion(!old_self.securityQuestion().isEmpty() ? old_self.securityQuestion() : to_self.securityQuestion());
        to_self.setSecurityAnswer  (!old_self.securityAnswer  ().isEmpty() ? old_self.securityAnswer  () : to_self.securityAnswer  ());
    }
    emit signal_account_changing(m_current_account, to_contact_info);
    m_current_account.reset();
    m_current_account = to_contact_info;
    emit signal_account_changed(to_contact_info);
}

void Accounts::set_contact_list(const std::shared_ptr<ContactInfos>& contact_infos) {
    if (m_contact_infos == contact_infos) {
        qWarning().noquote().nospace() << "Warnning : set_contact_list is same.";
        // return;
    }
    emit signal_contact_list_changing(m_contact_infos, contact_infos);
    if (m_contact_infos != contact_infos) {
        m_contact_infos.reset();
    }
    m_contact_infos = contact_infos;
    emit signal_contact_list_changed(contact_infos);
    // qDebug().noquote().nospace() << "contact list size : " << m_contact_infos->contactInfos().size();
}

std::shared_ptr<ContactInfo > Accounts::get_current_account() const {
    return m_current_account;
}

std::shared_ptr<ContactInfos> Accounts::get_contact_list() const {
    return m_contact_infos;
}

std::shared_ptr<AuthRequest > Accounts::get_auth_request() const {
    std::shared_ptr<AuthRequest> auth_request = std::make_shared<AuthRequest>();
    auto current_account = get_current_account();
    if (current_account) {
        if (current_account->hasBaseInfo()) {
            auto& baseInfo  = current_account->baseInfo();
            auto& accountId = baseInfo.accountId();
            auto& userId    = baseInfo.userId   ();
            // 没有 accountId 才用 userId 代替
            if (!accountId.isEmpty()) {
                auth_request->setAccountId(accountId);
            }
            else {
                auth_request->setUserId(userId);
            }
        }
        if (current_account->hasSelfInfo()) {
            auto& selfInfo = current_account->selfInfo();
            auto& password = selfInfo.password();
            auto& token    = selfInfo.token   ();
            // 优先使用 token 认证
            if (!token.isEmpty()) {
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
    if (contact_info && m_current_account && contact_info->baseInfo().accountId() == m_current_account->baseInfo().accountId()) {
        return true;
    }
    return false;
}

bool Accounts::is_contact(const std::shared_ptr<ContactInfo >& contact_info) const {
    // qDebug().noquote().nospace() << "contact list size : " << m_contact_infos->contactInfos().size();
    if (m_current_account && contact_info) {
        for (const auto& contact : m_contact_infos->contactInfos()) {
            if (contact.baseInfo().accountId() == contact_info->baseInfo().accountId()) {
                return true;
            }
        }
    }
    return false;
}

// 记住密码及自动登陆选项
void Accounts::set_account_auth_opt(const QString&account_id, const std::shared_ptr<AuthOpt>& auth_opt) {
    m_map_account_id_auth_opt[account_id] = auth_opt;
}

// 记住密码及自动登陆选项
std::shared_ptr<AuthOpt> Accounts::get_auth_opt(const QString&account_id) const {
    return m_map_account_id_auth_opt.at(account_id);
}

bool Accounts::set_contact(const QString&account_id, const std::shared_ptr<ContactInfo>& contact_info) {
    // 只存缓存
    if (contact_info->hasBaseInfo() && contact_info->baseInfo().accountId() == account_id) {
        m_map_all_contacts[account_id] = contact_info;
        return true;
    }
    return false;
}

std::shared_ptr<ContactInfo> Accounts::get_contact(const QString&account_id) const {
    // 自己
    if (m_current_account->hasBaseInfo() && m_current_account->baseInfo().accountId() == account_id) {
        return m_current_account;
    }
    // 缓存
    if (m_map_all_contacts.contains(account_id)) {
        return m_map_all_contacts.at(account_id);
    }
    // 好友
    else if (m_contact_infos) {
        for(auto& contact : m_contact_infos->contactInfos()) {
            if (contact.hasBaseInfo() && contact.baseInfo().accountId() == account_id) {
                return std::make_shared<ContactInfo>(contact);
            }
        }
    }
    return nullptr;
}

bool Accounts::set_avatar(const QString&avatar_id, const QPixmap& pixamp) {
    m_map_avatars[avatar_id] = pixamp;
    return false;
}

const QPixmap Accounts::get_avatar(const QString&avatar_id) {
    if (!m_map_avatars.contains(avatar_id)) {
        // 获取头像  // 圆角头像
        auto avatar_file_path = Database::instance().get_local_file_id_path(avatar_id + ";size:128*128");
        auto pixmap_avatar_source = QPixmap(avatar_file_path);
        // if(pixmap_avatar_source.isNull()) {
        //     // 修改图标大小
        //     int height_item = opt.decorationSize.height();
        //     int height_icon = (height_item * group_height_scale());
        //     opt.decorationSize = QSize(height_icon, height_icon);

        //     // 根据颜色和大小生成图标
        //     QColor color_icon = opt.palette.color(QPalette::Text);  // 指定固定的颜色
        //     static Utils::FontIcons icon_font = Utils::FontIcons();
        //     if (icon_font.size.avatar != opt.decorationSize || icon_font.color.normal != color_icon) {
        //         icon_font.size.avatar = opt.decorationSize;
        //         icon_font.color.normal = color_icon;
        //         icon_font.update();
        //     }

        //     QIcon&ico_show = icon_font.icon.contacts;
        //     pixmap_avatar_source = QPixmap(ico_show.pixmap(opt.decorationSize, QIcon::Normal));
        // }
        auto pixmap_avatar = Utils::to_round_pixmap(pixmap_avatar_source, 128 / 2);
        m_map_avatars[avatar_id] = pixmap_avatar;
    }
    if (m_map_avatars.contains(avatar_id)) {
        return m_map_avatars.at(avatar_id);
    }
    return QPixmap();
}
