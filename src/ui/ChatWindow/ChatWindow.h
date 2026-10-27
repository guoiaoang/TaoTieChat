#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class SearchWindow;
class ContactWindow;
class MessageWindow;
class ItemModel;
class ItemDelegate;
class ContactModel;
class ContactProxy;
class ChatroomModel;
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

public:
    static bool is_supported_image_formats(const QString& file_name);

private:
    void release_resources();
    void init();

    // network
    bool network_status_sync();

    // 显示设置菜单
    void slot_show_settings_menu();
    // 显示 创建/添加 好友/群组 菜单
    void slot_show_add_menu();

    // 需要创建副本，而不是使用指针共享同一份内存
    void set_cache_contact_data(std::shared_ptr<ContactData::ItemData> contact_data);
    std::shared_ptr<ContactData::ItemData> get_cache_contact_data  () const;

    // online_download:是否在线下载，防止循环调用
    void update_contact_avatar(const QString& avatar_id, bool online_download);
    void show_avatar(std::shared_ptr<QImage> avatar_image);

    // 获取聊天记录窗口
    MessageWindow* get_message_window(const QString& account_id);
    // 更新列表中显示的最后一条消息
    void update_last_message(std::shared_ptr<MessageData::ItemData> last_message);

    // 选中并高亮指定的聊天列表
    bool highlight_chatroom(const QString& chatroom_id);

public Q_SLOTS:
    // 聊天室管理
    // 添加到聊天室列表
    void add_chat_room(const QString& chatroom_id);
    // 显示指定聊天室的对话
    void show_chat_room_messages(std::shared_ptr<ContactData::ItemData> contact_data);

    // // 联系人管理
    // 添加到联系人列表
    void add_contact(std::shared_ptr<ContactData::ItemData> contact_data, const ENUMS::ItemGroupType&group_type = ENUMS::ItemGroupType::NONE);
    // model/view 模式下的选择  // 按键移动导致所选行变化时触发激活动作
    void slot_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous);
    // 单击联系人/群组
    void slot_contact_item_clicked(const QModelIndex &index);

    // model/view 模式下的选择  // 按键移动导致所选行变化时触发激活动作
    void slot_chatroom_selection_changed(const QModelIndex &current, const QModelIndex &previous);
    // 单击聊天室
    void slot_chatroom_item_clicked(const QModelIndex &index);

    // 添加聊天内容到对话内
    void add_message(std::shared_ptr<SyncMessage> sync_message);

    // 更新联系人信息，可能是头像、昵称、状态等
    // void slot_update_current_account_data();
    void slot_update_contact_data(std::shared_ptr<ContactData::ItemData> contact_data);

    // 导航按钮事件
    void slot_button_navigate_avatar ();
    void slot_button_navigate_message();
    void slot_button_navigate_contact();

    // 发消息
    void slot_message_to(std::shared_ptr<ContactData::ItemData> contact_data);
    void slot_voice_to  (std::shared_ptr<ContactData::ItemData> contact_data);
    void slot_video_to  (std::shared_ptr<ContactData::ItemData> contact_data);

    // 更新联系人信息
    void slot_update_contact_list_info (std::shared_ptr<ContactData::ItemData> to_contact_data);  // 联系人列表与好友申请
    void slot_update_chatroom_list_info(std::shared_ptr<ContactData::ItemData> to_contact_data);  // 对话列表

public slots:
    void slot_update_avatar_file  (const Network::FileDownloadNotify notify);  // 左上角主界面的图标
    void slot_update_chatroom_file(const Network::FileDownloadNotify notify);  // 聊天室列表中的头像
    void slot_update_contact_file (const Network::FileDownloadNotify notify);  // 联系人列表中的头像

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
    //
    std::shared_ptr<QAction> m_action_clean_search_box;
    // 搜索按钮
    std::shared_ptr<QMenu  > m_menu_add        ;
    std::shared_ptr<QAction> m_action_add      ;
    std::shared_ptr<QAction> m_action_create   ;
    // model/view/delegate/proxy
    std::shared_ptr<ContactModel > m_contact_model   ;
    std::shared_ptr<ItemDelegate > m_contact_delegate;
    std::shared_ptr<ContactProxy > m_contact_proxy   ;
    std::shared_ptr<ChatroomModel> m_chatroom_model  ;
    std::shared_ptr<ChatroomProxy> m_chatroom_proxy  ;
    // window
    std::shared_ptr<SearchWindow > m_search_window   ;
    std::shared_ptr<ContactWindow> m_contact_window  ;  // 当前显示的联系人详情窗口
    // cache
    std::shared_ptr<ContactData::ItemData> m_cache_self_data;
};
