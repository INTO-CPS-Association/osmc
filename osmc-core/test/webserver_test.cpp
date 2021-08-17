//
// Created by Casper Thule on 16/08/2021.
//
#include "gtest/gtest.h"

#include "FmuContainerCore.h"
#include <chrono>
#include "HTTPRequest.hpp"
using namespace std;

namespace {
    TEST(CoreTest, Webserver
    ) {
        cout << " Webserver test" << endl;

        atomic<bool> oosWebserverHandlerCalled = atomic<bool>(false);
        std::function<bool()> webserverOOSHandler = [&oosWebserverHandlerCalled]() {
            std::cout << "Handler called" << std::endl;
            oosWebserverHandlerCalled.store(true);
            return false;
        };

        Webserver webserver;
        ASSERT_FALSE(webserver.isServerRunning());
        int port = 1234;
        webserver.RegisterOOSHandler(webserverOOSHandler);
        webserver.startServer(port);

        int sleepTimeMilliseconds = 10;
        this_thread::sleep_for(chrono::milliseconds(sleepTimeMilliseconds));
        ASSERT_TRUE(webserver.isServerRunning());

        http::Request request{"http://localhost:"+std::to_string(port)+outOfSyncPath};
        const auto response = request.send("GET");
        std::string bodyString =std::string{response.body.begin(), response.body.end()};
        ASSERT_TRUE(oosWebserverHandlerCalled.load());
        ASSERT_TRUE(bodyString.compare("OOS: false") == 0);

    }
}