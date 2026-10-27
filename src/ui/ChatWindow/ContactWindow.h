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
    // 显示来源，唯一
    enum class SourceFlag : int {
        UNSET    = 1 << 0,
        Profile  = 1 << 1,  // 0x00000001  // 个人资料
        Search   = 1 << 2,  // 0x00000002  // 搜索结果
        Request  = 1 << 3,  // 0x00000004  // 好友请求
        Contact  = 1 << 4,  // 0x00000008  // 好友列表
    };
    Q_ENUM(SourceFlag)

    // 显示模式，唯一
    enum class ViewFlag : int {
        View     = 1 << 0,  // 0x00000000  // 查看
        Edit     = 1 << 1,  // 0x00000001  // 编辑
        ViewOnly = 1 << 2,  // 0x00000002  // 只允许查看
    };
    Q_ENUM(ViewFlag)

    // 好友关系，可重叠
    enum class ShipFlag : int {
        Stranger = 1 << 0,  // 0x00000001  // 陌生人
        Friend   = 1 << 1,  // 0x00000002  // 好友
        Self     = 1 << 2,  // 0x00000004  // 自己
    };
    // 声明标志集合类型 ShipFlags
    Q_DECLARE_FLAGS(ShipFlags, ShipFlag)
    // // 为 ShipFlags 激活标准位运算符（必须放在类外或全局作用域）
    // Q_DECLARE_OPERATORS_FOR_FLAGS(ShipFlags)
    // (可选) 如果需要将标志暴露给 Qt 元对象系统/QML/属性系统
    Q_FLAG(ShipFlags)

public:
    explicit ContactWindow(QWidget *parent = nullptr);
    ~ContactWindow();

public:
    std::shared_ptr<ContactData::ItemData> get_contact_data() const;
    void set_contact_data(std::shared_ptr<ContactData::ItemData> contact_data, const SourceFlag&source_flag);

private:
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
    // 新添加好友时刷新好友关系状态
    void slot_contact_status_changed(QVector<std::shared_ptr<ContactData::ItemData>> to_contact_datas);
    void slot_contact_info_changed  (        std::shared_ptr<ContactData::ItemData>  to_contact_data  );
    void slot_file_downloaded       (const Network::FileDownloadNotify notify);

    // view_avatar 在媒体窗口查看当前头像
    void view_avatar();
    // 选择头像文件的对话框
    QString dialog_select_avatar_file();

    // network
    void update_profile_text(std::shared_ptr<ContactData::ItemData> contact_data, update_profile_callback callback_update_profile = nullptr);
    void update_profile_avatar(const QString&avatar_path);
    // 用户仅修改头像时不应该完全重置资料
    // void update_profile_avatar_only(std::shared_ptr<ContactData::ItemData> avatar_data);

// Q_SIGNALS:
//     void sig_contact_add    (std::shared_ptr<ContactData::ItemData> contact_data);
//     void sig_contact_remove (std::shared_ptr<ContactData::ItemData> contact_data);
//     void sig_contact_message(std::shared_ptr<ContactData::ItemData> contact_data);
//     void sig_contact_voice  (std::shared_ptr<ContactData::ItemData> contact_data);
//     void sig_contact_video  (std::shared_ptr<ContactData::ItemData> contact_data);

public:
    const SourceFlag get_source_flag() const { return m_source_flag; }
    const ViewFlag   get_view_flag  () const { return m_view_flag  ; }
    const ShipFlags  get_ship_flags () const { return m_ship_flags ; }
    void set_source_flag(const SourceFlag&source_flag) { updata_show(    source_flag  , get_view_flag(), get_ship_flags()); }
    void set_view_flag  (const ViewFlag  &view_flag  ) { updata_show(get_source_flag(),     view_flag  , get_ship_flags()); }
    void set_ship_flags (const ShipFlags &ship_flags ) { updata_show(get_source_flag(), get_view_flag(),     ship_flags  ); }

private:
    void updata_show(const SourceFlag&source_flag, const ViewFlag&view_flag, const ShipFlags&ship_flags);

private:
    SourceFlag m_source_flag = SourceFlag::UNSET;
    ViewFlag   m_view_flag   = ViewFlag::View;
    ShipFlags  m_ship_flags  = ShipFlag::Stranger;

private:
    std::shared_ptr<Ui::ContactWindow    > m_ui           ;
    std::shared_ptr<QMenu                > m_menu_manager ;
    std::shared_ptr<QAction              > m_action_edit  ;
    std::shared_ptr<QAction              > m_action_remove;
    std::shared_ptr<ContactData::ItemData> m_contact_data ;
    Utils::WidgetTools::LineEdit           m_edit_tool;
    //
    QString m_temp_avatar_id;
    QString m_temp_avatar_path;
    QImage  m_temp_avatar_image;
};

// 为 ShipFlags 激活标准位运算符（必须放在类外或全局作用域）
Q_DECLARE_OPERATORS_FOR_FLAGS(ContactWindow::ShipFlags)
inline ContactWindow::ShipFlags operator~(ContactWindow::ShipFlag flag) {
    return ~ContactWindow::ShipFlags(flag);
}
