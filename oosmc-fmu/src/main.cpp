#include <iostream>

#include <fstream>
#include <sstream>

#include <cstdlib>
#include <ctime>
#include <string.h>

#include <iomanip>
#ifdef _WIN32
#define timegm _mkgmtime
#endif

/* #include <filesystem> */
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>


#include "fmi2Functions.h"


#include <stdio.h>  /* defines FILENAME_MAX */

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

/* using namespace std; */

inline int ParseInt(const char *value) {
    return std::strtol(value, nullptr, 10);
}

std::time_t ParseISO8601(const std::string &input) {
    constexpr const size_t expectedLength = sizeof("1234-12-12T12:12:12Z") - 1;
    static_assert(expectedLength == 20, "Unexpected ISO 8601 date/time length");

    if (input.length() < expectedLength) {
        return 0;
    }

    std::tm time = {0};
    time.tm_year = ParseInt(&input[0]) - 1900;
    time.tm_mon = ParseInt(&input[5]) - 1;
    time.tm_mday = ParseInt(&input[8]);
    time.tm_hour = ParseInt(&input[11]);
    time.tm_min = ParseInt(&input[14]);
    time.tm_sec = ParseInt(&input[17]);
    time.tm_isdst = 0;
    const int millis = input.length() > 20 ? ParseInt(&input[20]) : 0;
    return timegm(&time) * 1000 + millis;
}

void showStatus(const char *what, fmi2Status status) {
    const char **statuses = new const char *[6]{"ok", "warning", "discard", "error", "fatal", "pending"};
    std::cout << "Executed '" << what << "' with status '" << statuses[status] << "'" << std::endl;

    if (status != fmi2OK) {
        throw status;
    }
}

int main(int argc, char *argv[]) {
    {
        std::cout << " Simulation test for FMI " << fmi2GetVersion() << std::endl;

        if (argc > 2) {
            std::cerr << "Usage: " << argv[0] << " [MODEL_DESCRIPTION_PATH]" << std::endl;
            return 1;
        }

        char path[FILENAME_MAX];

        if (argc == 1) {
            if (!GetCurrentDir(path, sizeof(path))) {
                return 1;
            }
        } else {
            strncpy(path, argv[1], sizeof(path) - 1);
        }


        path[sizeof(path) - 1] = '\0'; /* not really required */


        std::cout << "Working directory is " << path << std::endl;

        /* std::filesystem::create_directory("log"); */
        int fileIndex = 0;
        std::string fileNameBase = "log/log";
        std::string fileNameExt = ".csv";
        std::string fileName = fileNameBase + std::to_string(fileIndex) + fileNameExt;
        struct stat statBuffer;

        while (stat(fileName.c_str(), &statBuffer) != -1)
        {
            fileIndex++;
            fileName = fileNameBase + std::to_string(fileIndex) + fileNameExt;
        }

        std::ofstream file(fileName.c_str());
        if (!file.is_open())
        {
            std::cout << "Failed to open log file" << std::endl;
            return 1;
        }

        fmi2String instanceName = "rabbitmq";
        fmi2Type fmuType = fmi2CoSimulation;
        fmi2String fmuGUID = "63ba49fe-07d3-402c-b9db-2df495167424";
        std::string currentUri = (std::string("file://") + std::string(path));
        fmi2String fmuResourceLocation = currentUri.c_str();
        const fmi2CallbackFunctions *functions = nullptr;
        fmi2Boolean visible = false;
        fmi2Boolean loggingOn = false;


        auto c = fmi2Instantiate(
                instanceName,
                fmuType, fmuGUID,
                fmuResourceLocation,
                functions,
                visible,
                loggingOn);

        try {
            fmi2Boolean toleranceDefined = false;
            fmi2Real tolerance = 0;
            fmi2Real startTime = 0;
            fmi2Boolean stopTimeDefined = true;
            fmi2Real stopTime = true;

            showStatus("fmi2SetupExperiment", fmi2SetupExperiment(
                    c, toleranceDefined, tolerance,
                    startTime, stopTimeDefined, stopTime));

#define RABBITMQ_FMU_HOSTNAME_ID 0
#define RABBITMQ_FMU_PORT 1
#define RABBITMQ_FMU_USER 2
#define RABBITMQ_FMU_PWD 3
#define RABBITMQ_FMU_ROUTING_KEY 4
#define RABBITMQ_FMU_COMMUNICATION_READ_TIMEOUT 5
#define RABBITMQ_FMU_PRECISION 6

#define RABBITMQ_FMU_SEND_FLAG_CSTOP 21
#define RABBITMQ_FMU_COMMAND_STOP 23
#define RABBITMQ_FMU_COMMAND_INT 24

            fmi2ValueReference vrefs[] = {RABBITMQ_FMU_COMMUNICATION_READ_TIMEOUT, RABBITMQ_FMU_PRECISION,
                                          RABBITMQ_FMU_PORT};
            int intVals[] = {60, 10, 5672};
            fmi2SetInteger(c, vrefs, 3, intVals);


            fmi2ValueReference vrefsString[] = {RABBITMQ_FMU_HOSTNAME_ID, RABBITMQ_FMU_USER, RABBITMQ_FMU_PWD,
                                                RABBITMQ_FMU_ROUTING_KEY};
            const char *stringVals[] = {"localhost", "guest", "guest", "linefollower"};
            fmi2SetString(c, vrefsString, 4, stringVals);

            //fmi2SetBoolean(c, vrefsBoolean, sizeof(boolVals)/sizeof(*boolVals), boolVals);

            showStatus("fmi2EnterInitializationMode", fmi2EnterInitializationMode(c));
            showStatus("fmi2ExitInitializationMode", fmi2ExitInitializationMode(c));

            std::cout << "Initialization one"<<std::endl;

#define RABBITMQ_FMU_LEVEL 100

            size_t nvr = 3;
            const fmi2ValueReference *vr =
                new fmi2ValueReference[nvr]{RABBITMQ_FMU_LEVEL,RABBITMQ_FMU_LEVEL+1,RABBITMQ_FMU_LEVEL+2};
            fmi2Real *value = new fmi2Real[nvr];

            showStatus("fmi2GetReal", fmi2GetReal(c, vr, nvr, value));
            for (int i = 0; i < nvr; i++) {
                std::cout << "Ref: '" << vr[i] << "' Value '" << value[i] << "'" << std::endl;
            }


            fmi2Real currentCommunicationPoint = 0;
            fmi2Real communicationStepSize = 0.1;
            fmi2Boolean noSetFMUStatePriorToCurrentPoint = false;

            fmi2Real simDuration = 10;
            bool changeInput = false;
            fmi2ValueReference vrefsReals[] = {RABBITMQ_FMU_COMMAND_STOP};
            fmi2ValueReference vrefsInt[] = {RABBITMQ_FMU_COMMAND_INT};
            fmi2ValueReference vrefsBool[] = {26};
            fmi2ValueReference vrefsStrs[] = {25};
            fmi2Real reals[] = {3.5};
            fmi2Integer ints[] = {5};
            fmi2Boolean bools[] = {true};
            fmi2String strs[] = {"hejsan"};

            file << "simtime,stepdur,ref1,ref2,ref3\n";

            for(int i = 0; i <= simDuration; i++){
                auto t1 = std::chrono::high_resolution_clock::now();
                showStatus("fmi2DoStep", fmi2DoStep(c, currentCommunicationPoint, communicationStepSize,
                                                    noSetFMUStatePriorToCurrentPoint));
                auto t2 = std::chrono::high_resolution_clock::now();
                auto dur = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

                file << currentCommunicationPoint << ", " << dur << ", ";
                showStatus("fmi2GetReal", fmi2GetReal(c, vr, nvr, value));
                for (int i = 0; i < nvr; i++) {
                    file << value[i];
                    if (i < nvr - 1)
                        file << ", ";
                    else
                        file << "\n";
                    std::cout << "Ref: '" << vr[i] << "' Value '" << std::setprecision(10) << value[i] << "'" << std::endl;
                }

                currentCommunicationPoint = currentCommunicationPoint + communicationStepSize;

                if(changeInput){
                    showStatus("fmi2SetReal", fmi2SetReal(c, vrefsReals, 1, reals));
                    showStatus("fmi2SetString", fmi2SetString(c, vrefsStrs, 1, strs));
                    std::cout << "SHOULD have updated" << std::endl;
                }
                else{

                    showStatus("fmi2SetInteger", fmi2SetInteger(c, vrefsInt, 1, ints));
                    showStatus("fmi2SetBoolean", fmi2SetBoolean(c, vrefsBool, 1, bools));
                }

                changeInput=!changeInput;

            }

//        fmi2Terminate(fmi2Component c)
        } catch (const char *status) {
            std::cout << "Error " << status << std::endl;
        }
        fmi2FreeInstance(c);

        file.close();
    }

    return 0;
}
