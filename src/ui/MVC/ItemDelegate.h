#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class DemoContactDelegateItem;

// 无特殊设置时一个 QAbstractListModel 只会创建一个 ContactDelegate，再轮流调用 paint 对每一行数据进行绘制
// 但也可以：1.为不同的行/列设置了不同的 Delegate; 2.多栏视图（如 QTreeView 或 QTableView）为不同的“列（Column）”设置不同的 Delegate
class ItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

public:
    // 类型
    virtual const QString type() const { return ""; };
    // 用于 ItemDelegate 中对界面的绘制
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    // 用于 ItemDelegate 中返回条目的大小
    // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    // 获取点击事件  // false: 未处理，可交由其他对象处理
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    bool paint_group(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool paint_contact(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    // 显示群组时高度和图标的缩放倍数
    double group_height_scale() const { return double(3) / 2; }
    mutable std::shared_ptr<DemoContactDelegateItem> m_demo_contact_delegate_item = nullptr;
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegate>)  // 确保向 QVariant 提供支持
// 在程序初始化时注册（如果涉及到跨类型转换）
// qRegisterMetaType<std::shared_ptr<ItemDelegate>>();
static int g_register_model_item_provider = qRegisterMetaType<std::shared_ptr<ItemDelegate>>();

// 显示为联系人
#include "Database.h"
#include "DemoContactDelegateItem.h"
class ItemDelegateContact : public ItemDelegate{
public:
public:
    ItemDelegateContact(const std::shared_ptr<ContactInfo>& contact_info, ENUMS::ItemGroupType group_type = ENUMS::ItemGroupType::NONE) {
        m_demo_contact_delegate_item = std::make_shared<DemoContactDelegateItem>();
        set_height(m_demo_contact_delegate_item->size().height());
        set_contact_info(contact_info);
    }
    const QString type() const override { return "contact"; };

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // if (!index.isValid()) { return; }

        // 调整大小
        m_demo_contact_delegate_item->resize(option.rect.size());
        Utils::in_memory_layout_expander(m_demo_contact_delegate_item.get());
        auto style_data = m_demo_contact_delegate_item->get_style_data();

        // 保存画笔状态
        painter->save();
        painter->translate(option.rect.topLeft()); // 将画布原点移到 (x, y)

        // 将界面绘制到画面上。大量数据时会卡顿，需要使用二级缓存来加速。
        // m_demo_contact_delegate_item->render(painter, QPoint(0, 0));

        // 遮罩绘制
        if(0) {
            // painter->save();
            painter->setRenderHint(QPainter::Antialiasing);

            QRect rect = option.rect.adjusted(5, 5, -5, -5); // 留点边距

            // 创建路径：圆角矩形
            QPainterPath path;
            path.addRoundedRect(rect, 10, 10);

            // 关键：将当前 Painter 的裁剪区域设为这个路径
            painter->setClipPath(path);

            // // 之后画的内容都会被限制在这个圆角矩形里
            // QPixmap avatar = index.data(Qt::UserRole + 1).value<QPixmap>();
            // painter->drawPixmap(rect, avatar);

            // painter->restore();
        }

        // 绘制背景（选中状态）
        painter->fillRect(option.rect, (option.state & QStyle::State_Selected) ? option.palette.highlight() : option.palette.base());

        // 即使没有数据也要绘制界面
        std::shared_ptr<UserData> user_data = std::make_shared<UserData>();
        user_data->contact_info = m_contact_info;
        user_data->account_id   = user_data->contact_info ? user_data->contact_info->baseInfo().accountId() : QString{};
        if (user_data->contact_info && !user_data->account_id.isEmpty()) {
            user_data->user_id   = user_data->contact_info->baseInfo().userId   ();
            user_data->nickname  = user_data->contact_info->baseInfo().nickname ();
            user_data->signature = user_data->contact_info->baseInfo().signature();
            // 更新头像
            if( user_data->avatar_id != user_data->contact_info->baseInfo().avatarId()) {
                user_data->avatar_id  = user_data->contact_info->baseInfo().avatarId();
                auto avatar_file_path = Database::instance().get_local_file_id_path(user_data->avatar_id + ";size:128*128");
                QPixmap pixmap_avatar_source = QPixmap(avatar_file_path);
                user_data->pixmap_avatar = std::make_shared<QPixmap>(Utils::to_round_pixmap(pixmap_avatar_source, style_data->rect_avatar.width() / 2));
            }
            user_data->need_update = false;
            m_cache_user_data = user_data;
        }
        // 防止置空
        user_data = m_cache_user_data;

        // 用户未设置头像时设置默认头像
        if(!user_data->pixmap_avatar) {
            static Utils::FontIcons font_icon =  Utils::FontIcons().update();
            user_data->pixmap_avatar = std::make_shared<QPixmap>(Utils::to_round_pixmap(font_icon.icon.profile.pixmap(style_data->rect_avatar.size()), style_data->rect_avatar.width() / 2));
        }

        // 绘制头像
        painter->drawPixmap(style_data->rect_avatar, *user_data->pixmap_avatar.get());  // 圆角头像

        // 绘制昵称
        {
            painter->setFont(style_data->font_nickname);
            painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : option.palette.text().color());

            // 使用 QFontMetrics 计算省略后的文本，绘制处理后的文本  // TODO:缓存数据
            QFontMetrics metrics_nickname(style_data->font_nickname);
            QString elided_nickname = metrics_nickname.elidedText(user_data->nickname, Qt::ElideRight, style_data->rect_nickname.width());
            painter->drawText(style_data->rect_nickname, Qt::AlignLeft | Qt::AlignVCenter, elided_nickname);
        }

        // 绘制签名内容  // 单行不换行，显示不了的用省略号结尾
        {
            painter->setFont(style_data->font_signature);
            painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : QColor(100, 100, 100));

            // 使用 QFontMetrics 计算省略后的文本，绘制处理后的文本  // TODO:缓存数据
            QFontMetrics metrics_signature(style_data->font_signature);
            QString elided_signature = metrics_signature.elidedText(user_data->signature, Qt::ElideRight, style_data->rect_message.width());
            painter->drawText(style_data->rect_message, Qt::AlignLeft | Qt::AlignVCenter, elided_signature);
        }

        painter->restore();
    }

    // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(-1, get_height());
    }

public:
    void set_contact_info(const std::shared_ptr<ContactInfo>& contact_info) {
        if(m_contact_info) { m_contact_info.reset(); }
        // 这里保存拷贝，防止外部修改影响内部
        m_contact_info = std::make_shared<ContactInfo>(*contact_info);
    }
    const std::shared_ptr<ContactInfo>& get_contact_info() const {
        return m_contact_info;
    }
    // 判断所属分组
    bool is_member(ENUMS::ItemGroupType group_type) { return m_cache_user_data && group_type == m_cache_user_data->group_type; }

private:
    // 无法在绘图函数中传出参数，需要预先计算高度
    void set_height(int height) { m_height = height; }
    int get_height() const { return m_height; }

private:
    std::shared_ptr<ContactInfo> m_contact_info;
    int m_height = 80;
    // 缓存动态的数据内容  // TODO:缓存内容的更新使用专属的接口
    struct UserData{
        bool need_update = true; // 是否需要更新数据
        std::shared_ptr<ContactInfo> contact_info;
        QString account_id;
        QString user_id;
        QString avatar_id;
        QString nickname;
        QString signature;
        QString message;
        std::shared_ptr<QPixmap> pixmap_avatar;
        ENUMS::ItemGroupType group_type;  // 分组信息
    };
    // 加上 mutable，允许在 const 函数（如 paint）中修改缓存
    mutable std::shared_ptr<UserData> m_cache_user_data;
    std::shared_ptr<DemoContactDelegateItem> m_demo_contact_delegate_item;
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegateContact>)  // 确保向 QVariant 提供支持

// 分组标题
class ItemDelegateSpacer : public ItemDelegate{
private:
    // 只需要初始化一次的共享资源
    class Res{
        using FontIcons = Utils::FontIcons;
        friend ItemDelegateSpacer;
    public:
        Res() {
            m_font_icon       = FontIcons().update();
            m_size_collapse   = QSize(128, 128);  // 默认大小，绘制时会进行缩放
            m_pixmap_collapse = m_font_icon.icon.collapse.pixmap(m_size_collapse);
            m_pixmap_expand   = m_font_icon.icon.expand  .pixmap(m_size_collapse);;
        }
    private:
        QSize     m_size_collapse   ;
        QPixmap   m_pixmap_collapse ;
        QPixmap   m_pixmap_expand   ;
        FontIcons m_font_icon       ;
    };
    static Res&res() {
        static Res res;
        return res;
    }
private:
    // 不同实例中外部可修改的数据成员
    struct Data{
        QString group_name;
    // private:
        bool    collapsed     = true;  // 展开状态
        int     members_count = 0;
    };
    mutable Data m_data;
private:
    // 不同实例中绘图时缓存的资源
    struct Cache {
        bool    updated = false;  // 标记为视图需要更新  // 由外部触发更新
        // collapse
        int     item_width = -1;  // 控件宽度
        QRect   rect_collapse;
        QPixmap pixmap_collapse;
        bool    draw_collapse = false;
        // count
        int     members_count = -1;  // 分组成员计数缓存
        QRect   rect_count;
        QFont   font_count;
        bool    draw_count = false;
        // group name
        QRect   rect_group_name  ;  // 绘制区域
        QString elided_group_name; // 要显示的文字
        bool    draw_group_name = false;
        // 计算是否需要更新
        bool need_update(int item_width, int members_count) {
            return updated || item_width != this->item_width || members_count != this->members_count;
        }
    };
    mutable Cache m_cache;
public:
    ItemDelegateSpacer(QString group_name = QString{}) : m_data{ .group_name = group_name } { }
    ItemDelegateSpacer::Data& data() { return m_data; }
    void collapse() { m_data.collapsed = true ; m_cache.updated = true; }  // 修改数据并标记视图为需要更新
    void expand  () { m_data.collapsed = false; m_cache.updated = true; }  // 修改数据并标记视图为需要更新
    bool is_collapsed() const { return  m_data.collapsed; }
    bool is_expand   () const { return !m_data.collapsed; }
public:
    // 类型
    const QString type() const override { return "spacer"; }
    // 用于 ItemDelegate 中对界面的绘制  // 不要使用 QFontMetrics 等，会造成初始化及滑动时极其严重的卡顿。
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // 保存画笔状态
        painter->save();
        // 设置绘制的坐标原点，使用当前矩形的坐标系
        painter->translate(option.rect.topLeft()); // 将画布原点移到 (x, y)

        // 画内容
        {
            int item_width  = option.rect.width ();
            int item_height = option.rect.height();
            bool need_update = m_cache.need_update(item_width, m_data.members_count);

            // 调试时绘制背景及边框
#ifdef _DEBUG
            if(1) {
                painter->setBrush(Qt::darkCyan);  // 背景颜色
                painter->setPen(Qt::darkMagenta);  // 边框或字体颜色
                painter->drawRect(0, 0, item_width - 1, item_height);  // `item_width - 1` 防止绘制到矩形外而观察不到
            }
#endif // __DEBUG

            // ~~绘制背景（选中状态）~~  不需要修改背景
            // painter->fillRect(option.rect, (option.state & QStyle::State_Selected) ? option.palette.highlight() : option.palette.base());

            // 绘制收起/展开状态按钮
            if(need_update) {
                int collapse_margin_top      = 3;
                int collapse_margin_bottom   = 3;
                int collapse_margin_left     = 0;
                // int collapse_margin_right = 3;
                m_cache.pixmap_collapse = m_data.collapsed ? res().m_pixmap_collapse : res().m_pixmap_expand;
                int wh = item_height - (collapse_margin_top + collapse_margin_bottom);  // 缩放到高度减去上下各 3 像素的间距
                m_cache.pixmap_collapse = m_cache.pixmap_collapse.scaled(QSize(wh, wh), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                // 图片要传入原始大小的矩形，否则默认拉伸
                int collapse_width  = m_cache.pixmap_collapse.width();
                int collapse_height = m_cache.pixmap_collapse.height();
                m_cache.rect_collapse = QRect(collapse_margin_left, (item_height - collapse_height) / 2, collapse_width, collapse_height);

                // 足以显示最右侧及最底部时才绘制
                m_cache.draw_collapse = item_width >= m_cache.rect_collapse.right() && item_height >= m_cache.rect_collapse.bottom();

                qDebug() << "group name : " << m_data.group_name << ", collapsed : " << m_data.collapsed;
            }
            if(m_cache.draw_collapse) {
                painter->drawPixmap(m_cache.rect_collapse, m_cache.pixmap_collapse);
            }

            // 绘制计数
            if(need_update) {
                // 设置字体
                m_cache.font_count = option.font;
                m_cache.members_count = m_data.members_count;

                // 获取宽度
                int count_width = 0;
                static bool dynamic_calc_count = true;  // 实测动态计算对性能的影响与固定值几乎无差别
                if(dynamic_calc_count) {
                    m_cache.font_count.setKerning(false);  // 关闭字距调整，提升测算速度
                    QFontMetrics metrics_count(m_cache.font_count);
                    count_width = metrics_count.horizontalAdvance(QString::number(m_cache.members_count)); // 用 horizontalAdvance 代替 boundingRect，效率更高
                }
                else {
                    count_width = 30;  // 可容纳 `9999` 的显示
                }

                // 计算绘制区域
                int count_margin_right = 3;
                int count_x = item_width - count_width - count_margin_right;
                m_cache.rect_count = QRect(count_x, 0, count_width, item_height);

                // 足以显示最底部，及最左侧与其他控件不重叠时才显示
                m_cache.draw_count = m_cache.rect_collapse.right() <= m_cache.rect_count.left() && item_height >= m_cache.rect_count.height();
            }
            if(m_cache.draw_count) {
                painter->setFont(m_cache.font_count);
                painter->drawText(m_cache.rect_count, Qt::AlignRight | Qt::AlignVCenter, QString::number(m_cache.members_count));  // 右侧对齐
            }

            // 绘制群组名称
            if(need_update) {
                int group_name_margin_left  = 0;
                int group_name_margin_right = 6;
                int name_x = m_cache.rect_collapse.right() + group_name_margin_left;  // 图标往右挪 3 像素
                int name_width = m_cache.rect_count.x() - name_x - group_name_margin_right;      // 动态截断宽度，确保字长时不会盖住右边的数字
                m_cache.rect_group_name = QRect(name_x, 0, name_width, item_height);  // 要绘制到的区域

                // 动态计算太卡了，不要使用这种方法
                static bool dynamic_calc_group_name = false;
                if(dynamic_calc_group_name) {
                    // 使用 QFontMetrics 计算省略后的文本，绘制处理后的文本
                    QFontMetrics metrics_group_name(m_data.group_name);
                    m_cache.elided_group_name = metrics_group_name.elidedText(m_data.group_name, Qt::ElideRight, m_cache.rect_group_name.width());
                    // 群组名位于中间，需要有空间进行显示  // 滑动时太卡了
                    m_cache.draw_group_name = m_cache.rect_collapse.right() + group_name_margin_left <= m_cache.rect_count.left() - group_name_margin_right && item_height >= m_cache.rect_group_name.height();
                }
                else {
                    m_cache.elided_group_name = m_data.group_name;  // 要绘制的文字 ...
                    m_cache.draw_group_name = name_width > 0;  // 是否可进行绘制
                }
            }
            if(m_cache.draw_group_name) {
                painter->drawText(m_cache.rect_group_name, Qt::AlignLeft | Qt::AlignVCenter, m_cache.elided_group_name);
            }

            // 保存缓存关键词
            if(need_update) {
                m_cache.updated      = false;
                m_cache.item_width   = item_width;
                m_data.members_count = m_cache.members_count;
            }
        }

        painter->restore();  // 同步 translate 函数的启用状态
    };
    // 用于 ItemDelegate 中返回条目的大小
    // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override { return QSize(-1, 30); };
protected:
    // 获取点击事件  // false: 未处理，可交由其他对象处理
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        // 只处理鼠标左键释放（或者按下）事件
        if (event->type() != QEvent::MouseButtonRelease && event->type() != QEvent::MouseButtonDblClick) { return false; }
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton) { return false; }
        // 从 model 里直接把当前的智能指针捞出来（不要用 this 裸指针！）
        auto provider = index.data(Qt::UserRole + 1).value<std::shared_ptr<ItemDelegate>>();
        auto spacer = std::dynamic_pointer_cast<ItemDelegateSpacer>(provider);
        if (!spacer) { return false; }
        // 是否启用仅支持点击展开按钮  // 不必要求仅点击在展开图标上
        static const bool collapse_button_only = false;
        if(collapse_button_only) {
            // 将鼠标的全局/视口坐标，转换为当前 Item 的本地坐标
            QPoint local_pos = mouseEvent->pos() - option.rect.topLeft();
            // 判断是否点中了图标
            if(m_cache.rect_collapse.contains(local_pos)) {
                return false;
            }
        }
        // 切换折叠/展开状态
        spacer->is_expand() ? spacer->collapse() : spacer->expand();
        // 正确地将带有修改后状态的智能指针同步回 Model
        model->setData(index, QVariant::fromValue(provider), Qt::UserRole + 1);
        // emit model->dataChanged(index, index, { Qt::DisplayRole, Qt::UserRole + 1 });  // 通知数据被更新
        return true; // 事件已处理，不再向下传递
    }
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegateSpacer>)  // 确保向 QVariant 提供支持

// 显示为对话
class ItemDelegateChatroom : public ItemDelegate{
public:
    ItemDelegateChatroom() {}
    // 类型
    const QString type() const override { return "chatroom"; }
    // 用于 ItemDelegate 中对界面的绘制
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override { ; };
    // 用于 ItemDelegate 中返回条目的大小
    // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override { return QSize(-1, 80); };
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegateChatroom>)  // 确保向 QVariant 提供支持
