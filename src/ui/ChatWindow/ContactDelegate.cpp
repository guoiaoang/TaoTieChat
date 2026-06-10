#include "ContactDelegate.h"
#include <qicon.h>
#include "Utils.h"
#include "DemoContactDelegateItem.h"
#include "ContactWindow.h"  // Database::instance()

ContactDelegate::ContactDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
    m_demo_contact_delegate_item = std::make_shared<DemoContactDelegateItem>();
    set_height(m_demo_contact_delegate_item->size().height());
}

ContactDelegate::~ContactDelegate() {
}

void ContactDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (!index.isValid()) { return; }

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
        // QPixmap avatar = index.data(Qt::UserRole).value<QPixmap>();
        // painter->drawPixmap(rect, avatar);

        // painter->restore();
    }

    // 绘制背景（选中状态）
    painter->fillRect(option.rect, (option.state & QStyle::State_Selected) ? option.palette.highlight() : option.palette.base());

    // 即使没有数据也要绘制界面
    std::shared_ptr<UserData> user_data = std::make_shared<UserData>();
    user_data->contact_info = index.data(Qt::UserRole).value<std::shared_ptr<ContactInfo>>();
    user_data->account_id   = user_data->contact_info ? user_data->contact_info->baseInfo().accountId() : QString{};
    if(user_data->contact_info && !user_data->account_id.isEmpty() && !m_cache_account_data.contains(user_data->account_id)) {
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
        m_cache_account_data.emplace(user_data->account_id, user_data);
    }
    // 防止置空
    user_data = m_cache_account_data.contains(user_data->account_id) ? m_cache_account_data[user_data->account_id] : user_data;

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

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), get_height());
}
