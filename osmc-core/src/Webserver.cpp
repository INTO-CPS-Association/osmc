#include <iostream>
#include "Webserver.h"
#include <atomic>

Webserver::Webserver() {
}

void Webserver::RegisterOOSHandler(std::function<bool(void)> handler) {
    this->svr.Get(outOfSyncPath, [handler](const httplib::Request &, httplib::Response &res) {
        res.set_content("OOS: " + std::string(handler() ? "true" : "false"), "text/plain");
    });
}

void Webserver::startServer() {

    this->startServer(this->defaultHostname, this->defaultPort);
}

void Webserver::startServer(std::string hostname, int port) {
    this->hostname = hostname;
    this->port = port;

    std::function<void(const httplib::Request &, httplib::Response &res)> handler;
    if(this->externalHandler)
        handler = [handlerLambda = this->externalHandler](const httplib::Request &, httplib::Response &res) {
            res.set_content("OOS: " + std::string(handlerLambda() ? "true" : "false"), "text/plain");};
    else
        handler = [](const httplib::Request &, httplib::Response &res) {
            res.set_content("Hello World!", "text/plain");};

    this->oosGetHandler = handler;

    this -> _thd = std::thread([this, hostname, port]()
                       {
                            svr.listen(hostname.c_str(), port);
                       });
}

bool Webserver::isServerRunning() {
    return this->_thd.joinable();
}

void Webserver::stop(){
    if(this->_thd.joinable()){
        svr.stop();
        this->_thd.join();
    }
}

Webserver::~Webserver() {
    this->stop();
}



