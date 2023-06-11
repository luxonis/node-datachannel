#ifndef MEDIA_RTCPSRREPORTER_WRAPPER_H
#define MEDIA_RTCPSRREPORTER_WRAPPER_H

#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <unordered_set>

#include <napi.h>
#include <rtc/rtc.hpp>

#include "thread-safe-callback.h"

class RtcpSrReporterWrapper : public Napi::ObjectWrap<RtcpSrReporterWrapper>
{
public:
    static Napi::FunctionReference constructor;
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    RtcpSrReporterWrapper(const Napi::CallbackInfo &info);
    ~RtcpSrReporterWrapper();

    // Functions
    Napi::Value lastReportedTimestamp(const Napi::CallbackInfo &info);
    void configSetTimestamp(const Napi::CallbackInfo &info);
    Napi::Value configGetTimestamp(const Napi::CallbackInfo &info);
    Napi::Value configSecondsToTimestamp(const Napi::CallbackInfo &info);
    Napi::Value configTimestampToSeconds(const Napi::CallbackInfo &info);
    void setNeedsToReport(const Napi::CallbackInfo &info);

    // Close all existing track
    static void CloseAll();

private:
    static std::unordered_set<RtcpSrReporterWrapper *> instances;

    void doClose();

    std::shared_ptr<rtc::RtcpSrReporter> mRtcpSrReporterPtr = nullptr;
};

#endif // MEDIA_RTCPSRREPORTER_WRAPPER_H