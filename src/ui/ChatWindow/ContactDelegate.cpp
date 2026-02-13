#include "ContactDelegate.h"

ContactDelegate::ContactDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
}

ContactDelegate::~ContactDelegate() {
}

void ContactDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (!index.isValid())
        return;

    // 保存画笔状态
    painter->save();

    // 绘制背景（选中状态）
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    // 设置边距
    const int margin     = 10;
    const int avatarSize = 40;
    const int spacing    = 10;

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

    // 绘制头像（使用圆形或方形背景）
    {
        QPixmap avatar_pixmap = index.data(int(ContactData::Role::Pixmap)).value<QPixmap>();
        QString avatar_path   = index.data(int(ContactData::Role::Avatar)).toString();
        QString nickname      = index.data(int(ContactData::Role::Nickname)).toString();
        if(avatar_pixmap.isNull()) {
            avatar_pixmap = QPixmap(avatar_path);
        }
        if(!avatar_pixmap.isNull()) {
            painter->drawPixmap(avatarRect, avatar_pixmap);
        }
        else {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(100, 150, 200));
            painter->drawRoundedRect(avatarRect, 5, 5);

            painter->setPen(Qt::white);
            QFont avatarFont = painter->font();
            avatarFont.setPointSize(10);
            painter->setFont(avatarFont);
            painter->drawText(avatarRect, Qt::AlignCenter, nickname.trimmed().left(2).toUpper());
        }
    }

    // 绘制昵称
    {
        QString nickname = index.data(int(ContactData::Role::Nickname)).toString();

        QFont nicknameFont = painter->font();
        nicknameFont.setBold(true);
        nicknameFont.setPointSize(10);
        painter->setFont(nicknameFont);
        painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : option.palette.text().color());

        // QRect nicknameRect = contactRect;
        // nicknameRect.setHeight(20);
        // painter->drawText(nicknameRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, nickname);
        QFontMetrics nicknameMetrics(nicknameFont);
        QString nicknameElided = nicknameMetrics.elidedText(nickname, Qt::ElideRight, nicknameRect.width());
        painter->drawText(nicknameRect, Qt::AlignLeft | Qt::AlignVCenter, nicknameElided);
    }

    // 绘制备注内容  // 单行不换行，显示不了的用省略号结尾
    {
        QString notes    = index.data(int(ContactData::Role::Notes   )).toString();

        QFont notesFont = painter->font();
        notesFont.setBold(false);
        notesFont.setPointSize(9);
        painter->setFont(notesFont);
        painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : QColor(100, 100, 100));

        // QRect notesRect = contactRect;
        // notesRect.setTop(nicknameRect.bottom() + 5);
        // painter->drawText(notesRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, notes);  // 换行导致显示不全
        // painter->drawText(notesRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, notes); // 被覆盖，无提示
        // 使用 QFontMetrics 计算省略后的文本，绘制处理后的文本
        QFontMetrics notesMetrics(notesFont);
        QString notesElided = notesMetrics.elidedText(notes, Qt::ElideRight, notesRect.width());
        painter->drawText(notesRect, Qt::AlignLeft | Qt::AlignVCenter, notesElided);
    }

    painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // return QStyledItemDelegate::sizeHint(option, index);
    // 返回固定高度，或根据内容计算
    return QSize(0, 80);  // 高度 80 像素，宽度由 view 决定
}
