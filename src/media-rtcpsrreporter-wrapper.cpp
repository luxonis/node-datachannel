#include "media-rtcpsrreporter-wrapper.h"

Napi::FunctionReference RtcpSrReporterWrapper::constructor;
std::unordered_set<RtcpSrReporterWrapper *> RtcpSrReporterWrapper::instances;

void RtcpSrReporterWrapper::CloseAll()
{
    auto copy(instances);
    for (auto inst : copy)
        inst->doClose();
}

Napi::Object RtcpSrReporterWrapper::Init(Napi::Env env, Napi::Object exports)
{
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
          env,
          "RtcpSrReporter",
          {
              InstanceMethod("previousReportedTimestamp", &RtcpSrReporterWrapper::previousReportedTimestamp),
              InstanceMethod("configGetTimestamp", &RtcpSrReporterWrapper::configGetTimestamp),
              InstanceMethod("configTimestampToSeconds", &RtcpSrReporterWrapper::configTimestampToSeconds),
              InstanceMethod("setNeedsToReport", &RtcpSrReporterWrapper::setNeedsToReport),
              InstanceMethod("setStartTimeToCurrent", &RtcpSrReporterWrapper::setStartTimeToCurrent),
              InstanceMethod("startRecording", &RtcpSrReporterWrapper::startRecording),
          }
      );
  
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("RtcpSrReporter", func);
  return exports; 
}

RtcpSrReporterWrapper::RtcpSrReporterWrapper(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RtcpSrReporterWrapper>(info)
{
    mRtcpSrReporterPtr = *(info[0].As<Napi::External<std::shared_ptr<rtc::RtcpSrReporter>>>().Data());

    instances.insert(this);
}

RtcpSrReporterWrapper::~RtcpSrReporterWrapper()
{
    doClose();
}

void RtcpSrReporterWrapper::doClose()
{
    instances.erase(this);
}

Napi::Value RtcpSrReporterWrapper::previousReportedTimestamp(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "previousReportedTimestamp() called on invalid reporter").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    return Napi::Number::New(info.Env(), mRtcpSrReporterPtr->previousReportedTimestamp);
}

Napi::Value RtcpSrReporterWrapper::configGetTimestamp(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "configGetTimestamp() called on invalid reporter").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    return Napi::Number::New(info.Env(), mRtcpSrReporterPtr->rtpConfig->timestamp);
}

Napi::Value RtcpSrReporterWrapper::configTimestampToSeconds(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "configTimestampToSeconds() called on invalid reporter").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    if (info.Length() < 1 || !info[0].IsNumber())
    {
        Napi::Error::New(info.Env(), "configTimestampToSeconds() called with invalid parameters").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    return Napi::Number::New(info.Env(), mRtcpSrReporterPtr->rtpConfig->timestampToSeconds(info[0].As<Napi::Number>().Uint32Value()));
}

void RtcpSrReporterWrapper::setNeedsToReport(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "setNeedsToReport() called on invalid reporter").ThrowAsJavaScriptException();
        return;
    }

    mRtcpSrReporterPtr->setNeedsToReport();
}

void RtcpSrReporterWrapper::setStartTimeToCurrent(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "setStartTimeToCurrent() called on invalid reporter").ThrowAsJavaScriptException();
        return;
    }

    auto currentTime = double(std::chrono::high_resolution_clock::now().time_since_epoch().count()) / (1000 * 1000);
    mRtcpSrReporterPtr->rtpConfig->setStartTime(currentTime, rtc::RtpPacketizationConfig::EpochStart::T1970);
}


void RtcpSrReporterWrapper::startRecording(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "startRecording() called on invalid reporter").ThrowAsJavaScriptException();
        return;
    }

    mRtcpSrReporterPtr->startRecording();
}
