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

        FmuContainerCore core = FmuContainerCore(nullptr, nullptr);
        // Safe tolerance
        int safeToleranceMs = 100;
        core.setSafeTolerance(safeToleranceMs);
        double currentSimulationTime = 0.0;
        core.setCurrentSimulationTime(currentSimulationTime);
        int realTimeCheckIntervalMs = 50;
        core.setRealTimeCheckInterval(realTimeCheckIntervalMs);
        atomic<bool> oos = atomic<bool>(false);
        std::function<void(double, int)> cbFunction = [&oos](double tDiff, int safeTolerance) {
            oos.store(true);
        };
        core.setOutOfSyncCallbackFunction(cbFunction);
        core.startRealTimeClock();
        int sleepTimeMilliseconds = 200;
        this_thread::sleep_for(chrono::milliseconds(sleepTimeMilliseconds));

        ASSERT_TRUE(oos.load());
    }
}