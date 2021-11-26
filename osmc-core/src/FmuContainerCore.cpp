//
// Created by Kenneth Guldbrandt Lausdahl on 09/03/2020.
//

#include "FmuContainerCore.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

#define FmuContainerCore_LOG(status, category, message, args...)       \
  if (m_functions != NULL) {           \
    if (m_functions->logger != NULL) {                                   \
      m_functions->logger(m_functions->componentEnvironment, m_name, status, \
                        category, message, args);                      \
    }                                                                  \
  } else {                                                             \
    fprintf(stderr, "Name '%s', Category: '%s'\n", m_name, category);    \
    fprintf(stderr, message, args);                                    \
    fprintf(stderr, "\n");                                             \
  }

FmuContainerCore::FmuContainerCore(const fmi2CallbackFunctions *mFunctions, const char *mName, int realTimeCheckIntervalMs, int safeToleranceMs)
        : verbose(true), m_functions(mFunctions), m_name(mName){
    this->setRealTimeCheckInterval(realTimeCheckIntervalMs);
    this->setSafeTolerance(safeToleranceMs);
}

void FmuContainerCore::setPort(int port){
    if(!this->webserverStarted)
        this->port = port;
}

bool FmuContainerCore::startWebserver(){
    if(!this->webserverStarted)
    {
        // If no webserver handler has been explictely set, then create and set the default.
        if(!this->webserverHandler)
        {
            this->webserverHandler = [this](){return this->outOfSync.load();};
        }
        this->webserver.RegisterOOSHandler(this->webserverHandler);
        this->webserver.startServer(this->webserverHostname, this->port);
        this->webserverStarted = true;
        return true;
    }
    else {
        return false;
    }

}
bool FmuContainerCore::startRealTimeClock() {
    if(this->real_time_clock_started.first == StateBinary::unset) {
            this->real_time_clock_started.second = std::chrono::system_clock::now();
            this->real_time_clock_started.first = StateBinary::started;
            this->checkThresholdCallbackTimer.start(this->getRealTimeCheckInterval(), std::bind(&FmuContainerCore::checkThreshold, this));
            return true;
    }
    else {
        FmuContainerCore_LOG(fmi2Error, "logAll", "Invalid real time clock state: %s", this->printStateBinary(this->real_time_clock_started.first).c_str());
        return false;
    }
}

std::string FmuContainerCore::printStateBinary(FmuContainerCore::StateBinary stateBinary) {
    switch (stateBinary) {
        case StateBinary::unset:
            return "unset";
        case StateBinary::started:
            return "started";
        case StateBinary::stopped:
            return "stopped";
    }
}

double FmuContainerCore::getDifferenceSimulationTimeMinusRealTimeMs(){
    // Get the current time
    auto realTimeDifferenceMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - this->real_time_clock_started.second);
    // This the current simulation Time
    double currentSimulationTimeMs = this->getCurrentSimulationTimeMs();
    double tdiff = currentSimulationTimeMs - realTimeDifferenceMs.count();

    return tdiff;
}

void FmuContainerCore::checkThreshold() {
    double tdiff = std::abs(getDifferenceSimulationTimeMinusRealTimeMs());
    if(tdiff > this->getSafeTolerance())
    {
        // DT AND PT ARE OUT OF SYNC!
        if(this->outOfSyncCallbackFunction) {
            this->outOfSyncCallbackFunction(tdiff, this->getSafeTolerance());
        }
        this->outOfSync.store(true);
    }
    else {
        this->outOfSync.store(false);
        if(this->inSyncCallbackFunction)
            this->inSyncCallbackFunction();
    }

}

double FmuContainerCore::getCurrentSimulationTimeMs() {
    return this->currentSimulationTime.load();
}

void FmuContainerCore::setCurrentSimulationTime(const double currentSimulationTimeMs) {
    this->currentSimulationTime.store(currentSimulationTimeMs);
}

int FmuContainerCore::getSafeTolerance() {
    return this->safeToleranceMs;
}

void FmuContainerCore::setOutOfSyncCallbackFunction(std::function<void(double, int)> outOfSyncCallbackFunction) {
    this->outOfSyncCallbackFunction = outOfSyncCallbackFunction;
}

void FmuContainerCore::terminate() {
    if (this->checkThresholdCallbackTimer.is_running()){
        this->checkThresholdCallbackTimer.stop();
    }
    this->real_time_clock_started.first = StateBinary::stopped;
    if (this->webserverStarted){
     this->webserver.stop();
    }
}

FmuContainerCore::~FmuContainerCore() {
    this->terminate();

}

void FmuContainerCore::setSafeTolerance(int safeToleranceMs) {
    this->safeToleranceMs = safeToleranceMs;
}

void FmuContainerCore::setRealTimeCheckInterval(int realTimeCheckIntervalMs) {
    if(this->real_time_clock_started.first == StateBinary::unset)
        this->realTimeCheckIntervalMs = realTimeCheckIntervalMs;
}

bool FmuContainerCore::setWebserverHandler(std::function<bool(void)> webserverHandler) {
    if(!this->webserverStarted) {
        this->webserverHandler = webserverHandler;
        return true;
    }
    else {
        FmuContainerCore_LOG(fmi2Error, "logAll", "The webserver has already been started. Cannot set a new handler.","");
        return false;
    }
}

void FmuContainerCore::setInSyncCallbackFunction(std::function<void(void)> inSyncCallbackFunction) {
    this->inSyncCallbackFunction = inSyncCallbackFunction;

}

int FmuContainerCore::getRealTimeCheckInterval() {
    return this->realTimeCheckIntervalMs;
}

bool FmuContainerCore::getOutOfSync() {
    return this->outOfSync.load();
}

void FmuContainerCore::setWebserverHostname(fmi2String webServerHostname) {
    this->webserverHostname = webServerHostname;

}

std::ostream &operator<<(std::ostream &os, FmuContainerCore &c) {
    os << "------------------------------ INFO ------------------------------" << "\n";
    os << "Data" << "\n";
    os << "Safe tolerance: " << c.getSafeTolerance() << std::endl;
    os << "Real Time Check Interval: " << c.getRealTimeCheckInterval() << std::endl;
    os << "Out of sync: " << c.getOutOfSync() << std::endl;
    os << "\n";
    os << "------------------------------------------------------------------" << "\n";
    return os;
}

