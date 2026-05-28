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

    auto parent_rect = [this](const QWidget*widget) {
        // 1. 拿到子控件自己的本地矩形 (x 和 y 通常为 0，即 QRect(0, 0, width, height))
        QRect rect_child = widget->rect();

        // 2. 将这个本地矩形的左上角坐标，映射到指定的父窗口（比如 this 主窗口）的坐标系中
        QPoint pos_parent = widget->mapTo(this, QPoint(0, 0));

        // 3. 重新组合成子控件在指定父窗口中的新 QRect
        QRect rect_in_parent(pos_parent, rect_child.size());

        // qDebug() << "相对于指定顶层窗口的精确范围:" << rect_in_parent;
        return rect_in_parent;
    };

    // 当前获取到的大小包含重叠，交界处需要自行计算

    this->m_ui->label_time  ->setVisible(false);
    this->m_ui->label_notice->setVisible(false);

    // style_data->rect_window   = this->rect();
    style_data->rect_avatar   = parent_rect(this->m_ui->label_avatar  );
    style_data->rect_nickname = parent_rect(this->m_ui->label_nickname);
    style_data->rect_message  = parent_rect(this->m_ui->label_message );

    this->m_ui->label_time  ->setVisible(true);
    this->m_ui->label_notice->setVisible(true);

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
