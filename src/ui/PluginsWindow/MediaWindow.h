#pragma once

#include "Utils.h"

#ifdef DEBUG
#include "Dev.h"
#endif // DEBUG

namespace Ui {
class MediaWindow;
}

class MediaWindow : public QMainWindow {
    Q_OBJECT
public:
    static MediaWindow &instance() {
        static MediaWindow s_media_window;
        return s_media_window;
    }
private:
    explicit MediaWindow(QWidget *parent = nullptr);
    MediaWindow              (const MediaWindow & ) = delete;
    MediaWindow              (      MediaWindow &&) = delete;
    MediaWindow & operator = (const MediaWindow & ) = delete;
    MediaWindow & operator = (      MediaWindow &&) = delete;
public:
    ~MediaWindow();

public:
    void clean();
    bool load(const QString&media_path);
    bool load(const QPixmap&media_pixmap);
    void set_media_name(const QString&media_name);
    bool update_media();
    bool update_title();

signals:
    void media_changed(const QPixmap&pixmap);

public slots:
    void on_media_open();
    void on_media_save();
    void on_media_save_as();

public:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::MediaWindow *ui             = nullptr;
    QToolBar        *m_toolbar      = nullptr;
    QMenu           *m_menu_save    = nullptr;  // 另存为
    QActionGroup    *m_action_group = nullptr;

    QAction  *m_action_open        = nullptr;  // 打开
    QAction  *m_action_save        = nullptr;  // 保存
    QAction  *m_action_save_as     = nullptr;  // 另存为
    QAction  *m_action_prev        = nullptr;  // 上一个
    QAction  *m_action_next        = nullptr;  // 下一个
    QAction  *m_action_fit         = nullptr;  // 适应窗口
    QAction  *m_action_zoom_in     = nullptr;  // 放大
    QAction  *m_action_zoom_out    = nullptr;  // 缩小
    QAction  *m_action_normal_size = nullptr;  // 原始大小
    QAction  *m_action_full_screen = nullptr;  // 全屏显示
    QAction  *m_action_pin         = nullptr;  // 窗口置顶

    QString  m_media_path  ;
    QPixmap  m_media_pixmap;
    QString  m_media_name  ;

    // ====================================================================== //
#ifdef DEBUG
private:
    friend Dev::MediaWindowDev;
#endif // DEBUG
};


// 可参考项目：
//   * Qt/Examples/Qt-5.15.2/widgets/widgets/imageviewer/imageviewer.cpp
//   * Qt/Examples/Qt-5.15.2/widgets/widgets/movie/movieplayer.cpp
