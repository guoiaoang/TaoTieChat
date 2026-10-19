// #include "MainWindow.h"
#include "Utils.h"
#include "AuthWindow.h"
#include "ChatWindow.h"
#include "SignalInner.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE_EXPAND(PROJECT_NAME);
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName  (TOSTRING(ORGANIZATION_NAME)  );
    QCoreApplication::setOrganizationDomain(TOSTRING(ORGANIZATION_DOMAIN));
    QCoreApplication::setApplicationName   (TOSTRING(PROJECT_NAME)       );
    Utils::init_debug_output_format();
    Utils::load_translator(application);
    Utils::load_font(":/rc/fonts/InconsolataNerdFontMono-Regular.ttf");

    AuthWindow window_auth;
    ChatWindow window_chat;
    QObject::connect(SignalInner::instance(), &SignalInner::accepted, [&](){
        window_auth.hide();
        window_chat.reinit();  // 重新初始化界面及数据
        window_chat.show();
    });
    QObject::connect(SignalInner::instance(), &SignalInner::signout, [&](){
        window_auth.view();
        window_chat.hide();
    });
    window_auth.view();

    return application.exec();
}
