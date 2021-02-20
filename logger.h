//
// Created by andrey on 11/24/20.
//

#ifndef CORE_LOGGER_H
#define CORE_LOGGER_H

#include <dbl/DblManager.h>
#include <boost/filesystem.hpp>
#include "Config.h"
#include "FanucIo.h"
#include <vector>
#include <map>
#include <string>
#include <mutex>

struct ModRecord
{
    uint16_t value;
    uint64_t timestamp;
};

class Logger
{
public:
    Logger(Config &config, FanucIo &fanucInfo, std::vector<bool> &settings);
    void log();
    void storeLayerInfo(int num, bool start);
    void storeOffsets();
private:
    Config & mConfig;
    FanucIo & mFanucInfo;
    std::vector<bool> & mSettings;
    dbl::DblManager mDblManager;
    void storeJsonValues(const std::string &title, std::map<std::string, long> &values);
    void storeJsonValues(const std::string &title, std::map<std::string, float> &values);
    void storeAlarms();
    void storeState();
    void storeTorques();
    void storePositions();
    void storeLoads();
    void storeMotorSpeed();
    void storeTemperatures();
private:
    ModRecord readRS2Reg();
    std::mutex lcm;
};

#endif  // CORE_LOGGER_H
