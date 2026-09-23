#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class ItemModel;
class ContactModel;
class ItemDelegate;
class SearchWindow;
class ContactWindow;
class ChatroomModel;
class MessageWindow;
class ChatroomProxy;

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();
    void reinit();  // 重新初始化界面及数据

private:
    void release_resources();
    void init();

    // network
    bool updata_current_account_data();
    bool network_status_sync();
    bool download_contact_avatar(const std::shared_ptr<ContactData::ItemData>&contact_data);

    // 显示设置菜单
    void slot_show_settings_menu();
    // 显示 创建/添加 好友/群组 菜单
    void slot_show_add_menu();

    // // // 聊天室管理
    // // 单击聊天室中的一个对话  // 显示聊天窗口
    // void slot_chat_room_item_selected(const QModelIndex &index);
    // // 双击聊天室中的一个对话  // 微信会弹窗聊天窗口
    // void slot_chat_room_item_activated(const QModelIndex &index);

    // 需要创建副本，而不是使用指针共享同一份内存
    void set_cache_contact_data(const std::shared_ptr<ContactData::ItemData>&contact_data);
    void update_contact_avatar (const QString&avatar_id, bool local_only = false);  // local_only 防止循环调用
    std::shared_ptr<ContactData::ItemData> get_cache_contact_data  () const;
    std::shared_ptr<QImage               > get_cache_contact_avatar() const;

    void show_avatar(const std::shared_ptr<QImage>&avatar_image);

    // 切换并显示聊天室
    // void show_chatroom(const std::shared_ptr<ContactData::ItemData>&contact_data);

    // 获取聊天记录窗口
    MessageWindow* get_message_window(const QString&account_id);
    // 更新列表中显示的最后一条消息
    void update_last_message(const QString&chatroom_id, const std::shared_ptr<MessageData::ItemData>&last_message);

    // 选中并高亮指定的聊天列表
    bool highlight_chatroom(const QString&chatroom_id);

public Q_SLOTS:
    // // 聊天室管理
    // 添加到聊天室列表
    void add_chat_room(const std::shared_ptr<ContactData::ItemData>&contact_data);
    // 显示指定聊天室的对话
    void show_chat_room_messages(const std::shared_ptr<ContactData::ItemData>&contact_data);

    // // 联系人管理
    // 添加到联系人列表
    void add_contact(const std::shared_ptr<ContactData::ItemData>&contact_data, const ENUMS::ItemGroupType&group_type = ENUMS::ItemGroupType::NONE);
    // model/view 模式下的选择  // 按键移动导致所选行变化时触发激活动作
    void handle_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous);
    // 单击联系人/群组
    void handle_contact_item_clicked(const QModelIndex &index);

    // model/view 模式下的选择  // 按键移动导致所选行变化时触发激活动作
    void handle_chatroom_selection_changed(const QModelIndex &current, const QModelIndex &previous);
    // 单击聊天室
    void handle_chatroom_item_clicked(const QModelIndex &index);

    // 添加聊天内容到对话内
    void add_message(const std::shared_ptr<SyncMessage>&sync_message);

    // 更新联系人信息，可能是头像、昵称、状态等
    void current_account_data_changed();
    void update_contact_data(const std::shared_ptr<ContactData::ItemData>&contact_data);

    // 导航按钮事件
    void slot_button_navigate_avatar ();
    void slot_button_navigate_message();
    void slot_button_navigate_contact();

    // 发消息
    void slot_message_to(const std::shared_ptr<ContactData::ItemData>&contact_data);
    void slot_voice_to  (const std::shared_ptr<ContactData::ItemData>&contact_data);
    void slot_video_to  (const std::shared_ptr<ContactData::ItemData>&contact_data);

signals:
    void signout();

public slots:
    void slot_file_id_downloaded(const QString&file_id);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    std::shared_ptr<Ui::ChatWindow > m_ui;
    // 主窗口菜单
    // 记住密码与自动登陆
    std::shared_ptr<QMenu  > m_menu_settings   ;
    std::shared_ptr<QAction> m_action_settings ;
    std::shared_ptr<QAction> m_action_rember_password;
    std::shared_ptr<QAction> m_action_auto_login     ;
    std::shared_ptr<QAction> m_action_signout  ;
    std::shared_ptr<QAction> m_action_exit     ;
    // 搜索按钮
    std::shared_ptr<QMenu  > m_menu_add        ;
    std::shared_ptr<QAction> m_action_add      ;
    std::shared_ptr<QAction> m_action_create   ;
    // model/view/delegate/proxy
    std::shared_ptr<ChatroomModel> m_chatroom_model  ;
    std::shared_ptr<ChatroomProxy> m_chatroom_proxy  ;
    std::shared_ptr<ContactModel > m_contact_model   ;
    std::shared_ptr<ItemDelegate > m_contact_delegate;
    std::shared_ptr<SearchWindow > m_search_window   ;
    std::shared_ptr<ContactWindow> m_contact_window  ;  // 当前显示的联系人详情窗口
    //
    std::shared_ptr<ContactData::ItemData> m_cache_self_data  ;
    std::shared_ptr<QImage               > m_cache_self_avatar;
};
