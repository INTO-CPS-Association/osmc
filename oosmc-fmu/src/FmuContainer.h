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

const int safeInitial = 10000;

#define mergeFieldMap(mapName, other) \
    for(auto& pair: other.mapName) \
    { \
        this->mapName[pair.first]=pair.second; \
    }

using namespace std;

struct DataPoint {
    std::map<unsigned int, int> integerValues;
    std::map<unsigned int, double> doubleValues;
    std::map<unsigned int, bool> booleanValues;
    std::map<unsigned int, std::string> stringValues;


    DataPoint merge(DataPoint other) {
        mergeFieldMap(integerValues, other)
        mergeFieldMap(doubleValues, other)
        mergeFieldMap(booleanValues, other)
        mergeFieldMap(stringValues, other)
        return *this;
    }
};

enum FMIState {instantiated, initializing, initialized};

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

    bool beginInitialize();

    bool endInitialize();

private:
    DataPoint currentData;
    FmuContainerCore *core;
    FMIState state;

    const bool loggingOn;

    unsigned long precision;


};


#endif //RABBITMQ_FMU_FMUCONTAINER_H