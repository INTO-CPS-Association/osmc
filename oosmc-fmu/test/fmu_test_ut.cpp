#include "gtest/gtest.h"

#include "fmi2FunctionTypes.h"

#include "FmuContainerCore.h"

#include <stdio.h>  /* defines FILENAME_MAX */

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else

#include <unistd.h>

#define GetCurrentDir getcwd
#endif


using namespace std;

void showStatus(const char *what, fmi2Status status) {
    const char **statuses = new const char *[6]{"ok", "warning", "discard", "error", "fatal", "pending"};
    cout << "Executed '" << what << "' with status '" << statuses[status] << "'" << endl;

    if (status != fmi2OK) {
        throw status;
    }
}

namespace {
    TEST(FmuTest, BasicFlow
    ) {
        bool oos = false;
        cout << " Simulation test for FMI " << fmi2GetVersion() << endl;


        char cCurrentPath[FILENAME_MAX];

        if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
            return;
        }

        cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */


        cout << "Working directory is " << cCurrentPath << endl;

        fmi2String instanceName = "oosmc";
        fmi2Type fmuType = fmi2CoSimulation;
        fmi2String fmuGUID = "63ba49fe-07d3-402c-b9db-2df495167424";
        string currentUri = (string("file://") + string(cCurrentPath));
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

            int safeToleranceMs = 100;
            int realTimeCheckIntervalMs = 50;

            const size_t nvr = 2;
            unsigned int integerValrefs[nvr] = {0,1};
            int integerValues[nvr] = {safeToleranceMs, realTimeCheckIntervalMs};
            showStatus("fmi2SetInteger",fmi2SetInteger(c, integerValrefs, nvr, integerValues));

            showStatus("fmi2EnterInitializationMode", fmi2EnterInitializationMode(c));
            showStatus("fmi2ExitInitializationMode", fmi2ExitInitializationMode(c));

            fmi2Real currentCommunicationPoint = 0;
            fmi2Real communicationStepSize = 0.0001;

            // as sleepTimeMilliseconds > (communicationStepSize+safeToleranceMs) this should be enough to cause an OOS.
            int sleepTimeMilliseconds = 200;
            this_thread::sleep_for(chrono::milliseconds(sleepTimeMilliseconds));

            fmi2Boolean noSetFMUStatePriorToCurrentPoint = false;
            showStatus("fmi2DoStep", fmi2DoStep(c, currentCommunicationPoint, communicationStepSize,
                                                noSetFMUStatePriorToCurrentPoint));


            int nvrBoolean = 1;
            int booleanValues[nvrBoolean];
            integerValrefs[0]=2;
            showStatus("fmi2GetBoolean", fmi2GetBoolean(c, integerValrefs, 1, booleanValues));
            for (int i = 0; i < nvrBoolean; i++) {
                cout << "Ref: '" << integerValrefs[i] << "' Value '" << booleanValues[i] << "'" << endl;
            }
            oos = booleanValues[0];


        fmi2Terminate(c);
        } catch (const char *status) {
            cout << "Error " << status << endl;
        }
        fmi2FreeInstance(c);
        ASSERT_TRUE(oos);

    }

    TEST(FmuTest, GetMaxStepSize
    ) {
        bool oos = false;
        cout << " Simulation test for FMI " << fmi2GetVersion() << endl;


        char cCurrentPath[FILENAME_MAX];

        if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
            return;
        }

        cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */


        cout << "Working directory is " << cCurrentPath << endl;

        fmi2String instanceName = "oosmc";
        fmi2Type fmuType = fmi2CoSimulation;
        fmi2String fmuGUID = "63ba49fe-07d3-402c-b9db-2df495167424";
        string currentUri = (string("file://") + string(cCurrentPath));
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

            int safeToleranceMs = 100;
            int realTimeCheckIntervalMs = 50;

            const size_t nvr = 2;
            unsigned int integerValrefs[nvr] = {0,1};
            int integerValues[nvr] = {safeToleranceMs, realTimeCheckIntervalMs};
            showStatus("fmi2SetInteger",fmi2SetInteger(c, integerValrefs, nvr, integerValues));

            showStatus("fmi2EnterInitializationMode", fmi2EnterInitializationMode(c));
            showStatus("fmi2ExitInitializationMode", fmi2ExitInitializationMode(c));

            fmi2Real currentCommunicationPoint = 0;
            fmi2Real communicationStepSize = 0.00001;

            // as sleepTimeMilliseconds > (communicationStepSize+safeToleranceMs) this should be enough to cause an OOS.
            int sleepTimeMilliseconds = 200;
            this_thread::sleep_for(chrono::milliseconds(sleepTimeMilliseconds));

            fmi2Boolean noSetFMUStatePriorToCurrentPoint = false;
            showStatus("fmi2DoStep", fmi2DoStep(c, currentCommunicationPoint, communicationStepSize,
                                                noSetFMUStatePriorToCurrentPoint));


            int nvrBoolean = 1;
            int booleanValues[nvrBoolean];
            integerValrefs[0]=2;
            showStatus("fmi2GetBoolean", fmi2GetBoolean(c, integerValrefs, 1, booleanValues));
            for (int i = 0; i < nvrBoolean; i++) {
                cout << "Ref: '" << integerValrefs[i] << "' Value '" << booleanValues[i] << "'" << endl;
            }
            oos = booleanValues[0];


            fmi2Real maxStepSize = 50.0;
            showStatus("fmi2GetMaxStepSize", fmi2GetMaxStepsize(c, &maxStepSize));
            cout << "GetMaxStepSize: " << maxStepSize << endl;

            currentCommunicationPoint = currentCommunicationPoint + communicationStepSize;

            showStatus("fmi2DoStep", fmi2DoStep(c, currentCommunicationPoint, maxStepSize,
                                                noSetFMUStatePriorToCurrentPoint));

            showStatus("fmi2GetBoolean", fmi2GetBoolean(c, integerValrefs, 1, booleanValues));
            oos = booleanValues[0];
            for (int i = 0; i < nvrBoolean; i++) {
                cout << "Ref: '" << integerValrefs[i] << "' Value '" << booleanValues[i] << "'" << endl;
            }


            fmi2Terminate(c);
        } catch (const char *status) {
            cout << "Error " << status << endl;
        }
        fmi2FreeInstance(c);
        ASSERT_TRUE(!oos);

    }
}