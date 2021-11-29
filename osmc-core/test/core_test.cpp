//
// Created by Casper Thule on 16/08/2021.
//
#include "gtest/gtest.h"

#include "FmuContainerCore.h"
#include <chrono>
using namespace std;

namespace {
    TEST(CoreTest, OutOfSync
    ) {
        cout << " OutOfSync test" << endl;


        int safeToleranceMs = 100;
        int realTimeCheckIntervalMs = 50;
        double currentSimulationTime = 0.0;

        FmuContainerCore core = FmuContainerCore(nullptr, nullptr, realTimeCheckIntervalMs, safeToleranceMs);

        core.setCurrentSimulationTime(currentSimulationTime);
        atomic<bool> oos = atomic<bool>(false);
        std::function<void(double, int)> oosCallbackFunction = [&oos](double tDiff, int safeTolerance) {
            oos.store(true);
        };
        core.setOutOfSyncCallbackFunction(oosCallbackFunction);
        core.startRealTimeClock();

        int sleepTimeMilliseconds = 200;
        this_thread::sleep_for(chrono::milliseconds(sleepTimeMilliseconds));

        ASSERT_TRUE(oos.load());
    }

    TEST(CoreTest, getMaxStepSize
    ) {
        cout << " OutOfSync test" << endl;


        int safeToleranceMs = 100;
        int realTimeCheckIntervalMs = 50;
        double currentSimulationTime = 0.0;

        FmuContainerCore core = FmuContainerCore(nullptr, nullptr, realTimeCheckIntervalMs, safeToleranceMs);

        core.setCurrentSimulationTime(currentSimulationTime);
        atomic<bool> oos = atomic<bool>(false);
        std::function<void(double, int)> oosCallbackFunction = [&oos](double tDiff, int safeTolerance) {
            oos.store(true);
        };
        core.setOutOfSyncCallbackFunction(oosCallbackFunction);
        core.startRealTimeClock();

        int sleepTimeMilliseconds = 200;
        this_thread::sleep_for(chrono::milliseconds(sleepTimeMilliseconds));

        double simulationAheadMs = core.getDifferenceSimulationTimeMinusRealTimeMs();
//        std::cout << simulationAheadMs << std::endl;
        // This should be negative, as the simulation is behind the real time
        ASSERT_TRUE(simulationAheadMs < 0.0);

        //Convert simulationAheadMs to simulationBehind, as it is negative. And turn it into a positive value to catch up.
        double simulationBehindMs = std::abs(simulationAheadMs);
        core.setCurrentSimulationTime(simulationBehindMs);
        simulationAheadMs = core.getDifferenceSimulationTimeMinusRealTimeMs();
//        std::cout << simulationAheadMs << std::endl;
        core.checkThreshold();

        //Out of sync should be false
        ASSERT_TRUE(!core.getOutOfSync());
    }
}