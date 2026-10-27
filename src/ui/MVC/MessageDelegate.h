#pragma once

#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QHash>
#include <QColor>

#include "ItemDelegate.h"
#include "Defines.h"

/**
 * @brief 聊天消息委托：QTextDocument 解析结果缓存 + 按需重排版
 *
 * 对应聊天记录中得出的核心结论：
 *
 *  1. 卡顿根因：QListView 滚动/布局时会高频调用 sizeHint()，
 *     而 QTextDocument 的 HTML 解析 + 排版是昂贵的同步操作。
 *
 *  2. 关键时序：sizeHint() 先于 paint() 被调用。
 *     因此在 sizeHint() 中完成 QTextDocument 的创建与解析并放入缓存，
 *     paint() 直接命中缓存，绝不重复解析。
 *
 *  3. 缓存的是“解析后的 QTextDocument”而不是 QPixmap：
 *     - HTML 只解析一次（贵，只做一次）；
 *     - 窗口宽度变化时只需 setTextWidth() 重新排版（便宜），
 *       无需销毁文档重新 setHtml()。
 *     - 相比预渲染 QPixmap 方案内存占用更低，且天然支持高 DPI
 *       （矢量绘制，无需按 devicePixelRatioF 生成多倍位图）。
 *
 *  4. LRU 淘汰：只保留最近使用的约 200 个文档。滚动离开视口的
 *     旧条目被释放，滚回来时按需重建——兼顾内存与流畅度。
 *
 *  5. 缓存失效：本示例模型只在尾部追加（行号稳定），行号可直接作
 *     缓存键；模型结构/数据变化时由外部调用 clearCache()。
 *
 * 此外本委托还实现了富文本的文字选择（鼠标拖动高亮）与右键菜单，
 * 均基于缓存文档的 cursorForPosition() / PaintContext.selection 实现。
 */
class MessageDelegate : public ItemDelegateBase {
    Q_OBJECT

private:
    struct DocEntry;
    struct ItemGeometry;

public:
    explicit MessageDelegate(QObject *parent = nullptr);
    ~MessageDelegate() override;

    const ENUMS::ItemDataType item_type() const { return ENUMS::ItemDataType::CHATROOM; };

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

public:
    /// ---- 主题 ----
    /// 文档与主题无关（颜色绘制时注入），切换主题无需清缓存。
    void setTheme(const MessageData::ChatTheme &theme) { m_theme = theme; }
    const MessageData::ChatTheme &theme() const { return m_theme; }

    /// ---- 文字选择 ----
    /// 当前选中文字（跨段落换行已转为 '\n'），无选择时为空
    QString selectedText() const { return m_selectedText; }
    /// 清除选区（调用方负责触发重绘）
    void resetSelection();

    /// 右键菜单：复制链接 / 复制选中文字 / 全选本条消息 / 复制整条消息
    void showContextMenu(QAbstractItemView *view, const QModelIndex &index, const QPoint &globalPos, const QPoint &viewportPos);

    /// 缓存运行统计（窗口顶部的状态栏显示，用于验证缓存是否生效）
    struct CacheStats {
        int    cachedDocs = 0;  // 当前缓存的 QTextDocument 数量
        qint64 docCreated = 0;  // 历史创建（含 HTML 解析）次数，越小越好
        qint64 relayouts  = 0;  // 历史重排版次数（仅由宽度变化触发）
    };
    CacheStats cacheStats() const {
        return CacheStats{ int(m_cache.size()), m_createCount, m_layoutCount };
    }

public slots:
    /// 清空全部缓存（模型行删除 / 重置 / 数据变化 / 字体变化时调用）
    void clearCache();

private:
    std::shared_ptr<ItemGeometry> itemGeometry(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    /// 取（或按需创建）某行的文档，并保证其已按目标宽度排版。
    /// 命中缓存且宽度未变 → 零开销直接返回。
    QTextDocument *acquireDocument(const QModelIndex &index, qreal maxTextWidth, const QFont &font) const;

    QTextDocument *createDocument(const QModelIndex &index, const QFont &font) const;

    /// 取（或按需缩放）某行图片，返回适配 (maxW, maxH) 且考虑设备像素比的
    /// 位图（同样走 LRU 缓存：宽度变化只重缩放、不重新解码原图）。
    const QImage *acquireImage(const QModelIndex &index, qreal maxW, qreal maxH) const;

    /// 超出上限时按 lastUsed 淘汰最久未用的文档
    void trimCache() const;

    /// 用 PaintContext 调色板绘制文档（主题色 + 文字选区高亮都在这里注入）
    void paintDocument(QPainter *painter, const QModelIndex &index,
        std::shared_ptr<MessageData::ItemData> message_data,
        std::shared_ptr<ItemGeometry         > item_geometry) const;

    // ---- 文字选择状态 ----
    void beginSelection(const QModelIndex &index, const QPoint &viewportPos,
        std::shared_ptr<MessageData::ItemData> message_data,
        std::shared_ptr<ItemGeometry         > item_geometry);
    void extendSelection(const QModelIndex &index, const QPoint &viewportPos,
        std::shared_ptr<MessageData::ItemData> message_data,
        std::shared_ptr<ItemGeometry         > item_geometry);
    void syncSelectedText(QTextDocument *doc);

private:
    int     m_selRow    =    -1;  // 选区所在行
    int     m_selAnchor =    -1;  // 选区锚点（按下处）文档位置
    int     m_selCursor =    -1;  // 选区光标（当前处）文档位置
    bool    m_selecting = false;  // 是否正处于拖动选择中
    QString m_selectedText;       // 已选文字缓存（U+2029 已替换为 '\n'）
    QString m_pendingLink;        // 按下时命中的链接（松开时若未拖动则触发）
    QPoint  m_pressViewportPos;   // 按下位置（用于区分点击与拖动）

    // 一个条目的完整绘制几何信息（paint / sizeHint / editorEvent 共用，
    // 保证三者对同一行的布局计算完全一致）
    struct ItemGeometry {
        QImage  source_image;
        QString fileName    ;  // 文件消息：已按宽度省略的显示名
        qint64  fileSize = 0;  // 文件消息：字节数
        //
        int            viewport_width = 0;    // 视口宽度（决定气泡最大宽度）
        QPointF        text_document_origin;  // 文档/图片内容在 viewport 坐标系中的原点
        QRectF         rect_avatar;
        QRectF         rect_bubble;
        const QImage  *scaled_image  = nullptr;  // 图片消息：缩放后的位图（缓存条目持有）
        QTextDocument *text_document = nullptr;
    };

    struct DocEntry {
        QTextDocument *doc = nullptr;  // Text/System：解析后的文档
        QImage image;              // Image：按当前宽度缩放后的位图
        qreal  naturalWidth = -1;  // 不换行时的自然宽度（创建时测得，用于气泡自适应收窄）
        qreal  textWidth    = -1;  // 当前排版宽度；-1 表示尚未排版
        qreal  imageWidth   = -1;  // 当前图片逻辑宽度；-1 表示尚未缩放
        qint64 lastUsed     =  0;  // LRU 代际号
    };

    mutable QHash<int, DocEntry> m_cache;  // key: row
    mutable qint64 m_generation  = 0;      // LRU 代际计数器
    mutable qint64 m_createCount = 0;      // 统计：创建次数
    mutable qint64 m_layoutCount = 0;      // 统计：重排版次数
    MessageData::ChatTheme m_theme;

    static constexpr int kMaxCachedDocs = 200;  // 缓存限制
};
