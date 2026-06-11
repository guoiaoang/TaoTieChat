#pragma once

#include "headers.h"
#include "Utils.h"

namespace Ui {
class DemoContactDelegateItem;
}

class ListDelegate;

class DemoContactDelegateItem : public QWidget {
    Q_OBJECT
public:
    explicit DemoContactDelegateItem(QWidget *parent = nullptr);
    ~DemoContactDelegateItem();
public:
    // 样式数据
    struct StyleData{
        // rect
        // QRect rect_window  ;
        QRect rect_avatar  ;
        QRect rect_nickname;
        QRect rect_message ;
        QRect rect_time    ;
        QRect rect_notice  ;
        // font
        QFont font_nickname ;
        QFont font_signature;
        QFont font_time     ;
    };
    std::shared_ptr<StyleData> get_style_data();
private:
    std::shared_ptr<Ui::DemoContactDelegateItem> m_ui;
};
