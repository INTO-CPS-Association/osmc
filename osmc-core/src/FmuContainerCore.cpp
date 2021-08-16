//
// Created by Kenneth Guldbrandt Lausdahl on 09/03/2020.
//

#include "FmuContainerCore.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

#define FmuContainerCore_LOG(status, category, message, args...)       \
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

FmuContainerCore::FmuContainerCore(const fmi2CallbackFunctions *mFunctions, const char *mName)
        : verbose(true), m_functions(m_functions), m_name(mName){

}



std::map<FmuContainerCore::ScalarVariableId, ScalarVariableBaseValue> FmuContainerCore::getData() {
    return this->data;
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



