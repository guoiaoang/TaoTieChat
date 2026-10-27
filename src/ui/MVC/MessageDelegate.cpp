#include "MessageDelegate.h"

#include <algorithm>

#include <QAbstractItemView>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

#include "ItemModel.h"
#include "MessageDelegate.h"
#include "Accounts.h"
#include "SignalInner.h"

namespace {

// ---------- 布局常量（像素） ----------
constexpr int kListMargin     = 8;   // 列表左右外边距
constexpr int kRowVMargin     = 6;   // 行上下留白（代替 QListView::spacing）
constexpr int kMetaLineHeight = 18;  // 发送者/时间行高
constexpr int kAvatarSize     = 36;  // 头像宽度
constexpr int kAvatarSpacing  = 10;  // 头像与气泡间距
constexpr int kBubbleHPad     = 12;  // 气泡水平内边距
constexpr int kBubbleVPad     = 9;   // 气泡垂直内边距
constexpr int kBubbleRadius   = 9;   // 气泡圆角
constexpr int kMaxBubblePct   = 72;  // 气泡最大文本宽度占可用宽度的百分比  // TODO:不使用百分百绘制
constexpr int kMinTextWidth   = 0;   // 文本最小宽度，非气泡宽度
constexpr int kDragThreshold  = 4;   // 超过此位移视为拖动（不触发链接点击）

// ---------- 图片/文件消息常量 ----------
constexpr int kMaxImageHeight = 280; // 图片气泡最大逻辑高度（更大的图等比缩小）
constexpr int kFileCardHeight = 56;  // 文件卡片固定高度
constexpr int kFileIconSize   = 28;  // 文件图标边长
constexpr int kFileIconGap    = 10;  // 图标与文字间距

/// 视口有效宽度（sizeHint 与 paint 必须使用同一来源，否则气泡会画歪）
int viewportWidth(const QStyleOptionViewItem &option) {
    if (const auto *view = qobject_cast<const QAbstractItemView *>(option.widget)) {
        return view->viewport()->width();
    }
    return option.rect.isValid() ? option.rect.width() : 640;
}

// 判断是否是支持的数据类型
bool check_is_message_context(const QModelIndex &index) {
    if (!index.isValid()) { return false; }
    const QVariant variant_data = index.data(ENUMS::role_item_data());
    if (!variant_data.isValid() || !variant_data.canConvert<std::shared_ptr<MessageData::ItemData>>()) { return false; }
    auto message_data = variant_data.value<std::shared_ptr<MessageData::ItemData>>();
    if (!message_data) { return false; }
    // if (!message_data->context) { return false; }
    return true;
}

} // namespace

MessageDelegate::MessageDelegate(QObject *parent) : ItemDelegateBase(parent) {
    m_theme = MessageData::ChatTheme::light();
}

MessageDelegate::~MessageDelegate() {
    clearCache();
}

void MessageDelegate::clearCache() {
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
        delete it->doc;   // pixmap 自动析构；doc 可能为 nullptr
    }
    m_cache.clear();
    resetSelection(); // 行号即将失效，选区一并清除
}

// 生成文本消息
QTextDocument *MessageDelegate::createDocument(const QModelIndex &index, const QFont &font) const {
    auto *doc = new QTextDocument;
    doc->setDocumentMargin(0);  // 边距由气泡矩形自己控制
    doc->setDefaultFont(font);
    do {
        if (!check_is_message_context(index)) { break; }

        const auto item_data = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        // QString text = item_data->media_type == MessageData::MediaType::TEXT && item_data->text ? item_data->text->text : "";
        // // QString short_text = message_context->hasShortText() ? message_context->shortText() : QString();
        // // 如果是系统消息，判断是否自动显示格式化的时间
        // if (message_context->messageType() == MessageType::SystemMessage
        //     && message_context->clientMessageId().startsWith("timestamp_", Qt::CaseSensitivity::CaseInsensitive)
        //     && short_text.isEmpty()
        //     && message_context->hasTimestamp()
        //     && message_context->timestamp().hasTimestamp()
        //     && message_context->timestamp().timestamp().toDateTime().isValid()
        // ) {
        //     short_text = Utils::format_smart_time(message_context->timestamp().timestamp().toDateTime());
        // }

        QString text;
        // 系统的时间戳消息
        if (item_data->sender_type == MessageData::SenderType::SYSTEM && item_data->media_type == MessageData::MediaType::TIMESTAMP) {
            text = Utils::format_smart_time(item_data->message_time);
        }
        // 系统/用户的纯文本消息
        else if (item_data->media_type == MessageData::MediaType::TEXT && item_data->text) {
            text= item_data->text->text;
        }

        doc->setPlainText(text);
    } while (false);
    return doc;
}

// 缓存管理
QTextDocument *MessageDelegate::acquireDocument(const QModelIndex &index, qreal maxTextWidth, const QFont &font) const {
    const int row = index.row();
    auto it = m_cache.find(row);

    if (it == m_cache.end()) {
        // 首次访问：解析 HTML 并测出自然宽度（昂贵，但每个条目只做一次）
        DocEntry entry;
        entry.doc = createDocument(index, font);
        entry.naturalWidth = entry.doc->idealWidth();
        entry.textWidth = -1;
        entry.lastUsed = ++m_generation;
        ++m_createCount;
        it = m_cache.insert(row, entry);
    } else {
        // 命中缓存：刷新 LRU 代际号
        it->lastUsed = ++m_generation;
    }

    DocEntry &e = *it;

    // 目标宽度 = min(自然宽度, 可用最大宽度)：
    // 短消息气泡自动收窄，长消息按上限换行
    const qreal targetWidth = qMax<qreal>(kMinTextWidth, qMin(e.naturalWidth, maxTextWidth));

    if (qAbs(e.textWidth - targetWidth) > 0.5) {
        // 设置任意位置换行
        QTextOption option = e.doc->defaultTextOption();
        option.setWrapMode(QTextOption::WrapAnywhere);
        e.doc->setDefaultTextOption(option);
        // 宽度变化（窗口 resize / 首次布局）：只重排版，不重新解析 HTML
        e.doc->setTextWidth(targetWidth);
        e.textWidth = targetWidth;
        ++m_layoutCount;
    }

    trimCache();
    return e.doc;
}

// 生成图片消息
const QImage *MessageDelegate::acquireImage(const QModelIndex &index, qreal maxW, qreal maxH) const {
    const int row = index.row();
    if (!m_cache.contains(row)) {
        DocEntry entry;
        entry.lastUsed = ++m_generation;
        m_cache[row] = entry;
    }
    auto& entry = m_cache[row];
    entry.lastUsed = ++m_generation;

    do {
        if (!check_is_message_context(index)) { break; }
        auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        // auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        if (!is_image) { break; }
        const auto& image = message_data->image->image;
        if (image->bytesPerLine()) { return nullptr; }

        // 等比缩放到 (maxW, maxH) 以内；小图不放大
        const qreal scale   = qMin<qreal>(1.0, qMin(maxW / image->width(), maxH / image->height()));
        const qreal targetW = qMax<qreal>(1.0, image->width () * scale);
        const qreal targetH = qMax<qreal>(1.0, image->height() * scale);

        if (entry.image.isNull() || qAbs(entry.imageWidth - targetW) > 0.5) {
            // 宽度变化（窗口 resize / 首次布局）：按设备像素比重缩放，保证高 DPI 清晰
            const qreal dpr = qApp->devicePixelRatio();
            entry.image = image->scaled(qRound(targetW * dpr), qRound(targetH * dpr), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            entry.image.setDevicePixelRatio(dpr);
            entry.imageWidth = targetW;
            ++m_layoutCount; // 与文档重排版同口径计入统计
        }

        trimCache();
    } while (false);
    return &entry.image;
}

void MessageDelegate::trimCache() const {
    if (m_cache.size() <= kMaxCachedDocs) { return; }

    while (m_cache.size() > kMaxCachedDocs) {
        auto oldest = std::min_element(m_cache.begin(), m_cache.end(), [](const DocEntry &a, const DocEntry &b) { return a.lastUsed < b.lastUsed; });
        delete oldest->doc;
        m_cache.erase(oldest);
    }
}

// ---------------------------------------------------------------------------
// 几何计算（paint / sizeHint / editorEvent 三处共用，保证一致）
// ---------------------------------------------------------------------------

std::shared_ptr<MessageDelegate::ItemGeometry> MessageDelegate::itemGeometry(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    std::shared_ptr<ItemGeometry> item_geometry = std::make_shared<ItemGeometry>();
    do {
        if (!check_is_message_context(index)) { break; }
        auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        // auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        item_geometry->viewport_width = viewportWidth(option);
        // 系统消息：整行可用宽度，居中绘制
        if (is_system) {
            const qreal max_width = item_geometry->viewport_width - 2 * kListMargin;
            item_geometry->text_document = acquireDocument(index, max_width, option.font);
            const qreal width = item_geometry->text_document->size().width();
            int origin_x = option.rect.left() + (item_geometry->viewport_width - width) / 2.0;
            int origin_y = option.rect.top() + kRowVMargin;
            item_geometry->text_document_origin = QPointF(origin_x, origin_y);
            break;
        }

        // 去掉两头头像宽度后中间的宽度。
        // 头像位置对聊天类消息（文本/图片/文件）一致
        // const int availW = item_geometry->viewport_width - 2 * kListMargin - kAvatarSize - kAvatarSpacing;
        const int max_width = item_geometry->viewport_width - (kListMargin + kAvatarSize + kAvatarSpacing + kBubbleHPad) * 2;
        // 计算气泡绘制宽度
        // const qreal maxBubbleW = qMax<qreal>(kMinTextWidth, availW * kMaxBubblePct / 100.0);
        const qreal max_bubble_width = qMax<qreal>(kMinTextWidth, max_width);
        const qreal meta_top = option.rect.top() + kRowVMargin;
        // 非群组时不需要显示昵称
        bool per_message_show_nickname = false;
        auto context_offset_y = per_message_show_nickname ? kMetaLineHeight : 0;
        int rect_avatar_x = message_data->is_me
            ? item_geometry->viewport_width - kListMargin - kAvatarSize
            : option.rect.left() + kListMargin;
        int rect_avatar_y = meta_top + context_offset_y;
        item_geometry->rect_avatar = QRectF(rect_avatar_x, rect_avatar_y, kAvatarSize, kAvatarSize);

        // 文本消息
        if (is_text) {
            // 普通文本消息
            item_geometry->text_document = acquireDocument(index, max_bubble_width, option.font);
            const qreal text_width    = item_geometry->text_document->size().width ();   // == 排版宽度（已含收窄）
            const qreal text_height   = item_geometry->text_document->size().height();
            const qreal bubble_width  = text_width  + 2 * kBubbleHPad;
            const qreal bubble_height = text_height + 2 * kBubbleVPad;
            int bubble_x = message_data->is_me
                ? item_geometry->rect_avatar.left () - kAvatarSpacing - bubble_width
                : item_geometry->rect_avatar.right() + kAvatarSpacing;
            int bubble_y = item_geometry->rect_avatar.top();
            item_geometry->rect_bubble = QRectF(bubble_x, bubble_y, bubble_width, bubble_height);
            int origin_x = bubble_x + kBubbleHPad;
            int origin_y = bubble_y + kBubbleVPad;
            item_geometry->text_document_origin = QPointF(origin_x, origin_y);
            break;
        }

        // 图片消息
        else if (is_image) {
            // 图片消息：缩放位图 + 气泡内边距
            item_geometry->scaled_image = acquireImage(index, max_bubble_width, kMaxImageHeight);
            const qreal dpr           = item_geometry->scaled_image ? item_geometry->scaled_image->devicePixelRatio() : 1.0;
            const qreal image_width   = item_geometry->scaled_image ? item_geometry->scaled_image->width () / dpr     : kMinTextWidth;
            const qreal image_height  = item_geometry->scaled_image ? item_geometry->scaled_image->height() / dpr     : 0;
            const qreal bubble_width  = image_width  + 2 * kBubbleHPad;
            const qreal bubble_height = image_height + 2 * kBubbleVPad;
            int bubble_x = message_data->is_me
                ? item_geometry->rect_avatar.left () - kAvatarSpacing - bubble_width
                : item_geometry->rect_avatar.right() + kAvatarSpacing;
            int bubble_y = item_geometry->rect_avatar.top();
            int origin_x = bubble_x + kBubbleHPad;
            int origin_y = bubble_y + kBubbleVPad;
            item_geometry->rect_bubble = QRectF(bubble_x, bubble_y, bubble_width, bubble_height);
            item_geometry->text_document_origin = QPointF(origin_x, origin_y);
            break;
        }

        // 文件消息
        else if (is_file) {
            // 文件消息：固定高度卡片，宽度随文件名自适应（超长省略号）
            const QFontMetrics fm(option.font);
            const QString& name = message_data->file->file_name;
            const int name_space = int(max_bubble_width) - 2 * kBubbleHPad - kFileIconSize - kFileIconGap;
            item_geometry->fileName = fm.elidedText(name, Qt::ElideMiddle, qMax(name_space, 24));
            const qreal name_width = fm.horizontalAdvance(item_geometry->fileName);
            const qreal bubble_width = qMax<qreal>(kMinTextWidth, qMin<qreal>(2 * kBubbleHPad + kFileIconSize + kFileIconGap + name_width, max_bubble_width));
            int bubble_x = message_data->is_me
                ? item_geometry->rect_avatar.left () - kAvatarSpacing - bubble_width
                : item_geometry->rect_avatar.right() + kAvatarSpacing;
            int bubble_y = item_geometry->rect_avatar.top();
            // int origin_x = bubble_x + kBubbleHPad;
            // int origin_y = bubble_y + kBubbleVPad;
            item_geometry->rect_bubble = QRectF(bubble_x, bubble_y, bubble_width, kFileCardHeight);
            item_geometry->fileSize = -1;
            break;
        }
    } while (false);
    return item_geometry;
}

// ---------------------------------------------------------------------------
// QStyledItemDelegate 接口
// ---------------------------------------------------------------------------

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // 关键时序：sizeHint 先于 paint 被调用。
    // 这里顺便完成文档/图片的创建与排版并写入缓存，paint 阶段直接复用。
    do {
        if (!check_is_message_context(index)) { break; }
        auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        if (is_system) {
            const qreal text_document_height = item_geometry->text_document->size().height();
            return QSize(item_geometry->viewport_width, int(kRowVMargin * 2 + text_document_height + 0.5));
        }

        // 非群组时不需要显示昵称
        bool per_message_show_nickname = false;
        auto context_offset_y = per_message_show_nickname ? kMetaLineHeight : 0;
        const qreal body_height = qMax<qreal>(item_geometry->rect_bubble.height(), qreal(kAvatarSize));
        return QSize(item_geometry->viewport_width, int(kRowVMargin * 2 + context_offset_y + body_height + 0.5));
    } while(false);
    return ItemDelegateBase::sizeHint(option, index);
}

void MessageDelegate::paintDocument(QPainter *painter, const QModelIndex &index,
        std::shared_ptr<MessageData::ItemData> message_data,
        std::shared_ptr<ItemGeometry         > item_geometry) const {
    do {
        // if (!check_is_message_context(index)) { break; }
        // auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        // auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        // 主题色通过 PaintContext 调色板注入——这是文档能保持主题无关的关键
        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.palette.setColor(QPalette::Text, is_system ? m_theme.systemText : m_theme.text);
        ctx.palette.setColor(QPalette::Link, m_theme.link);
        ctx.palette.setColor(QPalette::LinkVisited, m_theme.link);

        // 文字选区：只对当前选中的那一行生效
        if (!is_system && m_selRow == index.row() && m_selAnchor != m_selCursor) {
            QTextCursor text_cursor(item_geometry->text_document);
            text_cursor.setPosition(qMin(m_selAnchor, m_selCursor));
            text_cursor.setPosition(qMax(m_selAnchor, m_selCursor), QTextCursor::KeepAnchor);

            QAbstractTextDocumentLayout::Selection selection;
            selection.cursor = text_cursor;
            selection.format.setBackground(m_theme.selBg);
            selection.format.setForeground(m_theme.selText);
            ctx.selections.append(selection);
        }

        painter->translate(item_geometry->text_document_origin);
        item_geometry->text_document->documentLayout()->draw(painter, ctx);
    } while(false);
}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    do {
        if (!painter) { break; }
        if (!check_is_message_context(index)) { break; }
        auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        // ---- 系统消息：居中灰字（颜色由主题注入） ----
        if (is_system) {
            paintDocument(painter, index, message_data, item_geometry);
            painter->restore();
            return;
        }

        auto display_name        = message_data->sender->display_name;
        auto sender_avatar_image = message_data->sender->avatar_image;

        // 在群组中才需要绘制昵称，私聊中可以不显示
        bool per_message_display_nickname  = false;
        // 不必每条消息都显示时间
        bool per_message_display_timestamp = false;
        // 要显示的内容
        QString per_message_tips;
        // 获取显示名称
        if (per_message_display_nickname) {
            // 获取发送者的显示名称
            per_message_tips += display_name;
        }
        // 追加时间戳
        if (per_message_display_timestamp) {
            // 发送的时间
            QString send_time = Utils::format_smart_time(message_data->message_time);
            // if (message_context->hasTimestamp() & message_context->timestamp().hasTimestamp()) {
            //     auto datetime = message_context->timestamp().timestamp().toDateTime();
            //     datetime.setTimeZone(QTimeZone::utc());
            //     send_time = Utils::format_smart_time(message_data->message_time);
            // }
            if (!per_message_tips.isEmpty() && !send_time.isEmpty()) { per_message_tips += " "; }
            per_message_tips += send_time;
        }
        // 绘制昵称+时间戳
        if (!per_message_tips.isEmpty()) {
            // ---- 发送者 + 时间 ----
            QFont metaFont = option.font;
            metaFont.setPointSizeF(metaFont.pointSizeF() * 0.85);
            painter->setFont(metaFont);
            painter->setPen(m_theme.metaText);
            auto x = option.rect.left() + kListMargin + kAvatarSize + kAvatarSpacing;
            auto y = option.rect.top () + kRowVMargin;
            auto w = item_geometry->viewport_width - 2 * (kListMargin + kAvatarSize + kAvatarSpacing);
            auto h = kMetaLineHeight;
            const QRectF metaRect(x, y, w, h);
            painter->drawText(metaRect, is_me ? Qt::AlignRight : Qt::AlignLeft, per_message_tips);
        }
        // 画头像
        painter->drawImage(item_geometry->rect_avatar.toRect(), *sender_avatar_image.get());

        // ---- 气泡背景（圆角矩形，主题色；文本/图片/文件共用） ----
        QPainterPath path;
        path.addRoundedRect(item_geometry->rect_bubble, kBubbleRadius, kBubbleRadius);
        painter->setPen(QPen(m_theme.border, 1));
        painter->setBrush(is_me ? m_theme.bubbleMe : m_theme.bubbleOther);
        painter->drawPath(path);

        if (is_text) {
            // ---- 富文本内容：复用缓存文档排版，颜色/选区由 PaintContext 注入 ----
            paintDocument(painter, index, message_data, item_geometry);
        }

        else if (is_image && item_geometry->scaled_image) {
            // ---- 图片：圆角裁剪绘制（缩放位图来自缓存，矢量边框 + 位图内容） ----
            const qreal dpr = item_geometry->scaled_image->devicePixelRatio();
            const QRectF imgRect(item_geometry->text_document_origin, QSizeF(item_geometry->scaled_image->width() / dpr, item_geometry->scaled_image->height() / dpr));

            painter->save();
            QPainterPath clip;
            clip.addRoundedRect(imgRect, kBubbleRadius - 3, kBubbleRadius - 3);
            painter->setClipPath(clip);
            painter->drawImage(imgRect.topLeft(), *item_geometry->scaled_image);
            painter->restore();

            // painter->restore();
            // return;
        }

        else if (is_file) {
            auto human_size = message_data->file->human_size;
            // ---- 文件卡片：图标 + 文件名 + 大小 ----
            const QRectF iconRect(item_geometry->rect_bubble.left() + kBubbleHPad, item_geometry->rect_bubble.top() + (item_geometry->rect_bubble.height() - kFileIconSize) / 2.0, kFileIconSize, kFileIconSize);
            message_data->file->file_icon.paint(painter, iconRect.toRect());
            // if (const auto *style = option.widget ? option.widget->style() : qApp->style()) {
            //     style->standardIcon(QStyle::SP_FileIcon).paint(painter, iconRect.toRect());
            // }

            const qreal textLeft = iconRect.right() + kFileIconGap;
            const qreal textW = item_geometry->rect_bubble.right() - kBubbleHPad - textLeft;
            painter->setPen(m_theme.text);
            painter->setFont(option.font);
            painter->drawText(QRectF(textLeft, item_geometry->rect_bubble.top() + kBubbleVPad, textW, 18), Qt::AlignLeft | Qt::AlignVCenter, item_geometry->fileName);
            QFont sizeFont = option.font;
            sizeFont.setPointSizeF(sizeFont.pointSizeF() * 0.85);
            painter->setFont(sizeFont);
            painter->setPen(m_theme.metaText);
            painter->drawText(QRectF(textLeft, item_geometry->rect_bubble.top() + kBubbleVPad + 20, textW, 14), Qt::AlignLeft | Qt::AlignVCenter, human_size);
            // painter->restore();
            // return;
        }

        painter->restore();
    } while (false);
}

// ---------------------------------------------------------------------------
// 鼠标交互：链接点击 / 文字选择 / 图片与文件点击 / 悬停光标
// ---------------------------------------------------------------------------

void MessageDelegate::beginSelection(const QModelIndex &index, const QPoint &viewportPos,
        std::shared_ptr<MessageData::ItemData> message_data,
        std::shared_ptr<ItemGeometry         > item_geometry) {
    do {
        // if (!check_is_message_context(index)) { break; }
        // auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        // auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        m_selRow = index.row();
        // hitTest(FuzzyHit)：返回离该点最近的文档位置（坐标在段落边界外时取最近端）
        const int pos = qBound(0, int(item_geometry->text_document->documentLayout()->hitTest(viewportPos - item_geometry->text_document_origin, Qt::FuzzyHit)), int(item_geometry->text_document->characterCount()) - 1);
        m_selAnchor = pos;
        m_selCursor = pos;
        m_selecting = true;
        m_selectedText.clear();
    } while(false);
}

void MessageDelegate::extendSelection(const QModelIndex &index, const QPoint &viewportPos,
        std::shared_ptr<MessageData::ItemData> message_data,
        std::shared_ptr<ItemGeometry         > item_geometry) {
    do {
        // if (!check_is_message_context(index)) { break; }
        // auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        // auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        const int pos = qBound(0, int(item_geometry->text_document->documentLayout()->hitTest(viewportPos - item_geometry->text_document_origin, Qt::FuzzyHit)), int(item_geometry->text_document->characterCount()) - 1);
        m_selCursor = pos;
        syncSelectedText(item_geometry->text_document);
    } while(false);
}

void MessageDelegate::syncSelectedText(QTextDocument *doc) {
    if (m_selRow < 0 || m_selAnchor < 0 || m_selCursor < 0) {
        m_selectedText.clear();
        return;
    }
    QTextCursor c(doc);
    c.setPosition(qMin(m_selAnchor, m_selCursor));
    c.setPosition(qMax(m_selAnchor, m_selCursor), QTextCursor::KeepAnchor);
    // selectedText() 的段落分隔符是 U+2029，需要换成真正的换行
    m_selectedText = c.selectedText();
    m_selectedText.replace(QChar(0x2029), QChar('\n'));
}

void MessageDelegate::resetSelection() {
    m_selRow = -1;
    m_selAnchor = -1;
    m_selCursor = -1;
    m_selecting = false;
    m_selectedText.clear();
}

bool MessageDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    do {
        // 只需处理鼠标事件
        if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseMove && event->type() != QEvent::MouseButtonRelease) { break; }

        // =================================================================== //

        if (!check_is_message_context(index)) { break; }
        auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        // =================================================================== //

        auto *view = const_cast<QAbstractItemView *>(qobject_cast<const QAbstractItemView *>(option.widget));

        const QMouseEvent *mouse_event = static_cast<const QMouseEvent *>(event);
        auto pos_mouse = mouse_event->pos();

        if (event->type() == QEvent::MouseButtonPress) {
            if (mouse_event->button() != Qt::LeftButton) { break; }

            m_selecting = false;
            m_pendingLink .clear();
            m_pressViewportPos = pos_mouse;

            if (item_geometry->rect_bubble.contains(pos_mouse)) {
                if (is_text) {
                    const QString anchor = item_geometry->text_document->documentLayout()
                        ->anchorAt(pos_mouse - item_geometry->text_document_origin);
                    if (!anchor.isEmpty()) {
                        // 按在链接上：先记录，松开时若未拖动才触发（见 Release 分支）
                        m_pendingLink = anchor;
                    }
                    else {
                        // 按在正文上：开始文字选择
                        beginSelection(index, pos_mouse, message_data, item_geometry);
                        if (view) { view->viewport()->update(); }
                        return true; // 消费掉按下，避免视图做出其他反应
                    }
                }
            }
            else if (m_selRow >= 0) {
                // 点到气泡之外：清除已有选区
                resetSelection();
                if (view) { view->viewport()->update(); }
            }
            break;
        }

        else if (event->type() == QEvent::MouseMove) {
            // 位移超过阈值视为拖动：取消各类“待触发点击”
            if ((pos_mouse - m_pressViewportPos).manhattanLength() > kDragThreshold) {
                m_pendingLink.clear();
            }

            // 拖动选择中：延伸选区并重绘
            if (m_selecting && index.row() == m_selRow) {
                extendSelection(index, pos_mouse, message_data, item_geometry);
                if (view) { view->viewport()->update(); }
                return true;
            }

            // 悬停光标：链接/文件/可点图片→手型，正文→文本光标，其余→箭头
            if (view) {
                Qt::CursorShape shape = Qt::ArrowCursor;
                if (item_geometry->rect_bubble.contains(pos_mouse)) {
                    if (is_text) {
                        const QString anchor = item_geometry->text_document->documentLayout()
                            ->anchorAt(pos_mouse - item_geometry->text_document_origin);
                        shape = anchor.isEmpty() ? Qt::IBeamCursor : Qt::PointingHandCursor;
                    }
                    else if (is_file) {
                        shape = Qt::PointingHandCursor;
                    }
                    else if (is_image) {
                        shape = Qt::PointingHandCursor;
                    }
                }
                view->viewport()->setCursor(shape);
            }
            break;
        }

        // 需要判断在气泡内抬起才能触发打开文件的动作
        else if (event->type() == QEvent::MouseButtonRelease) {
            if (mouse_event->button() == Qt::LeftButton) {
                if (m_selecting) {
                    m_selecting = false; // 选区保留，等待复制/右键操作
                    return true;
                }
                if (item_geometry->rect_bubble.contains(pos_mouse)) {
                    if (!m_pendingLink.isEmpty()) {
                        emit SignalInner::instance()->signal_open_link(m_pendingLink);
                        m_pendingLink.clear();
                        return true;
                    }
                    // 图片默认为应用内插件查看
                    else if (is_image) {
                        emit SignalInner::instance()->signal_open_data(message_data);
                        return true;
                    }
                    // 其他文件默认为打开文件夹
                    else if (is_file) {
                        emit SignalInner::instance()->signal_open_file(message_data);
                        return true;
                    }
                }
            }
            break;
        }
    } while (false);

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// ---------------------------------------------------------------------------
// 右键菜单
// ---------------------------------------------------------------------------

void MessageDelegate::showContextMenu(QAbstractItemView *view, const QModelIndex &index, const QPoint &globalPos, const QPoint &viewportPos) {
    do {
        if (!check_is_message_context(index)) { break; }
        auto message_data  = index.data(ENUMS::role_item_data()).value<std::shared_ptr<MessageData::ItemData>>();
        // auto item_geometry = itemGeometry(option, index);
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;

        // =================================================================== //

        // 用与 paint/editorEvent 相同的几何计算方式构造 option
        QStyleOptionViewItem option;
        option.widget = view;
        option.rect = view->visualRect(index);
        option.font = view->font();

        auto item_geometry = itemGeometry(option, index);

        // 文本消息
        if (is_text) {
            QString anchor;
            if (!is_system && item_geometry->rect_bubble.contains(viewportPos)) {
                anchor = item_geometry->text_document->documentLayout()->anchorAt(viewportPos - item_geometry->text_document_origin);
            }

            QMenu menu(view);
            const bool hasSelection = (m_selRow == index.row()) && !m_selectedText.isEmpty();

            // 命中超链接：优先展示复制链接
            if (!anchor.isEmpty()) {
                QAction *copyLinkAct = menu.addAction(tr("Copy link address"));  // 复制链接地址
                if (menu.exec(globalPos) == copyLinkAct) {
                    QApplication::clipboard()->setText(anchor);
                }
                return;
            }

            QAction *copySelAct = menu.addAction(tr("Copy selected text"));  // 复制选中文字
            copySelAct->setEnabled(hasSelection);
            menu.addSeparator();
            QAction *selectAllAct = menu.addAction(tr("Select this entire message"));  // 全选本条消息
            QAction *copyAllAct = menu.addAction(tr("Copy the entire message"));  // 复制整条消息

            const QAction *chosen = menu.exec(globalPos);
            if (chosen == copySelAct) {
                QApplication::clipboard()->setText(m_selectedText);
            }
            else if (chosen == selectAllAct) {
                // 整条消息全选：借用 QTextCursor 的 Document 选择范围
                if (item_geometry && item_geometry->text_document) {
                    QTextCursor text_cursor(item_geometry->text_document);
                    text_cursor.select(QTextCursor::Document);
                    m_selRow    = index.row();
                    m_selAnchor = text_cursor.selectionStart();
                    m_selCursor = text_cursor.selectionEnd();
                    syncSelectedText(item_geometry->text_document);
                    if (view->viewport()) { view->viewport()->update(); }
                }
            }
            else if (chosen == copyAllAct) {
                if (item_geometry && item_geometry->text_document) {
                    QApplication::clipboard()->setText(item_geometry->text_document->toPlainText());
                }
            }
            break;
        }

        // 暂不支持的内容类型
        if (!is_file && !is_image) { break; }

        // 图片 / 文件
        const QString&file_path = message_data && message_data->file ? message_data->file->file_path : "";
        bool is_file_exists = QFileInfo::exists(file_path);

        QMenu menu(view);
        QAction *action_open_data   = menu.addAction(tr("Open data"  ));  // 查看数据
        QAction *action_copy_data   = menu.addAction(tr("Copy data"  ));  // 复制数据
        QAction *action_separator_1 = menu.addSeparator();
        QAction *action_open_file   = menu.addAction(tr("Open file"  ));  // 打开文件
        QAction *action_open_folder = menu.addAction(tr("Open folder"));  // 打开所在文件夹
        QAction *action_save_to     = menu.addAction(tr("Save To..." ));  // 保存到...
        QAction *action_separator_2 = menu.addSeparator();
        QAction *action_copy_file   = menu.addAction(tr("Copy file"  ));  // 复制文件到剪切板
        QAction *action_copy_path   = menu.addAction(tr("Copy path"  ));  // 复制路径

        // 统一设置可用状态
        action_open_data  ->setEnabled(is_file_exists);
        action_copy_data  ->setEnabled(is_file_exists);
        action_open_file  ->setEnabled(is_file_exists);
        action_open_folder->setEnabled(is_file_exists);
        action_save_to    ->setEnabled(is_file_exists);
        action_copy_file  ->setEnabled(is_file_exists);
        action_copy_path  ->setEnabled(is_file_exists);

        // 根据类型调整菜单
        if (is_image) {
            action_open_data->setText(tr("View Image"));  // 查看图片
            action_copy_data->setText(tr("Copy Image"));  // 查看图片
        }
        else {
            menu.removeAction(action_open_data  );
            menu.removeAction(action_copy_data  );
            menu.removeAction(action_separator_1);
        }

        // 弹出菜单的处理
        const QAction *chosen = menu.exec(globalPos);
        if (!chosen) { break; }

        ;    if (chosen == action_open_data  ) { emit SignalInner::instance()->signal_open_data  (message_data); }  // 使用内部插件打开
        else if (chosen == action_copy_data  ) { emit SignalInner::instance()->signal_copy_data  (message_data); }  // 复制文件内容到剪切板，比如图像内容
        else if (chosen == action_open_file  ) { emit SignalInner::instance()->signal_open_file  (message_data); }  // 使用系统默认的应用程序打开文件
        else if (chosen == action_open_folder) { emit SignalInner::instance()->signal_open_folder(message_data); }  // 打开文件所在文件夹
        else if (chosen == action_save_to    ) { emit SignalInner::instance()->signal_save_to    (message_data); }  // 文件复制到指定目录
        else if (chosen == action_copy_file  ) { emit SignalInner::instance()->signal_copy_file  (message_data); }  // 复制文件到剪切板，可在资源管理器中粘贴
        else if (chosen == action_copy_path  ) { emit SignalInner::instance()->signal_copy_path  (message_data); }  // 复制文件路径到剪切板
    } while (false);
}
