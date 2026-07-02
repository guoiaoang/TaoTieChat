#include "MediaWindow.h"
#include "ui_MediaWindow.h"

MediaWindow::MediaWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(std::make_shared<Ui::MediaWindow>()) {
    m_ui->setupUi(this);

    // new
    m_toolbar       = std::make_shared<QToolBar    >(    );
    m_menu_save     = std::make_shared<QMenu       >(    );
    m_actions_scale = std::make_shared<QActionGroup>(this);

    // 初始化需要的图标
    m_icons.reset();
    m_icons.color.normal   = this->palette().color(QPalette::Normal  , QPalette::WindowText);  // 常规状态图标颜色
    m_icons.color.disabled = this->palette().color(QPalette::Disabled, QPalette::WindowText);  // 禁用状态图标颜色
    m_icons.update();

    auto init_action = [this](
        std::shared_ptr<QAction>& action,
        const QIcon& icon,  const QString& text,
        const QKeySequence& shortcut,
        bool visible  , bool enabled,
        bool checkable, bool checked
    ) {
        if(!action) { action = std::make_shared<QAction>(); }
        action->setIcon     (icon     );  // 设置动作图标
        action->setText     (text     );  // 设置动作文本
        action->setShortcut (shortcut );  // 设置动作快捷键
        action->setVisible  (visible  );  // 根据需要显示或隐藏动作
        action->setEnabled  (enabled  );  // 根据需要启用或禁用动作
        action->setCheckable(checkable);  // 设置动作是否可选中
        action->setChecked  (checked  );  // 设置动作的选中状态
    };

    init_action(m_action_media_open        , m_icons.icon.folder_open  , tr("Open"         ), QKeySequence(QKeySequence::Open      ), true, true , false, false);  // 打开
    init_action(m_action_media_save        , m_icons.icon.save         , tr("Save"         ), QKeySequence(QKeySequence::Save      ), true, true , false, false);  // 保存
    init_action(m_action_media_save_as     , m_icons.icon.save_edit    , tr("Save As"      ), QKeySequence(QKeySequence::SaveAs    ), true, true , false, false);  // 另存为
    init_action(m_action_media_prev        , m_icons.icon.arrow_left   , tr("Prev"         ), QKeySequence(QKeySequence::Back      ), true, false, false, false);  // 上一个
    init_action(m_action_media_next        , m_icons.icon.arrow_right  , tr("Next"         ), QKeySequence(QKeySequence::Forward   ), true, false, false, false);  // 下一个
    init_action(m_action_media_fit         , m_icons.icon.fit_to_screen, tr("Fit To Window"), QKeySequence(                        ), true, true , true , true );  // 适应窗口
    init_action(m_action_media_zoom_in     , m_icons.icon.zoom_in      , tr("Zoom In"      ), QKeySequence(QKeySequence::ZoomIn    ), true, false, true , false);  // 放大
    init_action(m_action_media_zoom_out    , m_icons.icon.zoom_out     , tr("Zoom Out"     ), QKeySequence(QKeySequence::ZoomOut   ), true, false, true , false);  // 缩小
    init_action(m_action_media_normal_size , m_icons.icon.restore      , tr("Normal Size"  ), QKeySequence(                        ), true, true , true , false);  // 原始大小
    init_action(m_action_window_full_screen, m_icons.icon.fullscreen   , tr("Full Screen"  ), QKeySequence(QKeySequence::FullScreen), true, true , true , false);  // 全屏显示
    init_action(m_action_window_pin        , m_icons.icon.pin          , tr("Pin"          ), QKeySequence(                        ), true, true , true , false);  // 窗口置顶

    QObject::connect(m_action_media_open        .get(), &QAction::triggered, this, &MediaWindow::on_media_open        );
    QObject::connect(m_action_media_save        .get(), &QAction::triggered, this, &MediaWindow::on_media_save        );
    QObject::connect(m_action_media_save_as     .get(), &QAction::triggered, this, &MediaWindow::on_media_save_as     );
    QObject::connect(m_action_media_prev        .get(), &QAction::triggered, this, &MediaWindow::on_media_prev        );
    QObject::connect(m_action_media_next        .get(), &QAction::triggered, this, &MediaWindow::on_media_next        );
    QObject::connect(m_action_media_fit         .get(), &QAction::triggered, this, &MediaWindow::on_media_fit         );
    QObject::connect(m_action_media_zoom_in     .get(), &QAction::triggered, this, &MediaWindow::on_media_zoom_in     );
    QObject::connect(m_action_media_zoom_out    .get(), &QAction::triggered, this, &MediaWindow::on_media_zoom_out    );
    QObject::connect(m_action_media_normal_size .get(), &QAction::triggered, this, &MediaWindow::on_media_normal_size );
    QObject::connect(m_action_window_full_screen.get(), &QAction::triggered, this, &MediaWindow::on_window_full_screen);
    QObject::connect(m_action_window_pin        .get(), &QAction::triggered, this, &MediaWindow::on_window_pin        );

    // static inline QMetaObject::Connection QObject::connect<...>(const QAction *sender, void (QAction::*signal)(bool checked), const MediaWindow *context, void (MediaWindow::*&&slot)(), Qt::ConnectionType type = Qt::AutoConnection)
    // connect(m_action_open.get(), &QAction::triggered, this, &MediaWindow::on_media_open);

    auto make_separator = [this]() -> QAction * {
        QAction *action = new QAction(this);
        action->setSeparator(true);
        return action;
    };
    const QList<QAction *> actions_scale{
        m_action_media_fit         .get(),
        m_action_media_zoom_in     .get(),
        m_action_media_zoom_out    .get(),
        m_action_media_normal_size .get()
    };
    for(auto&action : actions_scale) {
        m_actions_scale->addAction(action);
    }
    // add to toolbar
    const QList<QAction *> toolbar_actions{
        // m_action_open           .get(),
        // m_action_save           .get(),
        // make_separator()        .get(),
        m_action_media_save_as     .get(),
        make_separator()                 ,
        m_action_media_prev        .get(),
        m_action_media_next        .get(),
        make_separator()                 ,
        m_action_media_fit         .get(),
        m_action_media_zoom_in     .get(),
        m_action_media_zoom_out    .get(),
        m_action_media_normal_size .get(),
        make_separator()                 ,
        m_action_window_full_screen.get(),
        m_action_window_pin        .get()
    };
    m_toolbar->addActions(toolbar_actions);

    // action saveas  // 另存为按钮作为扩展按钮 - 保存按钮的菜单
    m_menu_save        ->addAction(m_action_media_save_as.get());
    m_action_media_save->setMenu  (m_menu_save           .get());
    // m_menu_save->setStyle();
    m_action_media_open        ->setMenuRole(QAction::ApplicationSpecificRole);
    m_action_window_full_screen->setMenuRole(QAction::TextHeuristicRole      );

    // toolbar options
    m_toolbar->setWindowTitle(tr("Toolbar"));
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize(QSize(32, 32)); // 这里设置显示的图标大小
    m_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);  // 不显示右键菜单
    // add toolbar to window
    this->addToolBar(m_toolbar.get());

    connect(this, &MediaWindow::media_changed, m_ui->label, &QLabel::setPixmap);
    m_ui->label->setAlignment(Qt::AlignCenter);
    // 设置 label 的尺寸策略，使其能够收缩以避免出现滚动条
    m_ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    // 不自动缩放内容
    m_ui->label->setScaledContents(false);
    // m_ui->label->installEventFilter(this);
    // m_ui->layout_central->installEventFilter(this);
    m_ui->scrollArea->viewport()->installEventFilter(this);
    // m_ui->scrollArea->setWidgetResizable(true);

    // 窗口最大化、还原时触发图片自动拉伸
    this->installEventFilter(this);

    // TODO:功能已经支持，可以启用。但放大时滚动条功能目前暂未实现。
    m_action_media_zoom_in ->setEnabled(true);
    m_action_media_zoom_out->setEnabled(true);
}

MediaWindow::~MediaWindow() {
}

// TODO: 简化，同源文件缩放
void MediaWindow::show_media(const std::shared_ptr<MediaInfo>&media_info) {
    QString str_title_right = QApplication::applicationName() + "Media View";
    if(!media_info) {
        // reset
        m_ui->label->setPixmap(QPixmap());
        this->setWindowTitle(str_title_right);
        return;
    }

    QPixmap media_pixmap = media_info->media_pixmap;
    if(media_pixmap.isNull()) {
        media_pixmap.load(media_info->media_path);
    }

    if(m_fit_to_window) {
        // 更新缩放图片：使其适应 Label 大小，并保持纵横比
        // Qt::KeepAspectRatio 保证图片不变形
        // Qt::SmoothTransformation 保证缩放后的图片清晰（消除锯齿）
        QPixmap pixmap_scaled = media_pixmap.scaled(m_ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_ui->label->setPixmap(pixmap_scaled);

        // 同步更新缩放比例，防止默认在原图比例上进行缩放
        // 计算宽度和高度各自的缩放比例
        double scale_w = static_cast<double>(m_ui->label->width()) / media_pixmap.width();
        double scale_h = static_cast<double>(m_ui->label->height()) / media_pixmap.height();
        // KeepAspectRatio 模式下，实际缩放倍数是两者中的【最小值】
        m_scale_factory = std::min(scale_w, scale_h);
    }
    else {
        // 检测精度
        const double ff = 0.001;
        if(m_scale_factory > 1.0 - ff && m_scale_factory < 1.0 + ff) {
            // 如果缩放倍数为 1 附近的值，那么直接显示原图，避免不必要的缩放计算和可能的模糊
            m_ui->label->setPixmap(media_pixmap);
        }
        else {
            // 缩放倍数不为 1 时需要进行处理
            QPixmap pixmap_scaled = media_pixmap.scaled(media_pixmap.size() * m_scale_factory, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_ui->label->setPixmap(pixmap_scaled);
        }
    }

    // title
    auto play_list_count = m_media_infos.size();
    QString str_media_title = media_info->media_path;
    if(str_media_title.isEmpty()) {
        str_media_title = media_info->media_name;
    }
    QString str_index_count;
    if(m_media_infos.size() > 1) {
        str_index_count == QString("[%1 / %2]").arg(m_media_current_index).arg(play_list_count);
    }
    QString str_title_left;
    if(!str_media_title.isEmpty()) {
        str_title_left = str_index_count + str_media_title + " - ";
    }
    this->setWindowTitle(str_title_left + str_title_right);
    return;

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

void MediaWindow::set_play_list(const std::vector<std::shared_ptr<MediaInfo>>&play_list) {
    m_media_infos = play_list;
    m_media_current_index = m_media_infos.empty() ? -1 : 0;
    std::shared_ptr<MediaInfo> media_info;
    if( m_media_current_index >= 0) {
        media_info = m_media_infos.at(m_media_current_index);
    }
    show_media(media_info);
}

const std::vector<std::shared_ptr<MediaInfo>> MediaWindow::get_play_list() const {
    return m_media_infos;
}

void MediaWindow::on_media_open() {
    QFileDialog dialog(this, tr("Open File"));
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;

    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave) {
        dialog.setDefaultSuffix("jpg");
    }

    auto loadFile = [this](const QString &fileName) {
        QPixmap pixmap(fileName);
        if(pixmap.isNull()) {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(), tr("Cannot load %1").arg(QDir::toNativeSeparators(fileName)));
            return false;
        }

        set_play_list({ std::make_shared<MediaInfo>(MediaInfo{.media_path = fileName})});

        const QString message = tr(R"(Opened "%1", %2x%3, Depth: %4)").arg(QDir::toNativeSeparators(fileName)).arg(pixmap.width()).arg(pixmap.height()).arg(pixmap.depth());
        statusBar()->showMessage(message);
        return true;
    };

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void MediaWindow::on_media_save() {
    // QFileInfo file_info(m_media_path);
}

void MediaWindow::on_media_save_as() {
    QFileDialog dialog(this, tr("Save File As"));
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptSave;

    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave) {
        dialog.setDefaultSuffix("jpg");
    }

    auto saveFile = [this](const QString &fileName) {
        QPixmap pixmap;
        if(!m_media_infos.empty() && m_media_current_index >= 0 && m_media_current_index <= int64_t(m_media_infos.size())) {
            m_media_infos.at(m_media_current_index)->media_pixmap;
        }
        if(pixmap.isNull()) {
            pixmap = m_ui->label->pixmap();
        }
        if(!pixmap.save(fileName)) {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(), tr("Cannot write %1").arg(QDir::toNativeSeparators(fileName)));
            return false;
        }
        const QString message = tr(R"(Wrote "%1")").arg(QDir::toNativeSeparators(fileName));
        statusBar()->showMessage(message);
        return true;
    };

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

void MediaWindow::on_media_prev() {
    if(m_media_current_index > 0) {
        m_media_current_index--;
        show_media(m_media_infos.at(m_media_current_index));
    }
}

void MediaWindow::on_media_next() {
    if(m_media_current_index < int64_t(m_media_infos.size()) - 1) {
        m_media_current_index++;
        show_media(m_media_infos.at(m_media_current_index));
    }
}

void MediaWindow::on_media_fit(bool checked) {
    m_fit_to_window = true;
    show_media(m_media_infos.at(m_media_current_index));
}

void MediaWindow::on_media_zoom_in() {
    m_fit_to_window = false;
    m_scale_factory *= 1.25;
    show_media(m_media_infos.at(m_media_current_index));
}

void MediaWindow::on_media_zoom_out() {
    m_fit_to_window = false;
    m_scale_factory /= 1.25;
    show_media(m_media_infos.at(m_media_current_index));
}

void MediaWindow::on_media_normal_size () {
    m_fit_to_window = false;
    m_scale_factory = 1.0;
    show_media(m_media_infos.at(m_media_current_index));
}

void MediaWindow::on_window_full_screen(bool checked) {
    // 用于恢复之前的窗口大小状态
    static auto window_state_old = this->windowState();
    if(m_action_window_full_screen->isChecked()) {
        // 更新保存之前的窗口大小状态
        window_state_old = this->windowState();
        // 执行命令
        this->showFullScreen();
        m_action_window_full_screen ->setIcon(m_icons.icon.window_restore);
    }
    else {
        // 还原窗口状态
        if(window_state_old == Qt::WindowNoState) {
            this->showNormal();
        }
        else if(window_state_old == Qt::WindowMinimized) {
            this->showMinimized();
        }
        else if(window_state_old == Qt::WindowMaximized) {
            // TODO: QT 的 showFullScreen() 状态到 showMaximized() 状态，会先恢复常规大小后再最大化
            this->showMaximized();
        }
        else if(window_state_old == Qt::WindowFullScreen) {
            this->showFullScreen();
        }
        m_action_window_full_screen ->setIcon(m_icons.icon.fullscreen);
    }
}

void MediaWindow::on_window_pin(bool checked) {
    if(m_action_window_pin->isChecked()) {
        this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        m_action_window_pin ->setIcon(m_icons.icon.pin_off);
    }
    else {
        this->setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        m_action_window_pin ->setIcon(m_icons.icon.pin);
    }
    show(); // 必须调用 show() 重新刷新窗口状态
}

bool MediaWindow::eventFilter(QObject *watched, QEvent *event) {
    if(event->type() == QEvent::Resize) {
        if(watched == m_ui->label) {
            show_media(m_media_infos.at(m_media_current_index));
        }
        else if(watched == m_ui->scrollArea->viewport()) {
            show_media(m_media_infos.at(m_media_current_index));
        }
    }
    else if(event->type() == QEvent::WindowStateChange) {
        // 窗口最大化与还原时可能需要手动触发图片缩放的更新
        show_media(m_media_infos.at(m_media_current_index));
    }
    return QMainWindow::eventFilter(watched, event);
}
