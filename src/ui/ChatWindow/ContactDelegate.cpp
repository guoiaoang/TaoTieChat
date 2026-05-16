#include "ContactDelegate.h"
#include "Utils.h"

ContactDelegate::ContactDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
}

ContactDelegate::~ContactDelegate() {
}

void ContactDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (!index.isValid())
        return;

    int height = get_height();  // 高度智能由外部传入，不能在 const 函数中传出

    // 保存画笔状态
    painter->save();

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

    // 设置边距
    const int margin     = 10;
    const int avatarSize = 40;
    const int spacing    = 10;

    // 获取各种位置
    QRect avatarRect(option.rect.left() + margin,
                     option.rect.top() + margin,
                     avatarSize,
                     avatarSize);

    QRect contactRect(avatarRect.right() + spacing,
                      option.rect.top() + margin,
                      option.rect.width() - avatarRect.width() - spacing - margin * 2,
                      option.rect.height() - margin * 2);

    QRect nicknameRect = contactRect;
    nicknameRect.setHeight(20);

    QRect notesRect = contactRect;
    notesRect.setTop(nicknameRect.bottom() + 5);

    const std::shared_ptr<ContactInfo>&contact_info = index.data(Qt::UserRole).value<std::shared_ptr<ContactInfo>>();
    if(contact_info) {
        // 获取显示内容
        QString nickname      = contact_info->baseInfo().nickname();  // 昵称
        QString user_id       = contact_info->baseInfo().userId  ();  // 用户 ID
        QString notes         = contact_info->hasFriendInfo() ? contact_info->friendInfo().notes() : QString();  // 备注
        QPixmap avatar_pixmap;  // = contact_info->avatarPixmap();  // 头像图像
        QString avatar_path  ;  // = contact_info->avatarPath();  // 头像路径
        QString str_name = !nickname.isEmpty() ? nickname : user_id;
        QPixmap avatar   = !avatar_pixmap.isNull() ? avatar_pixmap : QPixmap(avatar_path);
        avatar = !avatar.isNull() ? avatar : Utils::to_pixmap(str_name);

        // 绘制头像
        painter->drawPixmap(avatarRect, Utils::to_round_pixmap(avatar, avatar.size().width() / 2));  // 圆角头像

        // 绘制昵称
        {
            QFont nicknameFont = painter->font();
            nicknameFont.setBold(true);
            nicknameFont.setPointSize(10);
            painter->setFont(nicknameFont);
            painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : option.palette.text().color());

            // 使用 QFontMetrics 计算省略后的文本，绘制处理后的文本
            QFontMetrics nicknameMetrics(nicknameFont);
            QString nicknameElided = nicknameMetrics.elidedText(str_name, Qt::ElideRight, nicknameRect.width());
            painter->drawText(nicknameRect, Qt::AlignLeft | Qt::AlignVCenter, nicknameElided);
        }

        // 绘制备注内容  // 单行不换行，显示不了的用省略号结尾
        {
            QFont notesFont = painter->font();
            notesFont.setBold(false);
            notesFont.setPointSize(9);
            painter->setFont(notesFont);
            painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : QColor(100, 100, 100));

            // 使用 QFontMetrics 计算省略后的文本，绘制处理后的文本
            QFontMetrics notesMetrics(notesFont);
            QString notesElided = notesMetrics.elidedText(notes, Qt::ElideRight, notesRect.width());
            painter->drawText(notesRect, Qt::AlignLeft | Qt::AlignVCenter, notesElided);
        }
    }

    painter->restore();

    // set_height(80);  // 大概高度  // paint 函数是 `void paint(...) const override`,因此需要在 sizeHint 中就计算好高度
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // return QStyledItemDelegate::sizeHint(option, index);
    // 返回固定高度，或根据内容计算
    // 动态高度需要在当前函数中计算完成再设置，无法在 paint 函数中设置
    return QSize(0, get_height());  // 宽度由 view 决定
}
