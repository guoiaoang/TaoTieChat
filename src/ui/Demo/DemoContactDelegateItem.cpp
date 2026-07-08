#include "DemoContactDelegateItem.h"
#include "ui_DemoContactDelegateItem.h"

DemoContactDelegateItem::DemoContactDelegateItem(QWidget *parent)
    : QWidget(parent), m_ui(std::make_shared<Ui::DemoContactDelegateItem>()) {
    m_ui->setupUi(this);
    m_ui->label_time  ->setVisible(false);
    m_ui->label_notice->setVisible(false);
    // 头像为正方形
    QSize size_info = m_ui->layout_info->sizeHint();
    m_ui->label_avatar->resize(size_info.height(), size_info.height());
}

DemoContactDelegateItem::~DemoContactDelegateItem() { }

std::shared_ptr<DemoContactDelegateItem::StyleData> DemoContactDelegateItem::get_style_data() {
    std::shared_ptr<StyleData> style_data = std::make_shared<DemoContactDelegateItem::StyleData>();

    // 放弃 setVisible，改用清空文本占位。或者如果非要用 setVisible，必须在每次显隐后手动“强制刷新”：
    this->m_ui->label_time  ->setVisible(false);
    this->m_ui->label_notice->setVisible(false);

    // 一旦动了可见性，必须立刻强制当前小局部布局重新计算！
    if (this->layout()) {
        this->layout()->invalidate(); // 使布局失效，标记需要重新计算
        this->layout()->activate();   // 强制立刻、同步完成重新排版，不准异步滞后
    }

    auto parent_rect = [this](const QWidget* widget) {
        return QRect(widget->mapTo(this, QPoint(0, 0)), widget->rect().size());
    };

    // 此时拿到的 nickname 宽度就是绝对精准、没有滞后的 227 了！
    style_data->rect_avatar   = parent_rect(this->m_ui->label_avatar  );
    style_data->rect_nickname = parent_rect(this->m_ui->label_nickname);
    style_data->rect_message  = parent_rect(this->m_ui->label_message );

    // 恢复可见性并再次强制刷新
    this->m_ui->label_time  ->setVisible(true);
    this->m_ui->label_notice->setVisible(true);

    if (this->layout()) {
        this->layout()->invalidate();
        this->layout()->activate();
    }

    style_data->rect_time   = parent_rect(this->m_ui->label_time  );
    style_data->rect_notice = parent_rect(this->m_ui->label_notice);

    // 修改
    int avatar_hor_offset = style_data->rect_avatar.height() - style_data->rect_avatar.width();
    style_data->rect_avatar  .setWidth(style_data->rect_avatar  .height());
    style_data->rect_nickname.setLeft (style_data->rect_nickname.left  () + avatar_hor_offset);
    style_data->rect_nickname.setRight(style_data->rect_time    .right ());
    style_data->rect_message .setLeft (style_data->rect_message .left  () + avatar_hor_offset);
    style_data->rect_message .setRight(style_data->rect_notice  .right ());

    // font
    style_data->font_nickname  = this->m_ui->label_nickname->font();
    style_data->font_signature = this->m_ui->label_message ->font();
    style_data->font_time      = this->m_ui->label_time    ->font();
    return style_data;
}
