#ifndef RABBITMQ_FMU_FMUCONTAINER_H
#define RABBITMQ_FMU_FMUCONTAINER_H

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <cmath>
#include <thread>
#include "fmi2Functions.h"
#include <list>
#include <iterator>
#include "FmuContainerCore.h"
#include <condition_variable>

using namespace std;

#include <string>
#include <map>
#include <ctime>

const int safeToleranceInitial = 500;
const int realTimeCheckIntervalInitial = 200;
/*
 * These are the scalar variable IDs
 */
const int safeToleranceId = 0;
const int realTimeCheckIntervalID = 1;
const int outOfSyncId = 2;
const int webServerHostnameId = 3;
const int webServerPortId = 4;

using namespace std;

enum class FMIState {instantiated, initializing, initialized, error, terminated};

class FmuContainer {
public:
    const fmi2CallbackFunctions *m_functions;
    const string m_name;


    FmuContainer(const fmi2CallbackFunctions *mFunctions,bool loggingOn, const char *mName);

    ~FmuContainer();

    fmi2ComponentEnvironment getComponentEnvironment();

    bool step(fmi2Real currentCommunicationPoint,
              fmi2Real communicationStepSize);

    bool fmi2GetMaxStepsize(fmi2Real *size);

    bool getString(const fmi2ValueReference vr[], size_t nvr, fmi2String value[]);

    bool getReal(const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]);

    bool getBoolean(const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]);

    bool getInteger(const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]);

    bool setString(const fmi2ValueReference vr[], size_t nvr, const fmi2String value[]);

    bool setReal(const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]);

    bool setBoolean(const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]);

    bool setInteger(const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]);

    bool terminate();

    bool setup(fmi2Real startTime);

    bool isLoggingOn();

    /**
     * Starts the core webserver
     * @return
     */
    bool beginInitialize();
    /**
    * Start the core real-time clock.
    * @return
    */
    bool endInitialize();

private:
    FmuContainerCore core;
    FMIState state = FMIState::instantiated;

    const bool loggingOn;


    void destroy();
};


#endif //RABBITMQ_FMU_FMUCONTAINER_H
