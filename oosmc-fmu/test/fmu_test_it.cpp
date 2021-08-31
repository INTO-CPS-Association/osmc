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

            /*size_t nvr = 2;
            const fmi2ValueReference *valRefs2 = new fmi2ValueReference[nvr]{0,1};
            fmi2Integer *integerValues2 = new fmi2Integer[nvr]{safeToleranceMs, realTimeCheckIntervalMs};
             fmi2SetInteger(c, valRefs2, 2, integerValues2);
*/
            //typedef fmi2Status fmi2SetIntegerTYPE(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Integer[]);
            size_t nvr = 2;
            unsigned int integerValrefs[nvr] = {0,1};
            int integerValues[nvr] = {safeToleranceMs, realTimeCheckIntervalMs};
            fmi2SetInteger(c, integerValrefs, nvr, integerValues);

            showStatus("fmi2EnterInitializationMode", fmi2EnterInitializationMode(c));
            showStatus("fmi2ExitInitializationMode", fmi2ExitInitializationMode(c));

//            fmi2Real currentCommunicationPoint = 0;
//            fmi2Real communicationStepSize = 10;
//            fmi2Boolean noSetFMUStatePriorToCurrentPoint = false;
//
//
//            showStatus("fmi2DoStep", fmi2DoStep(c, currentCommunicationPoint, communicationStepSize,
//                                                noSetFMUStatePriorToCurrentPoint));
//
//            showStatus("fmi2GetReal", fmi2GetReal(c, vr, nvr, value));
//            for (int i = 0; i < nvr; i++) {
//                cout << "Ref: '" << vr[i] << "' Value '" << value[i] << "'" << endl;
//            }


//        fmi2Terminate(fmi2Component c)
        } catch (const char *status) {
            cout << "Error " << status << endl;
        }
        fmi2FreeInstance(c);

    }
}