#include "ContactDelegate.h"

ContactDelegate::ContactDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
}

ContactDelegate::~ContactDelegate() {
}

void ContactDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (!index.isValid())
        return;

    // 获取数据
    QString avatar   = index.data(int(ContactData::Role::Avatar  )).toString();
    QString nickname = index.data(int(ContactData::Role::NickName)).toString();
    QString message  = index.data(int(ContactData::Role::Message )).toString();

    // 保存画笔状态
    painter->save();

    // 绘制背景（选中状态）
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    // 设置边距
    const int margin = 10;
    const int avatarSize = 40;
    const int spacing = 10;

    QRect avatarRect(option.rect.left() + margin,
                     option.rect.top() + margin,
                     avatarSize,
                     avatarSize);

    QRect contactRect(avatarRect.right() + spacing,
                      option.rect.top() + margin,
                      option.rect.width() - avatarRect.width() - spacing - margin * 2,
                      option.rect.height() - margin * 2);

    // 绘制头像（使用圆形或方形背景）
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(100, 150, 200));
    painter->drawRoundedRect(avatarRect, 5, 5);

    painter->setPen(Qt::white);
    QFont avatarFont = painter->font();
    avatarFont.setPointSize(10);
    painter->setFont(avatarFont);
    painter->drawText(avatarRect, Qt::AlignCenter, avatar.left(2).toUpper());

    // 绘制昵称
    QFont nicknameFont = painter->font();
    nicknameFont.setBold(true);
    nicknameFont.setPointSize(10);
    painter->setFont(nicknameFont);
    painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : option.palette.text().color());

    QRect nicknameRect = contactRect;
    nicknameRect.setHeight(20);
    painter->drawText(nicknameRect, Qt::AlignLeft | Qt::AlignVCenter, nickname);

    // 绘制消息内容
    QFont messageFont = painter->font();
    messageFont.setBold(false);
    messageFont.setPointSize(9);
    painter->setFont(messageFont);
    painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : QColor(100, 100, 100));

    QRect messageRect = contactRect;
    messageRect.setTop(nicknameRect.bottom() + 5);
    painter->drawText(messageRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, message);

    painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // return QStyledItemDelegate::sizeHint(option, index);
    // 返回固定高度，或根据内容计算
    return QSize(0, 80);  // 高度 80 像素，宽度由 view 决定
}
