#pragma once

#include "Headers.h"
#include "Utils.h"

class NetworkClient : public QObject{
    Q_OBJECT
private:
    NetworkClient() = default;
    NetworkClient(const NetworkClient&) = delete;
public:
    static NetworkClient& instance() {
        static NetworkClient s_network_client;
        return s_network_client;
    }

    void set_server_url(const QUrl&url) { m_url_server = url; }
    QUrl server_url() const { return m_url_server; }
    bool init();

public:
    bool signup(const signup_params& params_signup, const signup_callback& callback);
    bool signin(const signin_params& params_signin, const signin_callback& callback);
    // 精准匹配，不返回包含查询词的内容
    bool search_user (const search_user_params&params_search_user, const search_user_callback&callback);
    bool search_group(const QString&group) { return false; }
    //
    bool update_profile(const update_profile_params&params_update_profile, const update_profile_callback&callback);

private:
    bool m_inited = false;
    QUrl m_url_server;
    std::unique_ptr<AccountClient    > m_client_account;
    std::unique_ptr<SearchClient     > m_client_search ;
    std::shared_ptr<QGrpcHttp2Channel> m_channel       ;
};
