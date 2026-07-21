#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

namespace Ui {
class ContactWindow;
}

class ContactWindow : public QWidget {
    Q_OBJECT

public:
    enum class ShowFlag : int {
        unset    = 0,      // no flags
        View     = 1 << 1, // 0x00000002,  // 查看
        Edit     = 1 << 2, // 0x00000004,  // 编辑
        Contact  = 1 << 3, // 0x00000008,  // 联系人
        Stranger = 1 << 4, // 0x00000010,  // 陌生人/非联系人
        Self     = 1 << 5, // 0x00000020,  // 自己
        Search   = 1 << 6, // 0x00000040,  // 搜索页面
        Request  = 1 << 7, // 0x00000080,  // 好友请求
    };
    Q_DECLARE_FLAGS(ShowFlags, ShowFlag)
    // Q_DECLARE_FLAGS(ShowFlags, ShowFlag)
    // Q_DECLARE_OPERATORS_FOR_FLAGS(ShowFlags)

public:
    explicit ContactWindow(QWidget *parent = nullptr);
    ~ContactWindow();

public:
    const std::shared_ptr<ContactInfo>&get_cache_contact_info () const;
    const std::shared_ptr<QPixmap    >&get_cache_avatar_pixmap() const;
    void set_cache_contact_info (const std::shared_ptr<ContactInfo>&contact_info );
    void set_cache_avatar_pixmap(const std::shared_ptr<QPixmap    >&avatar_pixmap);
    void show_contact(const std::shared_ptr<ContactInfo>&contact_info, const std::shared_ptr<QPixmap>&avatar_pixmap = nullptr);
    void set_request_mode(bool request_mode = false);  // 好友申请页面
    void set_show_mode(const ShowFlags show_flags);
    ShowFlags get_show_mode() const { return m_show_flags; }

private:
    // 显示模式
    // bool set_edit_mode(bool edit = true);
    // 按钮和菜单的槽函数
    void slot_button_manager ();
    void slot_button_avatar  ();
    void slot_button_save    ();
    void slot_button_cancel  ();
    void slot_button_add     ();
    void slot_button_accept  ();
    void slot_button_ignore  ();
    void slot_button_messages();
    void slot_button_voice   ();
    void slot_button_video   ();
    void slot_action_edit    ();
    void slot_action_remove  ();

    // get_local_avatar_id_pixmap 根据文件 ID 在本地查找文件并加载为头像图片
    std::shared_ptr<QPixmap> get_local_avatar_id_pixmap(const QString&avatar_id);
    // view_avatar 在媒体窗口查看当前头像
    void view_avatar(const std::shared_ptr<ContactInfo>&contact_info);
    // show_avatar 当前窗口更新头像显示，无网络及校验功能
    void show_avatar(const std::shared_ptr<QPixmap>&avatar_pixmap);
    // 传入头像 ID，自动从本地或网络加载头像
    void show_avatar_id(const QString&avatar_id);
    // dialog_select_avatar_file 选择头像文件的对话框
    QString dialog_select_avatar_file();

    // network
    void update_profile_text(const ContactInfo&contact_info, const update_profile_callback&callback_update_profile = nullptr);
    void update_profile_avatar(const QString&avatar_path);

Q_SIGNALS:
    void contact_add    (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_remove (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_message(const std::shared_ptr<ContactInfo>&contact_info);
    void contact_voice  (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_video  (const std::shared_ptr<ContactInfo>&contact_info);

private:
    std::shared_ptr<Ui::ContactWindow> m_ui                 ;
    std::shared_ptr<QMenu            > m_menu_manager       ;
    std::shared_ptr<QAction          > m_action_edit        ;
    std::shared_ptr<QAction          > m_action_remove      ;
    std::shared_ptr<ContactInfo      > m_cache_contact_info ;
    std::shared_ptr<QPixmap          > m_cache_avatar       ;
    bool m_edit_mode = false;
    ShowFlags m_show_flags = ShowFlag::unset;
    Utils::WidgetTools::LineEdit m_edit_tool;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ContactWindow::ShowFlags)
