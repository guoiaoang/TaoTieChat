#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AuthWindow.h"

// 由于 `QStackedWidget::addWidget()` 会修改子窗口大小，
// 为保障子窗口大小不被意外修改，改为 QStackedWidget 最多只持有当前子窗口，
// 且先将主窗口大小设为子窗口大小，这样在拉伸主窗口时可以有效的修改子窗口的大小，
// 同时 QStackedWidget 不会影响其他子窗口的大小

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    // this->installEventFilter(this);  // 用于调试
    ui->setupUi(this);
    menuBar()->hide();
    statusBar()->hide();

    // // 1.创建
    m_auth_window = new AuthWindow(this);
    m_chat_window = new ChatWindow(this);

    // // 2.连接
    // 根据子窗口来调整父窗口的大小和标题等内容
    connect(ui->main_stacked, &QStackedWidget::currentChanged, this, [&](){ this->on_show_widget_change(); });
    connect(m_auth_window   , &AuthWindow    ::accepted      , this, [&](){ this->show_chat_window     (); });
    connect(m_chat_window   , &ChatWindow    ::accepted      , this, [&](){ this->show_auth_window     (); });  // 用于测试页面切换
    connect(m_chat_window   , &ChatWindow    ::signout       , this, [&](){ this->show_auth_window     (); });  // 退出登录

    // // 3.设置
    // m_auth_window->installEventFilter(this);  // debug
    // m_chat_window->installEventFilter(this);  // debug
    m_auth_window->setMinimumSize(1,1); // 设置为 (0,0) 并不能缩到最小
    m_chat_window->setMinimumSize(1,1);
    // 先调整主窗口大小再添加显示
    // 默认显示登陆页面
    this->resize(m_auth_window->size());
    ui->main_stacked->addWidget(m_auth_window);
    ui->main_stacked->setCurrentWidget(m_auth_window);

    // TODO:登陆检查
    QTimer::singleShot(0, this, &MainWindow::on_init);

    // // 4.显示
    // this->resize(600, 400);
}

MainWindow::~MainWindow() {
    delete ui;
    ui = nullptr; // 必须置空
    if(m_auth_window) {
        delete m_auth_window;
        m_auth_window = nullptr;
    }
    if(m_chat_window) {
        delete m_chat_window;
        m_chat_window = nullptr;
    }
}

// 用于调试
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    qDebug() << " className():" << obj->metaObject()->className()
             << " event->type():" << QMetaEnum::fromType<QEvent::Type>().valueToKey(event->type());
    return QObject::eventFilter(obj, event);
}

void MainWindow::on_init() {
    // 检查本地是否有可用的登陆记录并执行自动登陆
    if(!this->check_login()) {
        show_auth_window();
    }
}

void MainWindow::on_show_widget_change() {
    if(!ui || !ui->main_stacked || !ui->main_stacked->currentWidget()) {
        return;  // 销毁时会触发
    }
    QWidget *widget = ui->main_stacked->currentWidget();
    // qDebug() << widget->metaObject()->className() << " size : " << widget->size();  // debug
    // qDebug() << this  ->metaObject()->className() << " size : " << this  ->size();  // debug
    // this->resize(widget->size());  // 这里设置就晚了
    this->setWindowTitle   (widget->windowTitle   ());
    this->setWindowIcon    (widget->windowIcon    ());
    this->setWindowIconText(widget->windowIconText());
    connect(widget, &QWidget::windowTitleChanged   , this, [&](const QString &title   ){ this->setWindowTitle   (title   ); });
    connect(widget, &QWidget::windowIconChanged    , this, [&](const QIcon   &icon    ){ this->setWindowIcon    (icon    ); });
    connect(widget, &QWidget::windowIconTextChanged, this, [&](const QString &iconText){ this->setWindowIconText(iconText); });
    // connect(widget, &QWidget::customContextMenuRequested, this, [&](const QPoint &pos){this->setWindowIcon(icon);});
}

bool MainWindow::check_login() {
    return false;  // 无有效的登陆信息，需要用户手动登陆
}

bool MainWindow::show_window(QWidget*widget) {
    if(!widget) {
        return false;
    }

    // TODO:切换窗口时旧窗口会先闪烁一下再出现目标窗口

    // 保存旧窗口的位置及大小，用于还原
    static QWidget *widget_old = nullptr;
    static QMap<QWidget*, QRect> map_main_window_geometry;
    if(widget_old != nullptr && map_main_window_geometry.contains(widget_old)) {
        // 如果已经保存那么在切换窗口前进行更新
        map_main_window_geometry[widget_old] = this->geometry();
    }

    // 先清除已经添加的其他子窗口，防止意外的更改
    // 先添加到 stacked 会导致 widget 大小被修改
    while(ui->main_stacked->count()) {
        ui->main_stacked->removeWidget(ui->main_stacked->widget(0));
    }

    // 调整为适合的大小
    // 初始窗口居中显示，非初始窗口使用之前的位置和大小
    {
        QWidget*widget_temp = new QWidget;
        widget_temp->setMinimumSize(widget->size());
        ui->main_stacked->addWidget(widget_temp);
        this->adjustSize();
        if(!map_main_window_geometry.contains(widget)) {
            // 如果之前未保存，那么这个窗口就是初始化窗口，对初始化窗口居中显示
            this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), qApp->primaryScreen()->availableGeometry()));
            // 已经初始化的窗口存入列表，使得切换窗口前可以被更新
            map_main_window_geometry.insert(widget, this->geometry());
        }
        else {
            // 如果之前已经保存了那么就恢复之前的位置和大小
            this->setGeometry(map_main_window_geometry[widget]);
        }
        ui->main_stacked->removeWidget(widget_temp);
        widget_temp->deleteLater();
    };

    // 添加并显示当前窗口内容
    ui->main_stacked->addWidget(widget);
    ui->main_stacked->setCurrentWidget(widget);

    // 存入为之前显示的窗口
    widget_old = widget;

    return true;

    // 始终无法解决闪烁问题时可能需要切换主窗口
    if(0)
    {
        this->hide();
        static QVector<QWidget*> widget_all;
        if(!widget_all.contains(widget)) {
            widget_all.append(widget);
        }
        for(auto w:widget_all) {
            if(w != widget) {
                w->hide();
            }
        }
        widget->setWindowFlag(Qt::Dialog);
        QSize widget_size = widget->size();
        QSize screen_size = qApp->primaryScreen()->availableGeometry().size();
        widget->move((screen_size.width() - widget_size.width()) / 2, (screen_size.height() - widget_size.height()) / 2);
        widget->show();
    }
    return true;
}

bool MainWindow::show_auth_window() {
    return show_window(m_auth_window);
}

bool MainWindow::show_chat_window() {
    return show_window(m_chat_window);
}
