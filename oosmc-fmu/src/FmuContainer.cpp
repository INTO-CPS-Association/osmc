//
// Created by Kenneth Guldbrandt Lausdahl on 11/12/2019.
//

#include "FmuContainer.h"

#include <utility>
#include <thread>
/* LOGGING EXAMPLE
 *
 * FmuContainer_LOG(fmi2OK, "logAll",
 *      "Initialization completed with: Hostname='%s', Port='%d', Username='%s', routingkey='%s', starttimestamp='%s', communication timeout %d s",
 *      hostname.c_str(), port, username.c_str(), routingKey.c_str(), startTimeStamp.str().c_str(),
 *      this->communicationTimeout);
 */
#define FmuContainer_LOG(status, category, message, args...)       \
  if (m_functions != NULL) {                                             \
    if (m_functions->logger != NULL) {                                   \
      m_functions->logger(m_functions->componentEnvironment, m_name.c_str(), status, \
                        category, message, args);                      \
    }                                                                  \
  } else {                                                             \
    fprintf(stderr, "Name '%s', Category: '%s'\n", m_name.c_str(), category);    \
    fprintf(stderr, message, args);                                    \
    fprintf(stderr, "\n");                                             \
  }


FmuContainer::FmuContainer(const fmi2CallbackFunctions *mFunctions, bool logginOn, const char *mName)
        : m_functions(mFunctions), m_name(mName), loggingOn(logginOn), core(mFunctions, mName, realTimeCheckIntervalInitial, safeToleranceInitial){
}

FmuContainer::~FmuContainer() {
    this->terminate();

}

bool FmuContainer::isLoggingOn() {
    return this->loggingOn;
}

/*####################################################
 *  Other
 ###################################################*/

bool FmuContainer::setup(fmi2Real startTime) {
    if(this->state != FMIState::instantiated){
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "Setup called in the wrong state.",
                         "");
        this->state = FMIState::error;
        return false;
    }
    if(startTime != 0.0) {
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "Expected startTime to be 0.0. Actual start time was: %f",
                         startTime);
        return false;
    }
    else {
        return true;
    }
}

bool FmuContainer::terminate() {
    this->state = FMIState::terminated;
    this->core.terminate();
    return true;
}


fmi2ComponentEnvironment FmuContainer::getComponentEnvironment() { return (fmi2ComponentEnvironment) this; }

bool FmuContainer::step(fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize) {
    if(this->state != FMIState::initialized){
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "Step called in the wrong state.",
                         "");
        this->state = FMIState::error;
        return false;
    }
    // Multiple by 1000 due to seconds -> milliseconds
    this->core.setCurrentSimulationTime((currentCommunicationPoint + communicationStepSize)*1000);
    this->core.checkThreshold();
    return true;
}

/*####################################################
 *  Custom
 ###################################################*/

bool FmuContainer::fmi2GetMaxStepsize(fmi2Real *size) {
    double currentSimulationTimeMS = this->core.getCurrentSimulationTimeMs();
    // What is the current real time?
    double simulationTimeMinusRealTimeMs = this->core.getDifferenceSimulationTimeMinusRealTimeMs();
    if (simulationTimeMinusRealTimeMs < 0.0){
        // The simulation time is behind the real time.
        // Therefore make it absolute value and divide by 1000 to go from milliseconds to seconds.
        *size = std::abs(simulationTimeMinusRealTimeMs)/1000;
    }
    else {
        // simulation time is ahead. Expect orchestration algortihm to correct 0.0 step size
        *size = 0.0;
    }

    FmuContainer_LOG(fmi2OK, "logStatusOk", "fmi2GetMaxStepsize returning size: %f",
                     *size);
    return true;
}

/*####################################################
 *  GET
 ###################################################*/

bool FmuContainer::getBoolean(const fmi2ValueReference *vr, size_t nvr, fmi2Boolean *value) {

    // The only valid boolean is 2
    if(nvr == 1 && vr[0] == outOfSyncId){
        value[0] = this->core.getOutOfSync();
        return true;
    }
    else {
        if(nvr != 1){
            FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "getBoolean received invalid arguments. nvr expected: 1, actual: %zu. ",
                                      nvr);
        }
        else {
            if (vr[0] != outOfSyncId){
                FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "getBoolean received invalid arguments. Value references allowed: %i, actual: %i. ",
                                 outOfSyncId, value[0]);
            }
        }
        return false;
    }
}

bool FmuContainer::getInteger(const fmi2ValueReference *vr, size_t nvr, fmi2Integer *value) {
    if (nvr > 3 || nvr == 0)
    {
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "getInteger received invalid arguments. nvr expected: 1 to 2, actual: %zu. ",
                         nvr);
        return false;
    }
    else {
        for(int i = 0; i < nvr; i++){

            if(vr[i] == safeToleranceId)
                value[i] = this->core.getSafeTolerance();
            else if (vr[i] == realTimeCheckIntervalID)
                value[i] = this->core.getRealTimeCheckInterval();
            else if (vr[i] == timeDiscId)
                value[i] = this->core.getTimeDiscrepancy();
            else {
                FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "setInteger received invalid arguments. Value references allowed: 0 to 2, actual: %i. ",
                                 vr[i]);
                return false;
            }
        }
        return true;
    }
}

bool FmuContainer::getReal(const fmi2ValueReference *vr, size_t nvr, fmi2Real *value) {
    if (nvr > 0){
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "getReal is invalid. There are no reals.","");
        return false;
    }
    else {
        return true;
    }
}

bool FmuContainer::getString(const fmi2ValueReference *vr, size_t nvr, fmi2String *value) {
    if (nvr > 0){
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "getString is invalid. There are no strings.","");
        return false;
    }
    else {
        return true;
    }
}


/*####################################################
 *  SET
 ###################################################*/

bool FmuContainer::setBoolean(const fmi2ValueReference *vr, size_t nvr, const fmi2Boolean *value) {
    if (nvr > 0){
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "setBoolean is invalid. There are booleans to set.","");
        return false;
    }
    else {
        return true;
    }
}

bool FmuContainer::setInteger(const fmi2ValueReference *vr, size_t nvr, const fmi2Integer *value) {
    // Only allow sets in the instantiated mode as they are all parameters.
    if(this->state == FMIState::instantiated)
    {
        if (nvr > 3 || nvr == 0)
        {
            FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "setInteger received invalid arguments. nvr expected:1 to 3, actual: %zu. ",
                             nvr);
            return false;
        }
        else {
            for(int i = 0; i < nvr; i++){
                if(vr[i] == safeToleranceId)
                    this->core.setSafeTolerance(value[i]);
                else if (vr[i] == realTimeCheckIntervalID)
                    this->core.setRealTimeCheckInterval(value[i]);
                else if (vr[i] == webServerPortId)
                    this->core.setPort(value[i]);
                else {
                    FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "setInteger received invalid arguments. Value references allowed: 0 to 1, actual: %i. ",
                                     vr[i]);
                    return false;
                }
            }
        }
    }
    else{
        FmuContainer_LOG(fmi2OK, "logError", "setInteger is invalid in the current state.","");
        return false;
    }
    return true;
}

bool FmuContainer::setReal(const fmi2ValueReference *vr, size_t nvr, const fmi2Real *value) {
    if (nvr > 0){
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "setReal is invalid. There are reals to set.","");
        return false;
    }
    else {
        return true;
    }
}

bool FmuContainer::setString(const fmi2ValueReference *vr, size_t nvr, const fmi2String *value) {
    // Only allow sets in the instantiated mode as they are all parameters.
    if(this->state == FMIState::instantiated) {
        if (nvr > 1 || nvr == 0) {
            FmuContainer_LOG(fmi2Fatal, "logStatusFatal",
                             "setString received invalid arguments. nvr expected: 1, actual: %zu. ",
                             nvr);
            return false;
        } else if (nvr == 1) {

            if (vr[0] == webServerHostnameId) {
                this->core.setWebserverHostname(value[0]);
            } else {
                FmuContainer_LOG(fmi2Fatal, "logStatusFatal",
                                 "setString received invalid arguments. Value references allowed: %i, actual: %i. ",
                                 webServerHostnameId, vr[0]);
                return false;
            }
        }
        return true;
    }
    else {
        FmuContainer_LOG(fmi2OK, "logError", "setString is invalid in the current state.","");
        return false;
    }
}

bool FmuContainer::beginInitialize() {
    if(this->state != FMIState::instantiated){
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "beginInitializationMode called in the wrong state.",
                         "");
        this->state = FMIState::error;
        return false;
    }
    this->core.startWebserver();
    this->state = FMIState::initializing;
    return true;
}

bool FmuContainer::endInitialize() {
    if(this->state != FMIState::initializing){
        FmuContainer_LOG(fmi2Fatal, "logStatusFatal", "exitInitializationMode called in the wrong state.",
                         "");
        this->state = FMIState::error;
        return false;
    }
    this->state = FMIState::initialized;
    this->core.startRealTimeClock();
    return true;
}



