//
// Created by andrey on 11/24/20.
//

#include "logger.h"
#include "../../common/fileops.h"
#include <thread>
#include "globalstate.h"

const int SettingOutputJson = 1;

inline float makeFloat(long value, short dec)
{
    static float powm10[10] = {1.f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f, 0.00000001f, 0.000000001f};
    return value*powm10[dec];
}

Logger::Logger(Config &config, FanucIo &fanucInfo, std::vector<bool> &settings)
    : mConfig(config), mFanucInfo(fanucInfo),
      mSettings(settings), mDblManager(config.dblAddress(), config.dblPort())
{

}

void Logger::storeJsonValues(const std::string & title, std::map<std::string, long> & values)
{
    if (!values.empty()) {
        json record = {{"measurement", title}, {"time", get_iso_time()}, {"fields", json(values)}};
        if (!mConfig.noDbl())
            mDblManager.store(record);
        if (mSettings[SettingOutputJson])
            std::cout << record << std::endl;
    }
}

void Logger::storeJsonValues(const std::string & title, std::map<std::string, float> & values)
{
    if (!values.empty()) {
        json record = {{"measurement", title}, {"time", get_iso_time()}, {"fields", json(values)}};
        if (!mConfig.noDbl())
            mDblManager.store(record);
        if (mSettings[SettingOutputJson])
            std::cout << record << std::endl;
    }
}

void Logger::storeAlarms()
{
    std::lock_guard lock(lcm);
    json record;
    if (mFanucInfo.alarmsSet()) {
        record = {{"measurement", "Alarms"},
                  {"time", get_iso_time()},
                  {"fields", json::parse(R"({"Descriptions" : ")" + mFanucInfo.alarmDescriptions() + "\"}")}};
        if (mSettings[SettingOutputJson])
            std::cout << record << std::endl;
        if (!mConfig.noDbl())
            mDblManager.store(record);
    } else {
        record = {
            {"measurement", "Alarms"}, {"time", get_iso_time()}, {"fields", json({{"Description", "None"}})}};
        if (mSettings[SettingOutputJson])
            std::cout << record << std::endl;
        if (!mConfig.noDbl())
            mDblManager.store(record);
    }
}

void Logger::log()
{
    if (!mFanucInfo.failed()) {
        if (mConfig.logState()) {
            storeAlarms();
            storeState();
        }
        if (mConfig.logTorques())
            storeTorques();
        if (mConfig.logPositions())
            storePositions();
        if (mConfig.logLoads())
            storeLoads();
        if (mConfig.logSpeeds())
            storeMotorSpeed();
        if (mConfig.logTemperatures())
            storeTemperatures();
    }
}

void Logger::storeState()
{
    std::lock_guard lock(lcm);
    json record;
    if (mFanucInfo.state().stopped) {
        record = {{"measurement", "State"}, {"time", get_iso_time()}, {"fields", json({{"values", "Stopped"}})}};
        boost::filesystem::remove(mConfig.lockFile());
        spdlog::info("{} unlocked", mConfig.lockFile());
    } else {
        record = {{"measurement", "State"}, {"time", get_iso_time()}, {"fields", json({{"values", "Running"}})}};
        createFile(mConfig.lockFile());
        spdlog::info("{} locked", mConfig.lockFile());
    }
    if (mSettings[SettingOutputJson])
        std::cout << record << std::endl;
    if (!mConfig.noDbl())
        mDblManager.store(record);
    if (mFanucInfo.state().alarm) {
        record = {{"measurement", "State"}, {"time", get_iso_time()}, {"fields", json({{"values", "Alarm"}})}};
        if (mSettings[SettingOutputJson])
            std::cout << record << std::endl;
        if (!mConfig.noDbl())
            mDblManager.store(record);
    }
}

void Logger::storeTorques()
{
    std::lock_guard lock(lcm);
    std::map<std::string, long> values;
    json record;
    auto torques = mFanucInfo.spindleTorques();
    for (int i = 0; i < torques.size(); i++)
        values.insert(std::pair<std::string, long>(fmt::format("axis{0}", i), torques[i]));
    storeJsonValues("Spindle Torques", values);
}

void Logger::storePositions()
{
    std::lock_guard lock(lcm);
    std::map<std::string, float> values;
    json record;
    for (const auto &e : mFanucInfo.absPosition()) {
        values.insert(std::pair<std::string, float>(std::string(&e.axisName, 1), makeFloat(e.data, e.dec)));
    }
    storeJsonValues("Absolute Positions", values);
    values.clear();
    for (const auto &e : mFanucInfo.relPosition()) {
        values.insert(std::pair<std::string, float>(std::string(&e.axisName, 1), makeFloat(e.data, e.dec)));
    }
    storeJsonValues("Relative Positions",values);
    values.clear();
    for (const auto &e : mFanucInfo.machPosition()) {
        values.insert(std::pair<std::string, float>(std::string(&e.axisName, 1), makeFloat(e.data, e.dec)));
    }
    storeJsonValues("Machine Positions", values);
    values.clear();
    for (const auto &e : mFanucInfo.distToGo()) {
        values.insert(std::pair<std::string, float>(std::string(&e.axisName, 1), makeFloat(e.data, e.dec)));
    }
    storeJsonValues("Distance to Go",values);
}

void Logger::storeLoads()
{
    std::lock_guard lock(lcm);
    std::map<std::string, float> values;
    json record;
    for (const auto & e : mFanucInfo.servoLoadData()) {
        values.insert(std::pair<std::string, float>(std::string(&e.axisName,1), makeFloat(e.data, e.dec)));
    }
    storeJsonValues("Servo Loads", values);
    values.clear();
    for (const auto &e : mFanucInfo.spindleLoadData()) {
        values.insert(std::pair<std::string, float>(std::string(&e.axisName, 1), makeFloat(e.data, e.dec)));
    }
    storeJsonValues("Spindle Loads", values);
}

void Logger::storeMotorSpeed()
{
    std::lock_guard lock(lcm);
    json record;
    record = {
        {"measurement", "Motor Speed"},
        {"time", get_iso_time()},
        {
         "fields",
                        {{"value", std::to_string(mFanucInfo.motorSpeed())}},
        }};
    if (!mConfig.noDbl())
        mDblManager.store(record);
    if (mSettings[SettingOutputJson])
        std::cout << record << std::endl;
}

void Logger::storeTemperatures()
{
    std::lock_guard lock(lcm);
    json record = {
        {"measurement", "Servo Temperatures"},
        {"time", get_iso_time()},
        {"fields", {
                            {"X", std::to_string((int)mFanucInfo.servoTemperatures()[0])},
                            {"Y", std::to_string((int)mFanucInfo.servoTemperatures()[1])},
                            {"Z", std::to_string((int)mFanucInfo.servoTemperatures()[2])},
                            {"B", std::to_string((int)mFanucInfo.servoTemperatures()[3])},
                        }
        }};
    if (!mConfig.noDbl())
        mDblManager.store(record);
    if (mSettings[SettingOutputJson])
        std::cout << record << std::endl;
    record = {
        {"measurement", "Pulse Coder Temperatures"},
        {"time", get_iso_time()},
        {"fields", {
                            {"X", std::to_string((int)mFanucInfo.pcTemperatures()[0])},
                            {"Y", std::to_string((int)mFanucInfo.pcTemperatures()[1])},
                            {"Z", std::to_string((int)mFanucInfo.pcTemperatures()[2])},
                            {"B", std::to_string((int)mFanucInfo.pcTemperatures()[3])},
                        }
        }
    };
    if (!mConfig.noDbl())
        mDblManager.store(record);
    if (mSettings[SettingOutputJson])
        std::cout << record << std::endl;
}

ModRecord Logger::readRS2Reg()
{
    ModRecord result{};
    //mModManager.ReadRegister(1,100, &result.value, &result.timestamp);
    return result;
}

void Logger::storeLayerInfo(int num, bool start)
{
    spdlog::info("LAYER {0}, {1}", num, start);
    std::lock_guard lock(lcm);
    json record = {
        {"measurement", "Layers"},
        {"time", get_iso_time()},
        {"fields",
            {
                {"layer", num},
                {"status", start ? "started" : "finished"}
            }
        }
    };
    if (mSettings[SettingOutputJson])
        std::cout << record << std::endl;
    if (!mConfig.noDbl())
        mDblManager.store(record);
}

namespace ns {
void to_json(json& j, const FanucToolOffset& r) {
    j = json{ {"num", r.toolNumber}, {"type", r.offsetType}, {"value", r.value} };
}

json to_json(const FanucToolOffset& r)
{
    return json{ {"num", r.toolNumber}, {"type", r.offsetType}, {"value", r.value} };
}

json to_json(const FanucIo::ZOffset& r)
{
    return json{ {"G54", r.g54}, {"G55", r.g55}, {"G56", r.g56}, {"G57", r.g57}, {"G58", r.g58}, {"G59", r.g59}};
}


void from_json(const json& j, FanucToolOffset& r) {
    j.at("num").get_to(r.toolNumber);
    j.at("type").get_to(r.offsetType);
    j.at("value").get_to(r.value);
}

}

void Logger::storeOffsets()
{
    auto makeOffsetsTable = [&](std::vector<FanucToolOffset> & lengths, std::vector<FanucToolOffset> & radiuses)
    {
        json fields;
        int cr = 0;
        for(int i = 0; i < lengths.size(); ++i) {
            for (int j = cr; j < radiuses.size(); ++j)
                if (lengths[i].toolNumber == radiuses[j].toolNumber) {
                    fields.push_back({{"Tool Number", lengths[i].toolNumber},
                                      {"Length", lengths[i].value},
                                      {"Diameter", radiuses[i].value}});
                    cr = j;
                }
            if (!fields.contains("Diameter"))
                fields.push_back({"Tool", { {"Tool Number", lengths[i].toolNumber}, {"Length", lengths[i].value}, {"Diameter", 0}}});;
        }
       json record{{"measurement", "Tool Offsets"}, {"time", get_iso_time()}, {"fields", fields}};
      if (!mConfig.noDbl())
          mDblManager.store(record);
      if (mSettings[SettingOutputJson])
          std::cout << record << std::endl;
    };
    std::lock_guard lock(lcm);
    std::vector<FanucToolOffset> offsets;
    if (GlobalState::instance().isConnected()) {
        auto [v1, v2, v3] = mFanucInfo.readOffsets();
        json r;
        /*for(const auto & o : v2)
            r.push_back(ns::to_json(o));
        r.push_back(json("{\"length\" : 0}"));
        spdlog::info(r.dump());
        r.clear();*/
        makeOffsetsTable(v1, v2);
        auto shifts = mFanucInfo.readAxesShifts2();
        //shifts[0].
        if (!shifts.empty()) {
            r = {"Shifts", {"X", shifts[0]}, {"Y", shifts[1]}, {"Z", shifts[2]}, {"B", shifts[3]}};
            spdlog::info(r.dump());
        }
        auto cd = mFanucInfo.readCalibratioData();
        r = {{"r520", cd.r520}, {"r521", cd.r521}, {"r522", cd.r522}, {"r523", cd.r523},
                                  {"r524", cd.r524},  {"r525", cd.r525}, {"r526", cd.r526}, {"r527", cd.r527}};
        json record{{"measurement", "Calibration"}, {"time", get_iso_time()}, {"fields", r}};
        if (!mConfig.noDbl())
            mDblManager.store(record);
        if (mSettings[SettingOutputJson])
            std::cout << record << std::endl;

        auto zoffs = mFanucInfo.readZOffsets();
        record = {{"measurement", "Workspaces"}, {"time", get_iso_time()}, {"fields", {
                                  {"G54", {"X", zoffs.x.g54}, {"Y", zoffs.y.g54}, {"Z", zoffs.z.g54}},
                                  {"G55", {"X", zoffs.x.g55}, {"Y", zoffs.y.g55}, {"Z", zoffs.z.g55}},
                                  {"G56", {"X", zoffs.x.g56}, {"Y", zoffs.y.g56}, {"Z", zoffs.z.g56}},
                                  {"G57", {"X", zoffs.x.g57}, {"Y", zoffs.y.g57}, {"Z", zoffs.z.g57}},
                                  {"G58", {"X", zoffs.x.g58}, {"Y", zoffs.y.g58}, {"Z", zoffs.z.g58}},
                                  {"G59", {"X", zoffs.x.g59}, {"Y", zoffs.y.g59}, {"Z", zoffs.z.g59}},
                                                                                            }}
                                                                                       };
        if (!mConfig.noDbl())
            mDblManager.store(record);
        if (mSettings[SettingOutputJson])
            std::cout << record << std::endl;
        r = {"Zero Offsets", {{"X", ns::to_json(zoffs.x)}, {"Y", ns::to_json(zoffs.y)}, {"Z", ns::to_json(zoffs.z)}, {"B", ns::to_json(zoffs.b)}}};
        spdlog::info(r.dump());
    }
    else
        spdlog::info("Reading offsets: fanuc is not connected");
}
