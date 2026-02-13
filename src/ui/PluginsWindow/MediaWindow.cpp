#include "MediaWindow.h"
#include "ui_MediaWindow.h"

MediaWindow::MediaWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MediaWindow) {
    ui->setupUi(this);

    // new
    m_toolbar            = new QToolBar;
    m_menu_save          = new QMenu;
    m_action_group       = new QActionGroup(this);
    // new action
    m_action_open        = new QAction;  // 打开
    m_action_save        = new QAction;  // 保存
    m_action_save_as     = new QAction;  // 另存为
    m_action_prev        = new QAction;  // 上一个
    m_action_next        = new QAction;  // 下一个
    m_action_fit         = new QAction;  // 适应窗口
    m_action_zoom_in     = new QAction;  // 放大
    m_action_zoom_out    = new QAction;  // 缩小
    m_action_normal_size = new QAction;  // 原始大小
    m_action_full_screen = new QAction;  // 全屏显示
    m_action_pin         = new QAction;  // 窗口置顶

    // text
    m_action_open        ->setText(tr("Open"         ));
    m_action_save        ->setText(tr("Save"         ));
    m_action_save_as     ->setText(tr("Save As"      ));
    m_action_prev        ->setText(tr("Prev"         ));
    m_action_next        ->setText(tr("Next"         ));
    m_action_fit         ->setText(tr("Fit To Window"));
    m_action_zoom_in     ->setText(tr("Zoom In"      ));
    m_action_zoom_out    ->setText(tr("Zoom Out"     ));
    m_action_normal_size ->setText(tr("Normal Size"  ));
    m_action_full_screen ->setText(tr("Full Screen"  ));
    m_action_pin         ->setText(tr("Pin"          ));

    // icon
    m_action_open        ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen  ));
    m_action_save        ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave  ));
    m_action_save_as     ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs));
    m_action_prev        ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious    ));
    m_action_next        ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoNext        ));
    m_action_fit         ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomFitBest   ));
    m_action_zoom_in     ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn        ));
    m_action_zoom_out    ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut       ));
    m_action_normal_size ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ViewRestore   ));
    m_action_full_screen ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ViewFullscreen));
    m_action_pin         ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoUp          ));

    // shortcut
    m_action_open       ->setShortcut(QKeySequence::Open      );
    m_action_save       ->setShortcut(QKeySequence::Save      );
    m_action_save_as    ->setShortcut(QKeySequence::SaveAs    );
    m_action_prev       ->setShortcut(QKeySequence::Back      );
    m_action_next       ->setShortcut(QKeySequence::Forward   );
    m_action_zoom_in    ->setShortcut(QKeySequence::ZoomIn    ); // Qt::CTRL | Qt::Key_Plus );  // zoom in
    m_action_zoom_out   ->setShortcut(QKeySequence::ZoomOut   ); // Qt::CTRL | Qt::Key_Minus);  // zoom out
    m_action_full_screen->setShortcut(QKeySequence::FullScreen);

    // property // Enabled
    m_action_prev->setEnabled(false);
    m_action_next->setEnabled(false);
    // // 功能未实现，先禁用
    // m_action_open        ->setEnabled(false);
    // m_action_save        ->setEnabled(false);
    // m_action_save_as     ->setEnabled(false);
    // m_action_prev        ->setEnabled(false);
    // m_action_next        ->setEnabled(false);
    // m_action_fit         ->setEnabled(false);
    // m_action_zoom_in     ->setEnabled(false);
    // m_action_zoom_out    ->setEnabled(false);
    // m_action_normal_size ->setEnabled(false);
    // m_action_full_screen ->setEnabled(false);
    // m_action_pin         ->setEnabled(false);
    // property // Checkable
    m_action_fit        ->setCheckable(true);
    m_action_zoom_in    ->setCheckable(true);
    m_action_zoom_out   ->setCheckable(true);
    m_action_normal_size->setCheckable(true);
    m_action_full_screen->setCheckable(true);
    m_action_pin        ->setCheckable(true);
    // property // Checked
    m_action_fit->setChecked(true);

    // add to group
    m_action_group->addAction(m_action_fit        );
    m_action_group->addAction(m_action_zoom_in    );
    m_action_group->addAction(m_action_zoom_out   );
    m_action_group->addAction(m_action_normal_size);

    // action saveas  // 另存为按钮作为扩展按钮 - 保存按钮的菜单
    m_menu_save->addAction(m_action_save_as);
    m_action_save->setMenu(m_menu_save);
    // m_menu_save->setStyle();
    m_action_open       ->setMenuRole(QAction::ApplicationSpecificRole);
    m_action_full_screen->setMenuRole(QAction::TextHeuristicRole);

    // add to toolbar
    m_toolbar->addAction (m_action_open            );
    m_toolbar->addAction (m_action_save            ); m_toolbar->addSeparator();
    m_toolbar->addAction (m_action_prev            );
    m_toolbar->addAction (m_action_next            ); m_toolbar->addSeparator();
    m_toolbar->addActions(m_action_group->actions()); m_toolbar->addSeparator();
    m_toolbar->addAction (m_action_full_screen     );
    m_toolbar->addAction (m_action_pin             );
    // toolbar options
    m_toolbar->setWindowTitle(tr("Toolbar"));
    m_toolbar->setMovable(false);

    // add toolbar to window
    this->addToolBar(m_toolbar);

    connect(this, &MediaWindow::media_changed, ui->label, &QLabel::setPixmap);
    ui->label->setAlignment(Qt::AlignCenter);
    // 设置 label 的尺寸策略，使其能够收缩以避免出现滚动条
    ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    // 不自动缩放内容
    ui->label->setScaledContents(false);
    // ui->label->installEventFilter(this);
    // ui->layout_central->installEventFilter(this);
    ui->scrollArea->viewport()->installEventFilter(this);
    // ui->scrollArea->setWidgetResizable(true);

// #ifdef DEBUG
//     Dev::MediaWindowDev(this, parent);
// #endif // DEBUG
}

MediaWindow::~MediaWindow() {
    if (ui                   != nullptr) { delete ui                  ; ui                   = nullptr; }
    if (m_toolbar            != nullptr) { delete m_toolbar           ; m_toolbar            = nullptr; }
    if (m_menu_save          != nullptr) { delete m_menu_save         ; m_menu_save          = nullptr; }
    if (m_action_group       != nullptr) { delete m_action_group      ; m_action_group       = nullptr; }

    if (m_action_open        != nullptr) { delete m_action_open       ; m_action_open        = nullptr; }
    if (m_action_save        != nullptr) { delete m_action_save       ; m_action_save        = nullptr; }
    if (m_action_save_as     != nullptr) { delete m_action_save_as    ; m_action_save_as     = nullptr; }
    if (m_action_prev        != nullptr) { delete m_action_prev       ; m_action_prev        = nullptr; }
    if (m_action_next        != nullptr) { delete m_action_next       ; m_action_next        = nullptr; }
    if (m_action_fit         != nullptr) { delete m_action_fit        ; m_action_fit         = nullptr; }
    if (m_action_zoom_in     != nullptr) { delete m_action_zoom_in    ; m_action_zoom_in     = nullptr; }
    if (m_action_zoom_out    != nullptr) { delete m_action_zoom_out   ; m_action_zoom_out    = nullptr; }
    if (m_action_normal_size != nullptr) { delete m_action_normal_size; m_action_normal_size = nullptr; }
    if (m_action_pin         != nullptr) { delete m_action_pin        ; m_action_pin         = nullptr; }
}

void MediaWindow::clean() {
    // setWindowFilePath(QString());
    m_media_path   = QString();
    m_media_pixmap = QPixmap();
    m_media_name   = QString();
    update_title();
}

bool MediaWindow::load(const QString&media_path) {
    QPixmap pixmap(media_path);
    if(load(pixmap)) {
        m_media_path = media_path;
        return true;
    }
    return false;
}

bool MediaWindow::load(const QPixmap&media_pixmap) {
    clean();
    m_media_pixmap = media_pixmap;
    emit media_changed(media_pixmap);
    update_media();
    update_title();

    if(!media_pixmap.isNull()) {
        return true;
    }
    return false;
}

void MediaWindow::set_media_name(const QString&media_name) {
    m_media_name = media_name;
    update_title();
}

bool MediaWindow::update_media() {
    // 更新缩放图片：使其适应 Label 大小，并保持纵横比
    // Qt::KeepAspectRatio 保证图片不变形
    // Qt::SmoothTransformation 保证缩放后的图片清晰（消除锯齿）
    QPixmap pixmap_scaled = m_media_pixmap.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap_scaled);
    return true;

    // // 更新缩放图片：使其适应 ScrollArea 的 viewport 大小，并保持纵横比
    // // 使用 scrollArea->viewport()->size() 而不是 label->size()
    // // 这样当窗口缩小时，图片也会相应缩小
    // // Qt::KeepAspectRatio 保证图片不变形
    // // Qt::SmoothTransformation 保证缩放后的图片清晰（消除锯齿）
    // QSize viewport_size = ui->scrollArea->viewport()->size();
    // QPixmap pixmap_scaled = m_media_pixmap.scaled(viewport_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // ui->label->setPixmap(pixmap_scaled);
    // ui->label->resize(pixmap_scaled.size()); // 调整 label 大小以匹配图片
    // return true;
}

bool MediaWindow::update_title() {
    QPixmap label_pixmap = ui->label->pixmap();
    if(!m_media_name.isEmpty()) {
        this->setWindowTitle(label_pixmap.isNull() ? tr("%1 : Load failed!").arg(m_media_name) : m_media_name);
    }
    else if(m_media_path.isEmpty()) {
        this->setWindowTitle(label_pixmap.isNull() ? tr("%1 : Load failed!").arg(m_media_path) : m_media_path);
    }
    else {
        this->setWindowTitle(label_pixmap.isNull() ? QApplication::applicationName() : tr("Pixmap"));
    }
    return true;
}

bool MediaWindow::eventFilter(QObject *watched, QEvent *event) {
    if(event->type() == QEvent::Resize) {
        if(watched == ui->label) {
            update_media();
        }
        else if(watched == ui->scrollArea->viewport()) {
            update_media();
        }
    }
    return QMainWindow::eventFilter(watched, event);
}
