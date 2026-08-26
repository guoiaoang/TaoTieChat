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

class ChatModel {
public:
    enum Roles {
        SenderRole = Qt::UserRole + 1,  ///< QString 发送者
        HtmlRole,                       ///< QString HTML 内容
        TimeRole,                       ///< QDateTime 时间
        IsMeRole,                       ///< bool 是否自己
        TypeRole,                       ///< int ChatMessage::Type
        ImageRole,                      ///< QImage 原始图片（Image 消息）
        ImagePathRole,                  ///< QString 图片来源路径（可为空）
        FileNameRole,                   ///< QString 文件名（File 消息）
        FilePathRole,                   ///< QString 文件路径（File 消息）
        FileSizeRole,                   ///< qint64 文件字节数（File 消息）
    };
};

struct ChatMessage {
    enum class Type {
        Text,    ///< 普通聊天消息（富文本气泡）
        System,  ///< 系统提示（居中灰色小字）
        Image,   ///< 图片消息（缩放气泡，来自剪贴板粘贴或文件选择）
        File,    ///< 文件消息（文件卡片，点击打开所在位置）
    };

    QString sender;          ///< 发送者昵称
    QString html;            ///< 消息内容（HTML 片段，纯文本也合法；Text 消息用）
    QDateTime time;          ///< 发送时间
    bool isMe = false;       ///< 是否为自己发送（决定气泡左右对齐与配色）
    Type type = Type::Text;  ///< 消息类型

    // ---- 图片消息（Type::Image） ----
    QImage image;            ///< 原始位图（QImage 隐式共享，拷贝廉价）
    QString imageFilePath;   ///< 来源文件路径（粘贴的图片为空；非空时点击可用看图软件打开）

    // ---- 文件消息（Type::File） ----
    QString fileName;        ///< 显示文件名（含扩展名）
    QString filePath;        ///< 本地路径（空则"打开所在文件夹"不可用）
    qint64 fileSize = 0;     ///< 字节数（显示时转成 KB/MB）
};

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

/// 由昵称确定性生成头像底色（同一人颜色稳定）
QColor avatarColor(const QString &name) {
    size_t h = qHash(name);
    return QColor::fromHsl(int(h % 360), 130, 140);
}

/// 字节数 → 人类可读大小（KB/MB/GB）
QString humanSize(qint64 bytes) {
    if (bytes < 1024) {
        return QStringLiteral("%1 B").arg(bytes);
    }
    const char *units[] = { "KB", "MB", "GB", "TB" };
    double v = double(bytes);
    int u = -1;
    do { v /= 1024.0; ++u; } while (v >= 1024.0 && u < 3);
    return QStringLiteral("%1 %2").arg(QString::number(v, 'f', 1), QLatin1String(units[u]));
}

} // namespace

// ---------------------------------------------------------------------------
// 主题
// ---------------------------------------------------------------------------

ChatTheme ChatTheme::light() {
    ChatTheme t;
    t.viewBg      = QColor(0xF5, 0xF5, 0xF5);
    t.windowBg    = QColor(0xFA, 0xFA, 0xFA);
    t.bubbleMe    = QColor(0x95, 0xEC, 0x69);   // 微信绿
    t.bubbleOther = QColor(0xFF, 0xFF, 0xFF);
    t.text        = QColor(0x1A, 0x1A, 0x1A);
    t.systemText  = QColor(0x8A, 0x8A, 0x8A);
    t.metaText    = QColor(0x9A, 0x9A, 0x9A);
    t.border      = QColor(0, 0, 0, 25);
    t.link        = QColor(0x1E, 0x88, 0xE5);
    t.selBg       = QColor(0x1E, 0x88, 0xE5);
    t.selText     = QColor(0xFF, 0xFF, 0xFF);
    return t;
}

ChatTheme ChatTheme::dark() {
    ChatTheme t;
    t.viewBg      = QColor(0x1B, 0x1B, 0x1B);
    t.windowBg    = QColor(0x20, 0x20, 0x20);
    t.bubbleMe    = QColor(0x2E, 0x7D, 0x46);   // 暗化的微信绿
    t.bubbleOther = QColor(0x2C, 0x2C, 0x2C);
    t.text        = QColor(0xE4, 0xE4, 0xE4);
    t.systemText  = QColor(0x8A, 0x8A, 0x8A);
    t.metaText    = QColor(0x8F, 0x8F, 0x8F);
    t.border      = QColor(255, 255, 255, 30);
    t.link        = QColor(0x64, 0xB5, 0xF6);
    t.selBg       = QColor(0x37, 0x62, 0xA8);
    t.selText     = QColor(0xFF, 0xFF, 0xFF);
    return t;
}

// MessageDelegate::MessageDelegate(QObject *parent) : QStyledItemDelegate(parent) {
//     m_theme = ChatTheme::light();
// }

MessageDelegate::MessageDelegate(QObject *parent) : ItemDelegateBase(parent) {
    m_theme = ChatTheme::light();
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

bool check_is_message_context(const QModelIndex &index) {
    if (!index.isValid()) { return false; }
    const QVariant variant_data = index.data(ENUMS::role_item_data());
    if (!variant_data.isValid() || !variant_data.canConvert<std::shared_ptr<TaoTieChat::MessageContext>>()) { return false; }
    return true;
}

// ---------------------------------------------------------------------------
// 文档的创建与获取（缓存核心）
// ---------------------------------------------------------------------------

QTextDocument *MessageDelegate::createDocument(const QModelIndex &index, const QFont &font) const {
    auto *doc = new QTextDocument;
    doc->setDocumentMargin(0);  // 边距由气泡矩形自己控制
    doc->setDefaultFont(font);
    do {
        if (!check_is_message_context(index)) { break; }

        const auto message_context = index.data(ENUMS::role_item_data()).value<std::shared_ptr<TaoTieChat::MessageContext>>();
        const QString short_text = message_context->hasShortText() ? message_context->shortText() : QString();

        // =================================================================== //

        // const QString html = short_text;
        // doc->setHtml(html);
        doc->setPlainText(short_text);
    } while (false);
    return doc;
}

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

const QPixmap *MessageDelegate::acquireImage(const QModelIndex &index, qreal maxW, qreal maxH) const {
    const int row = index.row();
    if (!m_cache.contains(row)) {
        DocEntry entry;
        entry.lastUsed = ++m_generation;
        m_cache[row] = entry;
    }
    auto&entry = m_cache[row];
    entry.lastUsed = ++m_generation;

    do {
        if (!check_is_message_context(index)) { break; }

        const auto message_context = index.data(ENUMS::role_item_data()).value<std::shared_ptr<TaoTieChat::MessageContext>>();
        const QString short_text = message_context->hasShortText() ? message_context->shortText() : QString();

        // =================================================================== //

        // 原图走模型角色获取（QImage 隐式共享，无拷贝开销）
        // TODO：未实现图片消息
        // const QImage img = index.data(ChatModel::ImageRole).value<QImage>();
        const QImage img = QImage(message_context->shortText());
        if (img.isNull()) { return nullptr; }

        // 等比缩放到 (maxW, maxH) 以内；小图不放大
        const qreal scale   = qMin<qreal>(1.0, qMin(maxW / img.width(), maxH / img.height()));
        const qreal targetW = qMax<qreal>(1.0, img.width() * scale);
        const qreal targetH = qMax<qreal>(1.0, img.height() * scale);

        if (entry.pixmap.isNull() || qAbs(entry.pixmapWidth - targetW) > 0.5) {
            // 宽度变化（窗口 resize / 首次布局）：按设备像素比重缩放，保证高 DPI 清晰
            const qreal dpr = qApp->devicePixelRatio();
            const QImage scaled = img.scaled(qRound(targetW * dpr), qRound(targetH * dpr), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            entry.pixmap = QPixmap::fromImage(scaled);
            entry.pixmap.setDevicePixelRatio(dpr);
            entry.pixmapWidth = targetW;
            ++m_layoutCount; // 与文档重排版同口径计入统计
        }

        trimCache();
    } while (false);
    return &entry.pixmap;
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

MessageDelegate::ItemGeometry MessageDelegate::itemGeometry(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    ItemGeometry g;
    g.viewWidth = viewportWidth(option);
    // const auto type = static_cast<ChatMessage::Type>(index.data(ChatModel::TypeRole).toInt());
    const auto type = ChatMessage::Type::Text;
    g.isSystem = type == ChatMessage::Type::System;
    g.isText   = type == ChatMessage::Type::Text  ;
    g.isImage  = type == ChatMessage::Type::Image ;
    g.isFile   = type == ChatMessage::Type::File  ;

    do {
        if (!check_is_message_context(index)) { break; }

        const auto message_context = index.data(ENUMS::role_item_data()).value<std::shared_ptr<TaoTieChat::MessageContext>>();
        const QString short_text = message_context->hasShortText() ? message_context->shortText() : QString();

        // =================================================================== //

        auto account_id = Accounts::instance().get_current_account()->baseInfo().accountId();

        // g.isMe = index.data(ChatModel::IsMeRole).toBool();
        g.isMe = message_context->userId() == account_id;

        // 去掉两头头像宽度后中间的宽度。
        // 头像位置对聊天类消息（文本/图片/文件）一致
        // const int availW = g.viewWidth - 2 * kListMargin - kAvatarSize - kAvatarSpacing;
        const int availW = g.viewWidth - (kListMargin + kAvatarSize + kAvatarSpacing + kBubbleHPad) * 2;
        // 计算气泡绘制宽度
        // const qreal maxBubbleW = qMax<qreal>(kMinTextWidth, availW * kMaxBubblePct / 100.0);
        const qreal maxBubbleW = qMax<qreal>(kMinTextWidth, availW);
        const qreal metaTop = option.rect.top() + kRowVMargin;
        g.avatarRect = QRectF(g.isMe ? g.viewWidth - kListMargin - kAvatarSize : option.rect.left() + kListMargin, metaTop + kMetaLineHeight, kAvatarSize, kAvatarSize);

        // 系统提示消息
        if (g.isSystem) {
            // 系统消息：整行可用宽度，居中绘制
            const qreal maxTextW = g.viewWidth - 2 * kListMargin;
            g.doc = acquireDocument(index, maxTextW, option.font);
            const qreal w = g.doc->size().width();
            g.docOrigin = QPointF(option.rect.left() + (g.viewWidth - w) / 2.0, option.rect.top() + kRowVMargin);
            break;
        }

        // 文本消息
        else if (g.isText) {
            // 普通文本消息
            g.doc = acquireDocument(index, maxBubbleW, option.font);

            const qreal textW = g.doc->size().width();   // == 排版宽度（已含收窄）
            const qreal textH = g.doc->size().height();

            const qreal bubbleW = textW + 2 * kBubbleHPad;
            const qreal bubbleH = textH + 2 * kBubbleVPad;

            g.bubbleRect = QRectF(g.isMe ? g.avatarRect.left() - kAvatarSpacing - bubbleW : g.avatarRect.right() + kAvatarSpacing, g.avatarRect.top(), bubbleW, bubbleH);
            g.docOrigin = QPointF(g.bubbleRect.left() + kBubbleHPad, g.bubbleRect.top() + kBubbleVPad);
            break;
        }

        // 图片消息
        else if (g.isImage) {
            // 图片消息：缩放位图 + 气泡内边距
            g.image = acquireImage(index, maxBubbleW, kMaxImageHeight);
            const qreal dpr  = g.image ? g.image->devicePixelRatio() : 1.0;
            const qreal imgW = g.image ? g.image->width() / dpr : kMinTextWidth;
            const qreal imgH = g.image ? g.image->height() / dpr : 0;
            const qreal bubbleW = imgW + 2 * kBubbleHPad;
            const qreal bubbleH = imgH + 2 * kBubbleVPad;
            g.bubbleRect = QRectF(g.isMe ? g.avatarRect.left() - kAvatarSpacing - bubbleW : g.avatarRect.right() + kAvatarSpacing, g.avatarRect.top(), bubbleW, bubbleH);
            g.docOrigin = QPointF(g.bubbleRect.left() + kBubbleHPad, g.bubbleRect.top() + kBubbleVPad);
            break;
        }

        // 文件消息
        else if (g.isFile) {
            // 文件消息：固定高度卡片，宽度随文件名自适应（超长省略号）
            const QFontMetrics fm(option.font);
            const QString name = short_text;
            // const QString name = index.data(ChatModel::FileNameRole).toString();
            const int nameSpace = int(maxBubbleW) - 2 * kBubbleHPad - kFileIconSize - kFileIconGap;
            g.fileName = fm.elidedText(name, Qt::ElideMiddle, qMax(nameSpace, 24));
            const qreal nameW = fm.horizontalAdvance(g.fileName);
            const qreal bubbleW = qMax<qreal>(kMinTextWidth, qMin<qreal>(2 * kBubbleHPad + kFileIconSize + kFileIconGap + nameW, maxBubbleW));
            g.bubbleRect = QRectF(g.isMe ? g.avatarRect.left() - kAvatarSpacing - bubbleW : g.avatarRect.right() + kAvatarSpacing, g.avatarRect.top(), bubbleW, kFileCardHeight);
            g.fileSize = -1;
            // g.fileSize = index.data(ChatModel::FileSizeRole).toLongLong();
            break;
        }
    } while (false);
    return g;
}

// ---------------------------------------------------------------------------
// QStyledItemDelegate 接口
// ---------------------------------------------------------------------------

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // 关键时序：sizeHint 先于 paint 被调用。
    // 这里顺便完成文档/图片的创建与排版并写入缓存，paint 阶段直接复用。
    const ItemGeometry g = itemGeometry(option, index);

    if (g.isSystem) {
        const qreal docH = g.doc->size().height();
        return QSize(g.viewWidth, int(kRowVMargin * 2 + docH + 0.5));
    }

    const qreal bodyH = qMax<qreal>(g.bubbleRect.height(), qreal(kAvatarSize));
    return QSize(g.viewWidth, int(kRowVMargin * 2 + kMetaLineHeight + bodyH + 0.5));
}

void MessageDelegate::paintDocument(QPainter *painter, const ItemGeometry &g, const QModelIndex &index) const {
    // 主题色通过 PaintContext 调色板注入——这是文档能保持主题无关的关键
    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette.setColor(QPalette::Text, g.isSystem ? m_theme.systemText : m_theme.text);
    ctx.palette.setColor(QPalette::Link, m_theme.link);
    ctx.palette.setColor(QPalette::LinkVisited, m_theme.link);

    // 文字选区：只对当前选中的那一行生效
    if (!g.isSystem && m_selRow == index.row() && m_selAnchor != m_selCursor) {
        QTextCursor c(g.doc);
        c.setPosition(qMin(m_selAnchor, m_selCursor));
        c.setPosition(qMax(m_selAnchor, m_selCursor), QTextCursor::KeepAnchor);

        QAbstractTextDocumentLayout::Selection sel;
        sel.cursor = c;
        sel.format.setBackground(m_theme.selBg);
        sel.format.setForeground(m_theme.selText);
        ctx.selections.append(sel);
    }

    painter->translate(g.docOrigin);
    g.doc->documentLayout()->draw(painter, ctx);
}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    do {
        if (!check_is_message_context(index)) { break; }

        const auto message_context = index.data(ENUMS::role_item_data()).value<std::shared_ptr<TaoTieChat::MessageContext>>();
        const QString short_text = message_context->hasShortText() ? message_context->shortText() : QString();

        // =================================================================== //

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const ItemGeometry g = itemGeometry(option, index);

        // ---- 系统消息：居中灰字（颜色由主题注入） ----
        if (g.isSystem) {
            paintDocument(painter, g, index);
            painter->restore();
            return;
        }

        // const QString sender = index.data(ChatModel::SenderRole).toString();
        QString sender;
        if (message_context->hasUserId()) {
            auto contact = Accounts::instance().get_contact(message_context->userId());
            // 如果是好友就获取备注
            if (!g.isMe && contact->hasFriendInfo() && !contact->friendInfo().notes().isEmpty()) {
                sender = contact->friendInfo().notes();
            }
            // 没有备注尝试获取昵称
            else if (contact->hasBaseInfo() && !contact->baseInfo().nickname().isEmpty()) {
                sender = contact->baseInfo().nickname();
            }
#ifdef _DEBUG
            // 尝试获取用户 ID
            else if (contact->hasBaseInfo() && !contact->baseInfo().userId().isEmpty()) {
                sender = contact->baseInfo().userId();
            }
            // 尝试获取账号 ID
            else if (contact->hasBaseInfo() && !contact->baseInfo().accountId().isEmpty()) {
                sender = contact->baseInfo().accountId();
            }
#endif // _DEBUG
        }
        // const QString time = index.data(ChatModel::TimeRole).toDateTime().toString(QStringLiteral("HH:mm"));
        QString time;
        if (message_context->hasTimestamp() & message_context->timestamp().hasTimestamp()) {
            time = message_context->timestamp().timestamp().toDateTime().toString(QStringLiteral("HH:mm:SS"));
        }
        QString avatar_id;
        if (message_context->hasUserId()) {
            auto contact = Accounts::instance().get_contact(message_context->userId());
            if (contact->hasBaseInfo() && !contact->baseInfo().avatarId().isEmpty()) {
                avatar_id = contact->baseInfo().avatarId();
            }
        }
        QPixmap avatar_pixamp = Accounts::instance().get_avatar(avatar_id);

        // ---- 发送者 + 时间 ----
        QFont metaFont = option.font;
        metaFont.setPointSizeF(metaFont.pointSizeF() * 0.85);
        painter->setFont(metaFont);
        painter->setPen(m_theme.metaText);
        const QRectF metaRect(option.rect.left() + kListMargin + kAvatarSize + kAvatarSpacing,
                            option.rect.top() + kRowVMargin,
                            g.viewWidth - 2 * (kListMargin + kAvatarSize + kAvatarSpacing),
                            kMetaLineHeight);
        painter->drawText(metaRect, g.isMe ? Qt::AlignRight : Qt::AlignLeft, QStringLiteral("%1  %2").arg(sender, time));

        // ---- 头像（圆形 + 姓氏首字） ----
        // painter->setPen(Qt::NoPen);
        // painter->setBrush(avatarColor(sender));
        // painter->drawEllipse(g.avatarRect);
        // QFont avatarFont = option.font;
        // avatarFont.setBold(true);
        // avatarFont.setPointSizeF(avatarFont.pointSizeF() * 1.15);
        // painter->setFont(avatarFont);
        // painter->setPen(Qt::white);
        // painter->drawText(g.avatarRect, Qt::AlignCenter, sender.left(1));
        painter->drawPixmap(g.avatarRect.toRect(), avatar_pixamp);

        // ---- 气泡背景（圆角矩形，主题色；文本/图片/文件共用） ----
        QPainterPath path;
        path.addRoundedRect(g.bubbleRect, kBubbleRadius, kBubbleRadius);
        painter->setPen(QPen(m_theme.border, 1));
        painter->setBrush(g.isMe ? m_theme.bubbleMe : m_theme.bubbleOther);
        painter->drawPath(path);

        if (g.isImage && g.image) {
            // ---- 图片：圆角裁剪绘制（缩放位图来自缓存，矢量边框 + 位图内容） ----
            const qreal dpr = g.image->devicePixelRatio();
            const QRectF imgRect(g.docOrigin, QSizeF(g.image->width() / dpr, g.image->height() / dpr));
            painter->save();
            QPainterPath clip;
            clip.addRoundedRect(imgRect, kBubbleRadius - 3, kBubbleRadius - 3);
            painter->setClipPath(clip);
            painter->drawPixmap(imgRect.topLeft(), *g.image);
            painter->restore();
            painter->restore();
            return;
        }

        if (g.isFile) {
            // ---- 文件卡片：图标 + 文件名 + 大小 ----
            const QRectF iconRect(g.bubbleRect.left() + kBubbleHPad, g.bubbleRect.top() + (g.bubbleRect.height() - kFileIconSize) / 2.0, kFileIconSize, kFileIconSize);
            if (const auto *style = option.widget ? option.widget->style() : qApp->style()) {
                style->standardIcon(QStyle::SP_FileIcon).paint(painter, iconRect.toRect());
            }

            const qreal textLeft = iconRect.right() + kFileIconGap;
            const qreal textW = g.bubbleRect.right() - kBubbleHPad - textLeft;
            painter->setPen(m_theme.text);
            painter->setFont(option.font);
            painter->drawText(QRectF(textLeft, g.bubbleRect.top() + kBubbleVPad, textW, 18), Qt::AlignLeft | Qt::AlignVCenter, g.fileName);
            QFont sizeFont = option.font;
            sizeFont.setPointSizeF(sizeFont.pointSizeF() * 0.85);
            painter->setFont(sizeFont);
            painter->setPen(m_theme.metaText);
            painter->drawText(QRectF(textLeft, g.bubbleRect.top() + kBubbleVPad + 20, textW, 14), Qt::AlignLeft | Qt::AlignVCenter, humanSize(g.fileSize));
            painter->restore();
            return;
        }

        // ---- 富文本内容：复用缓存文档排版，颜色/选区由 PaintContext 注入 ----
        paintDocument(painter, g, index);

        painter->restore();
    } while (false);
}

// ---------------------------------------------------------------------------
// 鼠标交互：链接点击 / 文字选择 / 图片与文件点击 / 悬停光标
// ---------------------------------------------------------------------------

void MessageDelegate::beginSelection(const QModelIndex &index, const ItemGeometry &g, const QPoint &viewportPos) {
    m_selRow = index.row();
    // hitTest(FuzzyHit)：返回离该点最近的文档位置（坐标在段落边界外时取最近端）
    const int pos = qBound(0, int(g.doc->documentLayout()->hitTest(viewportPos - g.docOrigin, Qt::FuzzyHit)), int(g.doc->characterCount()) - 1);
    m_selAnchor = pos;
    m_selCursor = pos;
    m_selecting = true;
    m_selectedText.clear();
}

void MessageDelegate::extendSelection(const ItemGeometry &g, const QPoint &viewportPos) {
    const int pos = qBound(0, int(g.doc->documentLayout()->hitTest(viewportPos - g.docOrigin, Qt::FuzzyHit)), int(g.doc->characterCount()) - 1);
    m_selCursor = pos;
    syncSelectedText(g.doc);
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
        if (!check_is_message_context(index)) { break; }

        const auto message_context = index.data(ENUMS::role_item_data()).value<std::shared_ptr<TaoTieChat::MessageContext>>();
        const QString short_text = message_context->hasShortText() ? message_context->shortText() : QString();

        // =================================================================== //

        // const auto type = static_cast<ChatMessage::Type>(index.data(ChatModel::TypeRole).toInt());
        const auto type = ChatMessage::Type::Text;
        auto *view = const_cast<QAbstractItemView *>(qobject_cast<const QAbstractItemView *>(option.widget));

        if (event->type() == QEvent::MouseButtonPress) {
            const auto *me = static_cast<const QMouseEvent *>(event);
            if (me->button() != Qt::LeftButton) { break; }

            m_selecting = false;
            m_pendingLink.clear();
            m_pendingImage.clear();
            m_pendingFile.clear();
            m_pressViewportPos = me->pos();
            const ItemGeometry g = itemGeometry(option, index);

            if (g.bubbleRect.contains(me->pos())) {
                if (type == ChatMessage::Type::Text) {
                    const QString anchor = g.doc->documentLayout()->anchorAt(me->pos() - g.docOrigin);
                    if (!anchor.isEmpty()) {
                        // 按在链接上：先记录，松开时若未拖动才触发（见 Release 分支）
                        m_pendingLink = anchor;
                    }
                    else {
                        // 按在正文上：开始文字选择
                        beginSelection(index, g, me->pos());
                        if (view) { view->viewport()->update(); }
                        return true; // 消费掉按下，避免视图做出其他反应
                    }
                }
                else if (type == ChatMessage::Type::File) {
                    // 文件卡片可点击（打开所在位置）
                    // m_pendingFile = index.data(ChatModel::FilePathRole).toString();
                    m_pendingFile = short_text;
                }
                else if (type == ChatMessage::Type::Image) {
                    // 有来源路径的图片可点击（用看图软件打开）
                    // m_pendingImage = index.data(ChatModel::ImagePathRole).toString();
                    m_pendingImage = short_text;
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
            const auto *me = static_cast<const QMouseEvent *>(event);
            const ItemGeometry g = itemGeometry(option, index);

            // 位移超过阈值视为拖动：取消各类“待触发点击”
            if ((me->pos() - m_pressViewportPos).manhattanLength() > kDragThreshold) {
                m_pendingLink.clear();
                m_pendingImage.clear();
                m_pendingFile.clear();
            }

            // 拖动选择中：延伸选区并重绘
            if (m_selecting && index.row() == m_selRow) {
                extendSelection(g, me->pos());
                if (view) { view->viewport()->update(); }
                return true;
            }

            // 悬停光标：链接/文件/可点图片→手型，正文→文本光标，其余→箭头
            if (view) {
                Qt::CursorShape shape = Qt::ArrowCursor;
                if (g.bubbleRect.contains(me->pos())) {
                    if (type == ChatMessage::Type::Text) {
                        const QString anchor = g.doc->documentLayout()->anchorAt(me->pos() - g.docOrigin);
                        shape = anchor.isEmpty() ? Qt::IBeamCursor : Qt::PointingHandCursor;
                    }
                    else if (type == ChatMessage::Type::File) {
                        shape = Qt::PointingHandCursor;
                    }
                    // else if (type == ChatMessage::Type::Image && !index.data(ChatModel::ImagePathRole).toString().isEmpty()) {
                    else if (type == ChatMessage::Type::Image) {
                        shape = Qt::PointingHandCursor;
                    }
                }
                view->viewport()->setCursor(shape);
            }
            break;
        }

        else if (event->type() == QEvent::MouseButtonRelease) {
            const auto *me = static_cast<const QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                if (m_selecting) {
                    m_selecting = false; // 选区保留，等待复制/右键操作
                    return true;
                }
                if (!m_pendingLink.isEmpty()) {
                    emit linkActivated(m_pendingLink);
                    m_pendingLink.clear();
                    return true;
                }
                if (!m_pendingFile.isEmpty()) {
                    emit fileActivated(m_pendingFile);
                    m_pendingFile.clear();
                    return true;
                }
                if (!m_pendingImage.isEmpty()) {
                    emit imageActivated(m_pendingImage);
                    m_pendingImage.clear();
                    return true;
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

        const auto message_context = index.data(ENUMS::role_item_data()).value<std::shared_ptr<TaoTieChat::MessageContext>>();
        const QString short_text = message_context->hasShortText() ? message_context->shortText() : QString();

        // =================================================================== //

        // 用与 paint/editorEvent 相同的几何计算方式构造 option
        QStyleOptionViewItem option;
        option.widget = view;
        option.rect = view->visualRect(index);
        option.font = view->font();
        const ItemGeometry g = itemGeometry(option, index);

        // ---- 文件消息 ----
        if (g.isFile) {
            // const QString path = index.data(ChatModel::FilePathRole).toString();
            const QString path = short_text;
            QMenu menu(view);
            QAction *openAct = menu.addAction(QStringLiteral("打开所在文件夹"));
            openAct->setEnabled(QFileInfo::exists(path));
            QAction *copyPathAct = menu.addAction(QStringLiteral("复制路径"));

            const QAction *chosen = menu.exec(globalPos);
            if (chosen == openAct) {
                emit fileActivated(path);
            }
            else if (chosen == copyPathAct) {
                // QApplication::clipboard()->setText(index.data(ChatModel::FileNameRole).toString());
                QApplication::clipboard()->setText(short_text);
            }
            return;
        }

        // ---- 图片消息 ----
        if (g.isImage) {
            // const QImage img = index.data(ChatModel::ImageRole).value<QImage>();
            const QImage img = QImage(short_text);
            QMenu menu(view);
            QAction *copyAct = menu.addAction(QStringLiteral("复制图片"));
            copyAct->setEnabled(!img.isNull());
            QAction *saveAct = menu.addAction(QStringLiteral("另存为…"));
            saveAct->setEnabled(!img.isNull());

            const QAction *chosen = menu.exec(globalPos);
            if (chosen == copyAct) {
                QApplication::clipboard()->setImage(img);
            }
            else if (chosen == saveAct) {
                const QString path = QFileDialog::getSaveFileName(view, QStringLiteral("保存图片"), QStringLiteral("image.png"), QStringLiteral("PNG (*.png)"));
                if (!path.isEmpty()) {
                    img.save(path, "PNG");
                }
            }
            return;
        }

        // ---- 文本消息（原有逻辑） ----
        // 命中检测：右键处是否在链接上
        QString anchor;
        if (!g.isSystem && g.bubbleRect.contains(viewportPos)) {
            anchor = g.doc->documentLayout()->anchorAt(viewportPos - g.docOrigin);
        }

        QMenu menu(view);
        const bool hasSelection = (m_selRow == index.row()) && !m_selectedText.isEmpty();

        if (!anchor.isEmpty()) {
            QAction *copyLinkAct = menu.addAction(QStringLiteral("复制链接地址"));
            if (menu.exec(globalPos) == copyLinkAct) {
                QApplication::clipboard()->setText(anchor);
            }
            return;
        }

        QAction *copySelAct = menu.addAction(QStringLiteral("复制选中文字"));
        copySelAct->setEnabled(hasSelection);
        menu.addSeparator();
        QAction *selectAllAct = menu.addAction(QStringLiteral("全选本条消息"));
        QAction *copyAllAct = menu.addAction(QStringLiteral("复制整条消息"));

        const QAction *chosen = menu.exec(globalPos);
        if (chosen == copySelAct) {
            QApplication::clipboard()->setText(m_selectedText);
        }
        else if (chosen == selectAllAct) {
            // 整条消息全选：借用 QTextCursor 的 Document 选择范围
            QTextCursor c(g.doc);
            c.select(QTextCursor::Document);
            m_selRow = index.row();
            m_selAnchor = c.selectionStart();
            m_selCursor = c.selectionEnd();
            syncSelectedText(g.doc);
            view->viewport()->update();
        }
        else if (chosen == copyAllAct) {
            QApplication::clipboard()->setText(g.doc->toPlainText());
        }
    } while (false);
}
