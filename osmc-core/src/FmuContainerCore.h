//
// Created by Kenneth Guldbrandt Lausdahl on 09/03/2020.
//

#ifndef RABBITMQFMUPROJECT_FMUCONTAINERCORE_H
#define RABBITMQFMUPROJECT_FMUCONTAINERCORE_H


#include <list>
#include <iterator>

#include <string>
#include <map>
#include <ctime>
#include <iostream>
#include <mutex>
#include <variant>
#include "CallBackTimer.h"
#include <atomic>
#include "Webserver.h"

#include "../../thirdparty/fmi/include/fmi2Functions.h"

/*
 * These are the scalar variable IDs
 */
const unsigned int safeToleranceId = 0;
const unsigned int realTimeCheckInternalID = 1;



typedef std::variant<int, bool, double, std::string> ScalarVariableBaseValue;

std::ostream &operator<<(std::ostream &os, const ScalarVariableBaseValue &c);
class FmuContainerCore {

public:
    FmuContainerCore(const fmi2CallbackFunctions *mFunctions, const char *mName);
    void setSafeTolerance(int safeToleranceMs);
    bool startRealTimeClock();
    typedef unsigned int ScalarVariableId;
    std::map<ScalarVariableId, ScalarVariableBaseValue> getData();
    friend std::ostream &operator<<(std::ostream &os, const FmuContainerCore &c);
    void setCurrentSimulationTime(const double currentSimulationTimeMs);
    unsigned int getSafeTolerance();
    void setOutOfSyncCallbackFunction(std::function<void(double tDiff, int safeTolerance)> outOfSyncCallbackFunction);
    void setRecoveredCallbackFunction(std::function<void(void)> recoveredCallbackFunction);
    void setRealTimeCheckInterval(int realTimeCheckIntervalMs);
    ~FmuContainerCore();
    /**
     * Sets the handler of the OOS get request IFF the web server has not been started yet.
     * @param webserverHandler
     * @return
     */
    bool setWebserverHandler(std::function<bool(void)> webserverHandler);
    /**
     * Sets the port of the web server IFF the web server has not been started yet
     * @param port
     */
    void setPort(int port);
    /**
     * Starts the web server.
     * Uses a default web handler if one has not been explcitely provided.
     * @return true if the web server started, false otherwise
     */
    bool startWebserver();


private:
    void checkThreshold();
    CallBackTimer callBackTimer;
    enum class StateBinary {unset, started,
        stopped};
    std::string printStateBinary(StateBinary stateBinary);
    std::map<ScalarVariableId, ScalarVariableBaseValue> data;
    const bool verbose;
    const fmi2CallbackFunctions* m_functions;
    const char *m_name;
    std::chrono::time_point<std::chrono::system_clock> started;
    std::pair<StateBinary, std::chrono::time_point<std::chrono::system_clock>> real_time_clock;

    double getCurrentSimulationTimeMs();
    std::atomic<double> currentSimulationTime = std::atomic<double>(0.0);
    std::function<void(double, int)> outOfSyncCallbackFunction;
    std::function<void()> recoveredCallbackFunction;
    std::atomic<bool> outOfSync = std::atomic<bool>(false);
    int port = 8080;
    Webserver webserver;
    std::function<bool(void)> webserverHandler;


    bool webserverStarted = false;

};

#endif //RABBITMQFMUPROJECT_FMUCONTAINERCORE_H
