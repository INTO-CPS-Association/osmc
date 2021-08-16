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

#include "../../thirdparty/fmi/include/fmi2Functions.h"

const unsigned int safeToleranceId = 0;


typedef std::variant<int, bool, double, std::string> ScalarVariableBaseValue;

std::ostream &operator<<(std::ostream &os, const ScalarVariableBaseValue &c);
class FmuContainerCore {


public:
    FmuContainerCore(const fmi2CallbackFunctions *mFunctions, const char *mName);
    typedef unsigned int ScalarVariableId;
    std::map<ScalarVariableId, ScalarVariableBaseValue> getData();
    friend std::ostream &operator<<(std::ostream &os, const FmuContainerCore &c);


private:
    std::map<ScalarVariableId, ScalarVariableBaseValue> data;
    const bool verbose;
    const fmi2CallbackFunctions m_functions;
    const char *m_name;
};

#endif //RABBITMQFMUPROJECT_FMUCONTAINERCORE_H
