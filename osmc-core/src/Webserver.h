//
// Created by Casper Thule on 16/08/2021.
//

#ifndef OSMCPROJECT_WEBSERVER_H
#define OSMCPROJECT_WEBSERVER_H

#include "httplib.h"
#include <thread>
const std::string outOfSyncPath = "/out-of-sync";

class Webserver {

public:
    Webserver();
    void startServer();
    void RegisterOOSHandler(std::function<bool(void)> handler);
    void startServer(int port);
    bool isServerRunning();
    void stop();
    ~Webserver();


private:
    httplib::Server svr;
    const int defaultPort = 8080;
    std::function<bool(void)> externalHandler;
    std::thread _thd;
    std::atomic<bool> started = std::atomic<bool>(false);



    std::function<void(const httplib::Request &, httplib::Response &)> oosGetHandler;
};


#endif //OSMCPROJECT_WEBSERVER_H
