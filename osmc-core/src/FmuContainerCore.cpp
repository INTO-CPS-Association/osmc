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
    this->data[outOfSyncId] = ScalarVariableBaseValue(this->outOfSync.load());
}

std::map<FmuContainerCore::ScalarVariableId, ScalarVariableBaseValue> FmuContainerCore::getData() {
    return this->data;
}

void FmuContainerCore::setPort(int port){
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
        this->webserver.startServer(this->port);
        this->webserverStarted = true;
        return true;
    }
    else {
        return false;
    }

}
bool FmuContainerCore::startRealTimeClock() {
    if(this->real_time_clock_started.first == StateBinary::unset) {
        auto realTimeCheckInternalIDValue = this->data.find(realTimeCheckIntervalID);
        // Check that the real time check internal ID value has been set
        if (realTimeCheckInternalIDValue != data.end()){
            this->real_time_clock_started.second = std::chrono::system_clock::now();
            this->real_time_clock_started.first = StateBinary::started;
            this->checkThresholdCallbackTimer.start(std::get<fmi2Integer>(realTimeCheckInternalIDValue->second), std::bind(&FmuContainerCore::checkThreshold, this));
            return true;
        }
        else {
            FmuContainerCore_LOG(fmi2Error, "logAll", "Real time check internal ID value has not been set","");
            return false;
        }

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

void FmuContainerCore::checkThreshold() {
    // Get the current time
    auto realTimeDifferenceMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - this->real_time_clock_started.second);
    // This the current simulation Time
    double currentSimulationTimeMs = this->getCurrentSimulationTimeMs();
    double tdiff = std::abs(currentSimulationTimeMs - realTimeDifferenceMs.count());
    if(tdiff > this->getSafeTolerance())
    {
        // DT AND PT ARE OUT OF SYNC!
        if(this->outOfSyncCallbackFunction)
        this->outOfSyncCallbackFunction(tdiff, this->getSafeTolerance());
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
    return std::get<fmi2Integer>(this->data[safeToleranceId]);
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
    this->data[safeToleranceId] = ScalarVariableBaseValue(safeToleranceMs);

}

void FmuContainerCore::setRealTimeCheckInterval(int realTimeCheckIntervalMs) {
    this->data[realTimeCheckIntervalID] = ScalarVariableBaseValue(realTimeCheckIntervalMs);

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

void FmuContainerCore::setRecoveredCallbackFunction(std::function<void(void)> recoveredCallbackFunction) {
    this->inSyncCallbackFunction = recoveredCallbackFunction;

}

int FmuContainerCore::getRealTimeCheckInterval() {
    return std::get<fmi2Integer>(this->data[realTimeCheckIntervalID]);
}

bool FmuContainerCore::getOutOfSync() {
    return this->outOfSync.load();
}

std::ostream &operator<<(std::ostream &os, FmuContainerCore &c) {
    os << "------------------------------ INFO ------------------------------" << "\n";
    os << "Data" << "\n";
    for (auto &pair: c.getData()) {
        os << "\tId: " << pair.first;
        os << "value: " << pair.second;
    }
    os << "\n";
    os << "------------------------------------------------------------------" << "\n";
    return os;
}
std::ostream &operator<<(std::ostream &os, const ScalarVariableBaseValue &c) {
    if(std::holds_alternative<fmi2Integer>(c))
    {
        os<<"I" << std::get<fmi2Integer>(c);
    }
    else if(std::holds_alternative<fmi2Real>(c))
    {
        os<<"D" << std::get<fmi2Real>(c);
    }
    else if(std::holds_alternative<fmi2Boolean>(c))
    {
        os<<"B" << std::get<fmi2Boolean>(c);
    }
    else if(std::holds_alternative<std::string>(c))
    {
        os<<"S" << std::get<std::string>(c);
    }
    else{
        os << "UNKNOWN VARIANT TYPE OF ScalarVariableBaseValue";
    }
    return os;
}



