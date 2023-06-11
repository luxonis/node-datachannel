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
              InstanceMethod("lastReportedTimestamp", &RtcpSrReporterWrapper::lastReportedTimestamp),
              InstanceMethod("configSetTimestamp", &RtcpSrReporterWrapper::configSetTimestamp),
              InstanceMethod("configGetTimestamp", &RtcpSrReporterWrapper::configGetTimestamp),
              InstanceMethod("configSecondsToTimestamp", &RtcpSrReporterWrapper::configSecondsToTimestamp),
              InstanceMethod("configTimestampToSeconds", &RtcpSrReporterWrapper::configTimestampToSeconds),
              InstanceMethod("setNeedsToReport", &RtcpSrReporterWrapper::setNeedsToReport),
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

Napi::Value RtcpSrReporterWrapper::lastReportedTimestamp(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "lastReportedTimestamp() called on invalid reporter").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    return Napi::Number::New(info.Env(), mRtcpSrReporterPtr->lastReportedTimestamp());
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

void RtcpSrReporterWrapper::configSetTimestamp(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "configSetTimestamp() called on invalid reporter").ThrowAsJavaScriptException();
        return;
    }

    if (info.Length() < 1 || !info[0].IsNumber())
    {
        Napi::Error::New(info.Env(), "configSetTimestamp() called with invalid parameters").ThrowAsJavaScriptException();
        return;
    }

    mRtcpSrReporterPtr->rtpConfig->timestamp = info[0].As<Napi::Number>().Uint32Value();
}

Napi::Value RtcpSrReporterWrapper::configSecondsToTimestamp(const Napi::CallbackInfo &info)
{
    if (!mRtcpSrReporterPtr)
    {
        Napi::Error::New(info.Env(), "configSecondsToTimestamp() called on invalid reporter").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    if (info.Length() < 1 || !info[0].IsNumber())
    {
        Napi::Error::New(info.Env(), "configSecondsToTimestamp() called with invalid parameters").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    return Napi::Number::New(info.Env(), mRtcpSrReporterPtr->rtpConfig->secondsToTimestamp(info[0].As<Napi::Number>().DoubleValue()));
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
