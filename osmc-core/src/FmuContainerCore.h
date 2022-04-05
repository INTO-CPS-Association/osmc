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


class FmuContainerCore {

public:
    FmuContainerCore(const fmi2CallbackFunctions *mFunctions, const char *mName, int realTimeCheckIntervalMs,
                     int safeToleranceMs);

    /**
     * The allowed tolerance between the simulation time and the wall-clock time.
     * @param safeToleranceMs
     */
    void setSafeTolerance(int safeToleranceMs);
    int getSafeTolerance();

    /**
     * How often the threshold shall be checked
     * @param realTimeCheckIntervalMs
     */
    void setRealTimeCheckInterval(int realTimeCheckIntervalMs);
    int getRealTimeCheckInterval();

    /**
     * Sets real_time_clock_started to now
     * and starts the timer with realTimeCheckInterval and the checkthreshold function as callback
     * @return
     */
    bool startRealTimeClock();
    typedef unsigned int ScalarVariableId;

    friend std::ostream &operator<<(std::ostream &os, const FmuContainerCore &c);
    void setCurrentSimulationTime(const double currentSimulationTimeMs);

    /**
     * Set the function that is called upon out Of Sync
     * @see checkThreshold
     * @param recoveredCallbackFunction
     */
    void setOutOfSyncCallbackFunction(std::function<void(double tDiff, int safeTolerance)> outOfSyncCallbackFunction);

    /**
     * Set the function that is called upon resynchronisation
     * @see checkThreshold
     * @param inSyncCallbackFunction
     */
    void setInSyncCallbackFunction(std::function<void(void)> inSyncCallbackFunction);

    ~FmuContainerCore();
    /**
     * Sets the handler of the OutOfSync get request IFF the web server has not been started yet.
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
     * Uses a default web handler if one has not been explcitely provided via setWebserverHandler
     * @return true if the web server started, false otherwise
     */
    bool startWebserver();

    /**
     * Stops all activites controlled by the core
     */
    void terminate();

    /**
     * checkThreshold is called on request or automatically every realTimeCheckInterval milliseconds.
     * It calculates realTimeDiff = now - real_time_clock_started.
     * It calculates tDiff = |currentSimulationTime - realTimeDiff|
     * It checks whether tDiff > safeTolerance
     *      if so, it sets out of sync to true and invokes the outOfSyncCallbackFunction.
     *      else, sets out of sync to false and invokes the inSyncCallbackFunction
     * @see checkThresholdCallbackTimer
     */
    void checkThreshold();

    bool getOutOfSync();
    int getTimeDiscrepancy();


    void setWebserverHostname(fmi2String const string);

    /**
     * Function to retrieve the current simulation time in milliseconds
     * @return
     */
    double getCurrentSimulationTimeMs();


    double getDifferenceSimulationTimeMinusRealTimeMs();

private:

    /**
     * Upon starting the function invokes checkThreshold every realTimeCheckInterval milliseconds
     * @see checkThreshold
     */
    CallBackTimer checkThresholdCallbackTimer;


    enum class StateBinary {unset, started,
        stopped};
    std::string printStateBinary(StateBinary stateBinary);
    const bool verbose;
    const fmi2CallbackFunctions* m_functions;
    /**
     * FMU Instance name
     */
    const char *m_name;

    /**
     * Contains a point in time when the clock was started
     */
    std::pair<StateBinary, std::chrono::time_point<std::chrono::system_clock>> real_time_clock_started;

    std::atomic<double> currentSimulationTime = std::atomic<double>(0.0);
    std::function<void(double, int)> outOfSyncCallbackFunction;
    std::function<void()> inSyncCallbackFunction;
    std::atomic<bool> outOfSync = std::atomic<bool>(false);
    int port = 8080;
    Webserver webserver;
    std::function<bool(void)> webserverHandler;
    std::atomic<int> timeDisc = std::atomic<int>(0);


    bool webserverStarted = false;
    /**
     * Variable containing the safe tolerance in milliseconds
     */
    int safeToleranceMs;
    /**
     * Variable containing how often the checkThreshold shall be called by the timer
     */
    int realTimeCheckIntervalMs;
    std::string webserverHostname = "127.0.0.1";
};

#endif //RABBITMQFMUPROJECT_FMUCONTAINERCORE_H
