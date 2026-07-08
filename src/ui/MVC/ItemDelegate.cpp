#include "ItemDelegate.h"
#include "ItemModel.h"

bool ItemDelegate::paint_group(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // return false;

    // 验证数据类型
    {
        if (!index.isValid()) { return false; }
        auto var_data = index.data(ENUMS::role_item_data());
        if (!var_data.isValid()) { return false; }
        if (!var_data.canConvert<ENUMS::ItemGroupType>()) { return false; }
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
#ifdef _DEBUG
        opt.text += " aaabbbcccdddeeefffggghhhiiijjjkkklllmmmnnnooopppqqqrrrssstttuuuvvvwwwxxxyyyzzz";
#endif // _DEBUG

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

    return true;
};

bool ItemDelegate::paint_contact(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // return false;

    // 验证数据类型
    {
        if (!index.isValid()) { return false; }
        auto var_data = index.data(ENUMS::role_item_data());
        if (!var_data.isValid()) { return false; }
        if (!var_data.canConvert<std::shared_ptr<ContactInfo>>()) { return false; }
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
        auto contact_info = var_data.value<std::shared_ptr<ContactInfo>>();

        // 获取头像  // 圆角头像
        static std::map<QString, QPixmap> map_cacha_avatar;
        auto avatar_id  = contact_info->baseInfo().avatarId();
        if (!map_cacha_avatar.contains(avatar_id)) {
            auto avatar_file_path = Database::instance().get_local_file_id_path(avatar_id + ";size:128*128");
            auto pixmap_avatar_source = QPixmap(avatar_file_path);
            if(pixmap_avatar_source.isNull()) {
                // 修改图标大小
                int height_item = opt.decorationSize.height();
                int height_icon = (height_item * group_height_scale());
                opt.decorationSize = QSize(height_icon, height_icon);

                // 根据颜色和大小生成图标
                QColor color_icon = opt.palette.color(QPalette::Text);  // 指定固定的颜色
                static Utils::FontIcons icon_font = Utils::FontIcons();
                if (icon_font.size.avatar != opt.decorationSize || icon_font.color.normal != color_icon) {
                    icon_font.size.avatar = opt.decorationSize;
                    icon_font.color.normal = color_icon;
                    icon_font.update();
                }

                QIcon&ico_show = icon_font.icon.contacts;
                pixmap_avatar_source = QPixmap(ico_show.pixmap(opt.decorationSize, QIcon::Normal));
            }
            auto pixmap_avatar = Utils::to_round_pixmap(pixmap_avatar_source, style_data->rect_avatar.width() / 2);
            map_cacha_avatar[avatar_id] = pixmap_avatar;
        }
        // 头像
        auto pixmap_avatar = map_cacha_avatar[avatar_id];
        // 昵称
        auto nickname = contact_info->baseInfo().nickname() + " aaabbbcccdddeeefffggghhhiiijjjkkklllmmmnnnooopppqqqrrrssstttuuuvvvwwwxxxyyyzzz";
        QFontMetrics metrics_nickname(style_data->font_nickname);
        QString elided_nickname = metrics_nickname.elidedText(nickname, Qt::ElideRight, style_data->rect_nickname.width());
        // 签名
        auto signature = contact_info->baseInfo().signature() + " aaabbbcccdddeeefffggghhhiiijjjkkklllmmmnnnooopppqqqrrrssstttuuuvvvwwwxxxyyyzzz";
        QFontMetrics metrics_signature(style_data->font_signature);
        QString elided_signature = metrics_signature.elidedText(signature, Qt::ElideRight, style_data->rect_message.width());

        painter->save();
        painter->translate(option.rect.topLeft()); // 将画布原点移到 (x, y)

        // 绘制头像
        painter->drawPixmap(style_data->rect_avatar, pixmap_avatar);
        // 绘制昵称
        painter->setFont(style_data->font_nickname);
        painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : option.palette.text().color());
        painter->drawText(style_data->rect_nickname, Qt::AlignLeft | Qt::AlignVCenter, elided_nickname);
        // 绘制签名内容
        painter->setFont(style_data->font_signature);
        painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : QColor(100, 100, 100));
        painter->drawText(style_data->rect_message, Qt::AlignLeft | Qt::AlignVCenter, elided_signature);

        painter->restore();
    }

    painter->restore();
    return true;
};

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    do{
        if (!index.isValid()) { break; }

        QVariant var_type = index.data(ENUMS::role_item_type());
        if (!var_type.isValid() || !var_type.canConvert<ENUMS::ItemDataType>()) { break; }

        bool is_processed = false;
        auto data_item_type = var_type.value<ENUMS::ItemDataType>();
        if (data_item_type == ENUMS::ItemDataType::GROUP) {
            is_processed = paint_group(painter, option, index);
        }
        else if (data_item_type == ENUMS::ItemDataType::CONTACT) {
            is_processed = paint_contact(painter, option, index);
        }
        if (is_processed) { return; }
    } while(false);
    QStyledItemDelegate::paint(painter, option, index);
}

// **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    do{
        if (!index.isValid()) { break; }

        QVariant var_type = index.data(ENUMS::role_item_type());
        if (!var_type.isValid() || !var_type.canConvert<ENUMS::ItemDataType>()) { break; }

        auto data_item_type = var_type.value<ENUMS::ItemDataType>();
        if (data_item_type == ENUMS::ItemDataType::GROUP) {
            // size = sizeHint_group(option, index);
            size.setHeight(size.height() * group_height_scale());
        }
        else if (data_item_type == ENUMS::ItemDataType::CONTACT) {
            if(!m_demo_contact_delegate_item) {
                m_demo_contact_delegate_item = std::make_shared<DemoContactDelegateItem>();
            }
            return QSize(-1, m_demo_contact_delegate_item->size().height());
            // size = sizeHint_contact(option, index);
        }
        if (size != QSize()) { return size; }
    } while(false);
    // return QSize(option.rect.width(), 80);
    return QStyledItemDelegate::sizeHint(option, index);
}

bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    do{
        if (!index.isValid()) { break; }

        QVariant var_type = index.data(ENUMS::role_item_type());
        if (!var_type.isValid() || !var_type.canConvert<ENUMS::ItemDataType>()) { break; }

        bool is_processed = false;
        auto data_item_type = var_type.value<ENUMS::ItemDataType>();
        if (data_item_type == ENUMS::ItemDataType::GROUP) {
            // is_processed = paint_group(painter, option, index);
            is_processed = false;  // 会影响展开、折叠功能
        }
        else if (data_item_type == ENUMS::ItemDataType::CONTACT) {
            // is_processed = paint_contact(painter, option, index);
        }
        if (is_processed) { return is_processed; }
    } while(false);
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
