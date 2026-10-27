#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class DemoContactDelegateItem;

// 主要为便于工厂调用
class ItemDelegateBase : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    ~ItemDelegateBase() = default;
    virtual const ENUMS::ItemDataType item_type() const = 0;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    };
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegateBase>)  // 确保向 QVariant 提供支持
// 在程序初始化时注册（如果涉及到跨类型转换）
// qRegisterMetaType<std::shared_ptr<ItemDelegate>>();
static int g_register_model_item_provider = qRegisterMetaType<std::shared_ptr<ItemDelegateBase>>();

// 无特殊设置时一个 QAbstractListModel 只会创建一个 ContactDelegate，再轮流调用 paint 对每一行数据进行绘制
// 但也可以：1.为不同的行/列设置了不同的 Delegate; 2.多栏视图（如 QTreeView 或 QTableView）为不同的“列（Column）”设置不同的 Delegate
// *  该类实际为 ItemDelegateFactory
class ItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
public:
    // 用于 ItemDelegate 中对界面的绘制
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    // 用于 ItemDelegate 中返回条目的大小
    // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
protected:
    // 获取点击事件  // false: 未处理，可交由其他对象处理
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
private:
    void register_delegates() const;
private:
    mutable std::map<ENUMS::ItemDataType, std::shared_ptr<ItemDelegateBase>> m_map_delegates;
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegate>)  // 确保向 QVariant 提供支持
// 在程序初始化时注册（如果涉及到跨类型转换）
// qRegisterMetaType<std::shared_ptr<ItemDelegate>>();
// static int g_register_model_item_provider = qRegisterMetaType<std::shared_ptr<ItemDelegate>>();

// 显示为联系人
#include "Database.h"
#include "DemoContactDelegateItem.h"
class ItemDelegateContact : public ItemDelegateBase{
public:
    using ItemDelegateBase::ItemDelegateBase;
    const ENUMS::ItemDataType item_type() const override { return ENUMS::ItemDataType::CONTACT; };
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // return false;

        // 验证数据类型
        {
            if (!index.isValid()) { return; }
            auto var_data = index.data(ENUMS::role_item_data());
            if (!var_data.isValid()) { return; }
            if (!var_data.canConvert<std::shared_ptr<ContactData::ItemData>>()) { return; }
        }

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 绘制背景
        {
            // 绘制背景（处理选中、悬停状态）
            opt.backgroundBrush = QBrush((option.state & QStyle::State_Selected) ? opt.palette.color(QPalette::Highlight) : opt.backgroundBrush);
            opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
        }

        // 底部需要绘制一条分隔线以便用户确认添加了数据
        if (0) {
            auto pen_old = painter->pen();
            painter->setPen(QPen(QColor(Qt::darkGray)));
            painter->drawLine(opt.rect.bottomLeft() + QPoint(10, 0), opt.rect.bottomRight() - QPoint(10, 0));
            painter->setPen(pen_old);
        }

        // 绘制内容
        {
            // 获取样式
            if(!m_demo_contact_delegate_item) {
                m_demo_contact_delegate_item = std::make_shared<DemoContactDelegateItem>();
            }
            if(m_demo_contact_delegate_item->size() != option.rect.size()) {
                // 调整大小
                m_demo_contact_delegate_item->resize(option.rect.size());
                Utils::in_memory_layout_expander(m_demo_contact_delegate_item.get());
            }
            auto style_data = m_demo_contact_delegate_item->get_style_data();
            // qDebug().noquote().nospace() << "item size : " << option.rect.size()
            //     << ", demo size : " << m_demo_contact_delegate_item->size()
            //     << ", nicknam width : " << style_data->rect_nickname.width()
            //     << ", nicknam width : " << style_data->rect_message.width();

            // 获取数据内容
            auto var_data     = index.data(ENUMS::role_item_data());
            auto contact_data = var_data.value<std::shared_ptr<ContactData::ItemData>>();

            auto get_avatar_rounded = [](std::shared_ptr<ContactData::ItemData> contact_data, int radius) -> const QImage {
                // 获取头像  // 圆角头像  // 头像更新后需要这里清除缓存
                struct CacheContactData {
                    std::shared_ptr<ContactData::ItemData> data;
                    int radius = -1;
                    QImage image;
                };
                static std::map<QString, CacheContactData> s_caches;
                if (!contact_data) { return QImage(); }
                auto account_id = contact_data->account_id;
                // clean
                do {
                    if (!s_caches.contains(account_id)) { break; }
                    auto&cache = s_caches[account_id];
                    if (!cache.data) { break; }
                    auto data = cache.data;
                    // 圆角、id、路径不同均需更新
                    if (cache.radius      != radius                    ||
                        data->avatar_id   != contact_data->avatar_id   ||
                        data->avatar_path != contact_data->avatar_path ||
                        data->avatar_image->isNull() ||
                        cache.image.isNull()
                    ) {
                        s_caches.erase(account_id);
                    }
                } while (false);
                // set
                if (!s_caches.contains(account_id)) {
                    auto&cache   = s_caches[account_id];
                    cache.data   = std::make_shared<ContactData::ItemData>(*contact_data);  // 拷贝复制
                    cache.radius = radius;
                    cache.image  = Utils::to_round_image(*cache.data->avatar_image.get(), radius);
                }
                return s_caches[account_id].image;
            };
            auto image_avatar = get_avatar_rounded(contact_data, style_data->rect_avatar.width() / 2);

            // 提示消息
            auto info_text = contact_data->info_text;
            QFontMetrics metrics_info_text(style_data->font_signature);
            QString elided_info_text = metrics_info_text.elidedText(info_text, Qt::ElideRight, style_data->rect_message.width());
            // 时间 // 不要裁剪
            auto info_time = contact_data->info_time;
            QFontMetrics metrics_info_time(style_data->font_time);
            // QString elided_info_time = metrics_info_time.elidedText(info_time, Qt::ElideRight, style_data->rect_message.width());

            // 调整宽度
            QRect rect_nickname_last = style_data->rect_nickname;
            QRect rect_time_last     = style_data->rect_time;

            QRect rect_time = metrics_info_time.boundingRect(info_time);
            int adjusted_width = style_data->rect_time.width() - rect_time.width() - 10;  // 保留一定的间距，防止显示不全
            rect_time_last = rect_time_last.adjusted(adjusted_width, 0, 0, 0);
            rect_nickname_last = rect_nickname_last.adjusted(0, 0, -adjusted_width, 0);
            rect_nickname_last.setRight(rect_time_last.left() - 5);

            // 没有时间信息则调整显示名称的宽度
            if (info_time.isEmpty()) {
                rect_nickname_last.setRight(rect_time_last.right());
                rect_time_last.setLeft(rect_time_last.right());
            }

            // 昵称
            auto display_name = contact_data->display_name;
            QFontMetrics metrics_display_name(style_data->font_nickname);
            QString elided_display_name = metrics_display_name.elidedText(display_name, Qt::ElideRight, rect_nickname_last.width());

            painter->save();
            painter->translate(option.rect.topLeft()); // 将画布原点移到 (x, y)

            // 绘制头像
            painter->drawImage(style_data->rect_avatar, image_avatar);
            // 绘制昵称
            painter->setFont(style_data->font_nickname);
            painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : option.palette.text().color());
            painter->drawText(rect_nickname_last, Qt::AlignLeft | Qt::AlignVCenter, elided_display_name);
            // 绘制签名内容
            painter->setFont(style_data->font_signature);
            painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : QColor(100, 100, 100));
            painter->drawText(style_data->rect_message, Qt::AlignLeft | Qt::AlignVCenter, elided_info_text);
            painter->drawText(rect_time_last, Qt::AlignRight | Qt::AlignVCenter, info_time);

#ifdef _DEBUG
            if (0) {
                // 画出昵称和时间所在矩形
                painter->setPen(QPen(QColor(Qt::darkRed)));
                painter->drawRect(rect_nickname_last);
                painter->setPen(QPen(QColor(Qt::darkCyan)));
                painter->drawRect(rect_time_last);
                painter->setPen(QPen(QColor(Qt::darkMagenta)));
                painter->drawRect(style_data->rect_message);
            }
#endif // _DEBUG

            painter->restore();
        }

        painter->restore();
    }

    // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        do{
            if (!index.isValid()) { break; }

            QVariant variant = index.data(ENUMS::role_item_type());
            if (!variant.isValid() || !variant.canConvert<ENUMS::ItemDataType>()) { break; }

            auto type = variant.value<ENUMS::ItemDataType>();
            if (type != item_type()) { break; }
            if (!m_demo_contact_delegate_item) {
                m_demo_contact_delegate_item = std::make_shared<DemoContactDelegateItem>();
            }
            return QSize(-1, m_demo_contact_delegate_item->size().height());
        } while(false);
        return size;
    }

private:
    double group_height_scale() const { return double(3) / 2; }

private:
    mutable std::shared_ptr<DemoContactDelegateItem> m_demo_contact_delegate_item = nullptr;
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegateContact>)  // 确保向 QVariant 提供支持

// 分组标题
class ItemDelegateGroup : public ItemDelegateBase{
public:
    using ItemDelegateBase::ItemDelegateBase;
    const ENUMS::ItemDataType item_type() const override { return ENUMS::ItemDataType::GROUP; };
    // 用于 ItemDelegate 中对界面的绘制  // 不要使用 QFontMetrics 等，会造成初始化及滑动时极其严重的卡顿。
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // 验证数据类型
        {
            if (!index.isValid()) { return; }
            auto var_data = index.data(ENUMS::role_item_data());
            if (!var_data.isValid()) { return; }
            if (!var_data.canConvert<ENUMS::ItemGroupType>()) { return; }
        }

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 绘制背景
        {
            // 绘制背景（处理选中、悬停状态）
            opt.backgroundBrush = QBrush((option.state & QStyle::State_Selected) ? opt.palette.color(QPalette::Highlight) : opt.backgroundBrush);
            opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
        }

        // 绘制图标
        {
            // 修改图标大小
            int height_item = opt.decorationSize.height();
            int height_icon = (height_item * group_height_scale());
            opt.decorationSize = QSize(height_icon, height_icon);

            // 根据颜色和大小生成图标
            QColor color_icon = opt.palette.color(QPalette::Text);  // 指定固定的颜色
            static Utils::FontIcons icon_font = Utils::FontIcons();
            if(icon_font.size.button != opt.decorationSize || icon_font.color.normal != color_icon) {
                icon_font.size.button = opt.decorationSize;
                icon_font.color.normal = color_icon;
                icon_font.update();
            }
            QIcon ico_show = (option.state & QStyle::State_Open) ? icon_font.icon.expand : icon_font.icon.collapse;

            // qDebug().noquote().nospace() << QString(R"(is_expanded : %1)").arg(is_expanded);

            // 修改图标为水平居中并绘制
            opt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter; // 添加图标水平居中的对齐属性
            QRect rect_icon = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);

            painter->drawPixmap(rect_icon, ico_show.pixmap(opt.decorationSize, QIcon::Normal));
        }

        // 绘制群组和成员数
        {
            // 修改文本
            QVariant var_data = index.data(ENUMS::role_item_data());
            if (var_data.isValid() && var_data.canConvert<ENUMS::ItemGroupType>()) {
                auto data_group_type = var_data.value<ENUMS::ItemGroupType>();
                QString str_group_name = Utils::Transform::to_string(data_group_type);
                opt.text = str_group_name;
            }

// #ifdef _DEBUG
//         opt.text += " aaabbbcccdddeeefffggghhhiiijjjkkklllmmmnnnooopppqqqrrrssstttuuuvvvwwwxxxyyyzzz";
// #endif // _DEBUG

            // 获取总的绘制区域 // 拿到扣除图标、复选框后，真正属于文本的矩形区域
            QRect rect_text = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);

            // 成员数
            int member_count = index.model()->rowCount(index);
            QString str_member_count = QString("(%1)").arg(member_count);
            int width_member_count = opt.fontMetrics.horizontalAdvance(str_member_count);  // + 10;  // 加 10 像素作为右边距
            QRect rect_member_count = rect_text;
            rect_member_count.setLeft(rect_text.right() - width_member_count);  // 右侧数字区域

            // 群组名
            QRect rect_group_name = rect_text;
            rect_group_name.setRight(rect_member_count.left() - 5);  // 与成员数间隔一定的距离
            QString str_group_name_elided = opt.fontMetrics.elidedText(opt.text, opt.textElideMode, rect_group_name.width());  // 获取截断后的文字

            // 设置颜色
            // QColor textColor = opt.palette.color((opt.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text);
            QColor color_text = opt.palette.color(QPalette::Text);  // 指定固定的颜色
            painter->setPen(color_text);  // 自定义文字颜色

            // 绘制
            painter->drawText(rect_group_name, opt.displayAlignment, str_group_name_elided);  // 绘制群组名
            painter->drawText(rect_member_count, Qt::AlignRight | Qt::AlignVCenter, str_member_count);  // 绘制成员数
        }

        painter->restore();
    };

    // 用于 ItemDelegate 中返回条目的大小
    // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override { return QSize(-1, 30); };

private:
    double group_height_scale() const { return double(3) / 2; }
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegateGroup>)  // 确保向 QVariant 提供支持

// 显示为对话
class ItemDelegateChatroom : public ItemDelegateBase{
    using ItemDelegateBase::ItemDelegateBase;
public:
    const ENUMS::ItemDataType item_type() const override { return ENUMS::ItemDataType::CHATROOM; };
    // 用于 ItemDelegate 中对界面的绘制
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // return false;

        // 验证数据类型
        {
            if (!index.isValid()) { return; }
            auto var_data = index.data(ENUMS::role_item_data());
            if (!var_data.isValid()) { return; }
            if (!var_data.canConvert<std::shared_ptr<ChatroomData::ItemData>>()) { return; }
            auto chatroom_data = var_data.value<std::shared_ptr<ChatroomData::ItemData>>();
            if (!chatroom_data) { return; }
            // if (!chatroom_data->contact) { return; }
        }

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 绘制背景
        {
            // 绘制背景（处理选中、悬停状态）
            opt.backgroundBrush = QBrush((option.state & QStyle::State_Selected) ? opt.palette.color(QPalette::Highlight) : opt.backgroundBrush);
            opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
        }

        // 底部需要绘制一条分隔线以便用户确认添加了数据
        if (0) {
            auto pen_old = painter->pen();
            painter->setPen(QPen(QColor(Qt::darkGray)));
            painter->drawLine(opt.rect.bottomLeft() + QPoint(10, 0), opt.rect.bottomRight() - QPoint(10, 0));
            painter->setPen(pen_old);
        }

        // 绘制内容
        {
            // 获取样式
            if(!m_demo_contact_delegate_item) {
                m_demo_contact_delegate_item = std::make_shared<DemoContactDelegateItem>();
            }
            if(m_demo_contact_delegate_item->size() != option.rect.size()) {
                // 调整大小
                m_demo_contact_delegate_item->resize(option.rect.size());
                Utils::in_memory_layout_expander(m_demo_contact_delegate_item.get());
            }
            auto style_data = m_demo_contact_delegate_item->get_style_data();
            // qDebug().noquote().nospace() << "item size : " << option.rect.size()
            //     << ", demo size : " << m_demo_contact_delegate_item->size()
            //     << ", nicknam width : " << style_data->rect_nickname.width()
            //     << ", nicknam width : " << style_data->rect_message.width();

            // 获取数据内容
            auto var_data   = index.data(ENUMS::role_item_data());
            auto chatroom_data = var_data.value<std::shared_ptr<ChatroomData::ItemData>>();

            auto get_avatar_rounded = [](std::shared_ptr<ChatroomData::ItemData> chatroom_data, int radius) -> const QImage {
                // 获取头像  // 圆角头像  // 头像更新后需要这里清除缓存
                struct CacheChatroomData {
                    std::shared_ptr<ChatroomData::ItemData> data;
                    int radius = -1;
                    QImage image;
                };
                static std::map<QString, CacheChatroomData> s_caches;
                if (!chatroom_data) { return QImage(); }
                auto chatroom_id = chatroom_data->chatroom_id;
                // clean
                do {
                    if (!s_caches.contains(chatroom_id)) { break; }
                    auto&cache = s_caches[chatroom_id];
                    if ( cache.data) {
                        auto data = cache.data;
                        // 圆角、id、路径不同均需更新
                        if (cache.radius      != radius                     ||
                            data->avatar_id   != chatroom_data->avatar_id   ||
                            data->avatar_path != chatroom_data->avatar_path ||
                            data->avatar_image->isNull() ||
                            cache.image.isNull()
                        ) {
                            s_caches.erase(chatroom_id);
                        }
                        break;
                    }
                    // 存入的数据无效但新数据有效时需要刷新
                    else if (!cache.data || cache.image.isNull()) {
                        if(!chatroom_data->avatar_image->isNull()) {
                            s_caches.erase(chatroom_id);
                        }
                    }
                } while (false);
                // set
                if (!s_caches.contains(chatroom_id)) {
                    auto&cache   = s_caches[chatroom_id];
                    cache.data   = std::make_shared<ChatroomData::ItemData>(*chatroom_data);  // 拷贝复制
                    cache.radius = radius;
                    cache.image  = Utils::to_round_image(*cache.data->avatar_image.get(), radius);
                }
                return s_caches[chatroom_id].image;
            };
            auto image_avatar = get_avatar_rounded(chatroom_data, style_data->rect_avatar.width() / 2);
            // auto image_avatar = Utils::to_round_image(chatroom_data->avatar_image, style_data->rect_avatar.width() / 2);

            // qDebug().noquote().nospace() << "chatroom_data addr : " << (void*)&chatroom_data
            //     << ", display_name : " << chatroom_data->display_name
            //     << ", avatar_image : " << !chatroom_data->avatar_image.isNull()
            //     << ", round_image : "  << !image_avatar.isNull();

            // 提示消息
            auto last_message_text = chatroom_data->last_message_text;
            QFontMetrics metrics_signature(style_data->font_signature);
            QString elided_last_messsage = metrics_signature.elidedText(last_message_text, Qt::ElideRight, style_data->rect_message.width());
            // 消息时间不应当截断
            // auto datetime = chatroom_data->last_message_datetime;
            // datetime.setTimeZone(QTimeZone::utc());
            // // auto last_message_time = datetime.toLocalTime().toString("hh:mm");
            // auto last_message_time = Utils::format_smart_time(datetime);
            auto last_message_time = chatroom_data->display_datetime;
            QFontMetrics metrics_time(style_data->font_time);

            // 调整宽度
            QRect rect_nickname_last = style_data->rect_nickname;
            QRect rect_time_last     = style_data->rect_time;

            QRect rect_time = metrics_time.boundingRect(last_message_time);
            int adjusted_width = style_data->rect_time.width() - rect_time.width() - 10;  // 保留一定的间距，防止显示不全
            rect_time_last = rect_time_last.adjusted(adjusted_width, 0, 0, 0);
            rect_nickname_last = rect_nickname_last.adjusted(0, 0, -adjusted_width, 0);
            rect_nickname_last.setRight(rect_time_last.left() - 5);

            // 昵称
            auto display_name = chatroom_data->display_name;
            QFontMetrics metrics_display_name(style_data->font_nickname);
            QString elided_display_name = metrics_display_name.elidedText(display_name, Qt::ElideRight, rect_nickname_last.width());

            painter->save();
            painter->translate(option.rect.topLeft()); // 将画布原点移到 (x, y)

            // 绘制头像
            painter->drawImage(style_data->rect_avatar, image_avatar);
            // 绘制昵称
            painter->setFont(style_data->font_nickname);
            painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : option.palette.text().color());
            painter->drawText(rect_nickname_last, Qt::AlignLeft | Qt::AlignVCenter, elided_display_name);
            // 绘制签名内容
            painter->setFont(style_data->font_signature);
            painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : QColor(100, 100, 100));
            painter->drawText(style_data->rect_message, Qt::AlignLeft | Qt::AlignVCenter, elided_last_messsage);
            painter->drawText(rect_time_last, Qt::AlignRight | Qt::AlignVCenter, last_message_time);

#ifdef _DEBUG
            if (0) {
                // 画出昵称和时间所在矩形
                painter->setPen(QPen(QColor(Qt::darkRed)));
                painter->drawRect(rect_nickname_last);
                painter->setPen(QPen(QColor(Qt::darkCyan)));
                painter->drawRect(rect_time_last);
                painter->setPen(QPen(QColor(Qt::darkMagenta)));
                painter->drawRect(style_data->rect_message);
            }
#endif // _DEBUG

            painter->restore();
        }

        painter->restore();
    }
    // 用于 ItemDelegate 中返回条目的大小
    // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        do{
            if (!index.isValid()) { break; }

            QVariant variant = index.data(ENUMS::role_item_type());
            if (!variant.isValid() || !variant.canConvert<ENUMS::ItemDataType>()) { break; }

            auto type = variant.value<ENUMS::ItemDataType>();
            if (type != item_type()) { break; }
            if (!m_demo_contact_delegate_item) {
                m_demo_contact_delegate_item = std::make_shared<DemoContactDelegateItem>();
            }
            return QSize(-1, m_demo_contact_delegate_item->size().height());
        } while(false);
        return size;
    }

private:
    double group_height_scale() const { return double(3) / 2; }

private:
    mutable std::shared_ptr<DemoContactDelegateItem> m_demo_contact_delegate_item = nullptr;
};
Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegateChatroom>)  // 确保向 QVariant 提供支持

// // 显示为对话
// class ItemDelegateMessage : public ItemDelegateBase{
// public:
//     ItemDelegateMessage() {}
//     // 用于 ItemDelegate 中对界面的绘制
//     void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override { ; };
//     // 用于 ItemDelegate 中返回条目的大小
//     // **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
//     QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override { return QSize(-1, 80); };
// };
// Q_DECLARE_METATYPE(std::shared_ptr<ItemDelegateMessage>)  // 确保向 QVariant 提供支持
