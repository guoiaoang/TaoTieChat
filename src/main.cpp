// #include "MainWindow.h"
#include "Utils.h"
#include "AuthWindow.h"
#include "ChatWindow.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE_EXPAND(PROJECT_NAME);
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName  (TOSTRING(ORGANIZATION_NAME)  );
    QCoreApplication::setOrganizationDomain(TOSTRING(ORGANIZATION_DOMAIN));
    QCoreApplication::setApplicationName   (TOSTRING(PROJECT_NAME)       );
    Utils::init_debug_output_format();
    Utils::load_translator(application);
    Utils::load_font(":/rc/fonts/InconsolataNerdFontMono-Regular.ttf");

    std::shared_ptr<AuthWindow> window_auth;
    std::shared_ptr<ChatWindow> window_chat;

    window_auth = std::make_shared<AuthWindow>();
    QObject::connect(window_auth.get(), &AuthWindow::accepted, [&]() {
        // 先关闭登陆窗口再创建主窗口，避免单线程界面卡死
        window_auth->close();
        window_auth.reset();  // 销毁登录窗口
        //
        window_chat = std::make_shared<ChatWindow>();
        QObject::connect(window_chat.get(), &ChatWindow::signout, [&]() {
            // window_chat->close();
            // window_chat.reset();  // 销毁主窗口
            // //
            // window_auth = std::make_shared<AuthWindow>();
            // window_auth->show();
            QApplication::exit(0);
        });

        window_chat->show();
    });

    window_auth->show();

    return application.exec();
}
