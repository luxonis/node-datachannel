#include "media-track-wrapper.h"
#include "media-direction.h"
#include "media-rtcpreceivingsession-wrapper.h"
#include "media-rtcpsrreporter-wrapper.h"

Napi::FunctionReference TrackWrapper::constructor;
std::unordered_set<TrackWrapper *> TrackWrapper::instances;

void TrackWrapper::CloseAll()
{
    auto copy(instances);
    for (auto inst : copy)
        inst->doClose();
}

Napi::Object TrackWrapper::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(
        env,
        "Track",
        {
            InstanceMethod("direction", &TrackWrapper::direction),
            InstanceMethod("mid", &TrackWrapper::mid),
            InstanceMethod("type", &TrackWrapper::type),
            InstanceMethod("close", &TrackWrapper::close),
            InstanceMethod("sendMessage", &TrackWrapper::sendMessage),
            InstanceMethod("sendMessageBinary", &TrackWrapper::sendMessageBinary),
            InstanceMethod("isOpen", &TrackWrapper::isOpen),
            InstanceMethod("isClosed", &TrackWrapper::isClosed),
            InstanceMethod("maxMessageSize", &TrackWrapper::maxMessageSize),
            InstanceMethod("requestKeyframe", &TrackWrapper::requestKeyframe),
            InstanceMethod("setMediaHandler", &TrackWrapper::setMediaHandler),
            InstanceMethod("setH264Packetizer", &TrackWrapper::setH264Packetizer),
            InstanceMethod("onOpen", &TrackWrapper::onOpen),
            InstanceMethod("onClosed", &TrackWrapper::onClosed),
            InstanceMethod("onError", &TrackWrapper::onError),
            InstanceMethod("onMessage", &TrackWrapper::onMessage),
        });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Track", func);
    return exports;
}

TrackWrapper::TrackWrapper(const Napi::CallbackInfo &info) : Napi::ObjectWrap<TrackWrapper>(info)
{
    mTrackPtr = *(info[0].As<Napi::External<std::shared_ptr<rtc::Track>>>().Data());

    instances.insert(this);
}

TrackWrapper::~TrackWrapper()
{
    doClose();
}

void TrackWrapper::doClose()
{
    if (mTrackPtr)
    {
        try
        {
            mTrackPtr->close();
            mTrackPtr.reset();
        }
        catch (std::exception &ex)
        {
            std::cerr << std::string("libdatachannel error while closing track: ") + ex.what() << std::endl;
            return;
        }
    }

    mOnOpenCallback.reset();
    mOnClosedCallback.reset();
    mOnErrorCallback.reset();
    mOnMessageCallback.reset();

    instances.erase(this);
}

void TrackWrapper::close(const Napi::CallbackInfo &info)
{
    doClose();
}

Napi::Value TrackWrapper::direction(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "direction() called on destroyed track").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    Napi::Env env = info.Env();
    return Napi::String::New(env, directionToStr(mTrackPtr->direction()));
}

Napi::Value TrackWrapper::mid(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "mid() called on destroyed track").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    Napi::Env env = info.Env();
    return Napi::String::New(env, mTrackPtr->mid());
}

Napi::Value TrackWrapper::type(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "type() called on destroyed track").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    Napi::Env env = info.Env();
    return Napi::String::New(env, mTrackPtr->description().type());
}

Napi::Value TrackWrapper::sendMessage(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "sendMessage() called on destroyed track").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    Napi::Env env = info.Env();
    int length = info.Length();

    // Allow call with NULL
    if (length < 1 || (!info[0].IsString() && !info[0].IsNull()))
    {
        Napi::TypeError::New(env, "String or Null expected").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    try
    {
        return Napi::Boolean::New(info.Env(), mTrackPtr->send(info[0].As<Napi::String>().ToString()));
    }
    catch (std::exception &ex)
    {
        Napi::Error::New(env, std::string("libdatachannel error while sending track message: ") + ex.what()).ThrowAsJavaScriptException();
        return Napi::Boolean::New(info.Env(), false);
    }
}

Napi::Value TrackWrapper::sendMessageBinary(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "sendMessageBinary() called on destroyed track").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    Napi::Env env = info.Env();
    int length = info.Length();

    if (length < 1 || !info[0].IsBuffer())
    {
        Napi::TypeError::New(env, "Buffer expected").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    try
    {
        Napi::Uint8Array buffer = info[0].As<Napi::Uint8Array>();
        return Napi::Boolean::New(info.Env(), mTrackPtr->send((std::byte *)buffer.Data(), buffer.ByteLength()));
    }
    catch (std::exception &ex)
    {
        Napi::Error::New(env, std::string("libdatachannel error while sending track message: ") + ex.what()).ThrowAsJavaScriptException();
        return Napi::Boolean::New(info.Env(), false);
    }
}

Napi::Value TrackWrapper::isOpen(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!mTrackPtr)
    {
        return Napi::Boolean::New(env, false);
    }

    return Napi::Boolean::New(env, mTrackPtr->isOpen());
}

Napi::Value TrackWrapper::isClosed(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!mTrackPtr)
    {
        return Napi::Boolean::New(env, true);
    }

    return Napi::Boolean::New(env, mTrackPtr->isClosed());
}

Napi::Value TrackWrapper::maxMessageSize(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!mTrackPtr)
    {
        return Napi::Number::New(info.Env(), 0);
    }

    try
    {
        return Napi::Number::New(info.Env(), mTrackPtr->maxMessageSize());
    }
    catch (std::exception &ex)
    {
        Napi::Error::New(env, std::string("libdatachannel error: ") + ex.what()).ThrowAsJavaScriptException();
        return Napi::Number::New(info.Env(), 0);
    }
}

Napi::Value TrackWrapper::requestKeyframe(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "requestKeyframe() called on destroyed track").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, mTrackPtr->requestKeyframe());
}

void TrackWrapper::setMediaHandler(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "setMediaHandler() called on destroyed track").ThrowAsJavaScriptException();
        return;
    }

    Napi::Env env = info.Env();
    int length = info.Length();

    if (length < 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "MediaHandler class instance expected").ThrowAsJavaScriptException();
        return;
    }

    RtcpReceivingSessionWrapper *handler = Napi::ObjectWrap<RtcpReceivingSessionWrapper>::Unwrap(info[0].As<Napi::Object>());
    mTrackPtr->setMediaHandler(handler->getSessionInstance());
}

Napi::Value TrackWrapper::setH264Packetizer(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "setMediaHandler() called on destroyed track").ThrowAsJavaScriptException();
        return env.Null();
    }

    int length = info.Length();

    if (length < 3 || !info[0].IsNumber() || !info[1].IsString() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "missing or invalid parameters").ThrowAsJavaScriptException();
        return env.Null();
    }

    auto rtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(
        info[0].As<Napi::Number>().Uint32Value(),
        info[1].As<Napi::String>().Utf8Value(),
        info[2].As<Napi::Number>().Uint32Value(),
        rtc::H264RtpPacketizer::defaultClockRate,
        0);
    // create packetizer
    auto packetizer =
        std::make_shared<rtc::H264RtpPacketizer>(rtc::H264RtpPacketizer::Separator::StartSequence, rtpConfig);
    // create H264 handler
    auto h264Handler = std::make_shared<rtc::H264PacketizationHandler>(packetizer);
    // add RTCP SR handler
    auto srReporter = std::make_shared<rtc::RtcpSrReporter>(rtpConfig);
    h264Handler->addToChain(srReporter);
    // add RTCP NACK handler
    auto nackResponder = std::make_shared<rtc::RtcpNackResponder>();
    h264Handler->addToChain(nackResponder);
    
    mTrackPtr->setMediaHandler(h264Handler);
    auto instance = RtcpSrReporterWrapper::constructor.New({Napi::External<std::shared_ptr<rtc::RtcpSrReporter>>::New(info.Env(), &srReporter)});

    return instance;
}

void TrackWrapper::onOpen(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "onOpen() called on destroyed track").ThrowAsJavaScriptException();
        return;
    }

    Napi::Env env = info.Env();
    int length = info.Length();

    if (length < 1 || !info[0].IsFunction())
    {
        Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
        return;
    }

    // Callback
    mOnOpenCallback = std::make_unique<ThreadSafeCallback>(info[0].As<Napi::Function>());

    mTrackPtr->onOpen([&]()
                      {
        if (mOnOpenCallback)
            mOnOpenCallback->call([this](Napi::Env env, std::vector<napi_value> &args) {
                // Check the track is not closed
                if(instances.find(this) == instances.end())
                    throw ThreadSafeCallback::CancelException();

                // This will run in main thread and needs to construct the
                // arguments for the call
                args = {};
            }); });
}

void TrackWrapper::onClosed(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "onClosed() called on destroyed track").ThrowAsJavaScriptException();
        return;
    }

    Napi::Env env = info.Env();
    int length = info.Length();

    if (length < 1 || !info[0].IsFunction())
    {
        Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
        return;
    }

    // Callback
    mOnClosedCallback = std::make_unique<ThreadSafeCallback>(info[0].As<Napi::Function>());

    mTrackPtr->onClosed([&]()
                        {
        if (mOnClosedCallback)
            mOnClosedCallback->call([this](Napi::Env env, std::vector<napi_value> &args) {
                // Do not check if the data channel has been closed here

                // This will run in main thread and needs to construct the
                // arguments for the call
                args = {};
            }); });
}

void TrackWrapper::onError(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "onError() called on destroyed track").ThrowAsJavaScriptException();
        return;
    }

    Napi::Env env = info.Env();
    int length = info.Length();

    if (length < 1 || !info[0].IsFunction())
    {
        Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
        return;
    }

    // Callback
    mOnErrorCallback = std::make_unique<ThreadSafeCallback>(info[0].As<Napi::Function>());

    mTrackPtr->onError([&](const std::string &error)
                       {
        if (mOnErrorCallback)
            mOnErrorCallback->call([this, error](Napi::Env env, std::vector<napi_value> &args) {
                // Check the track is not closed
                if(instances.find(this) == instances.end())
                    throw ThreadSafeCallback::CancelException();

                // This will run in main thread and needs to construct the
                // arguments for the call
                args = {Napi::String::New(env, error)};
            }); });
}

void TrackWrapper::onMessage(const Napi::CallbackInfo &info)
{
    if (!mTrackPtr)
    {
        Napi::Error::New(info.Env(), "onMessage() called on destroyed track").ThrowAsJavaScriptException();
        return;
    }

    Napi::Env env = info.Env();
    int length = info.Length();

    if (length < 1 || !info[0].IsFunction())
    {
        Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
        return;
    }

    // Callback
    mOnMessageCallback = std::make_unique<ThreadSafeCallback>(info[0].As<Napi::Function>());

    mTrackPtr->onMessage([&](std::variant<rtc::binary, std::string> message)
                         {
        if (mOnMessageCallback)
            mOnMessageCallback->call([this, message = std::move(message)](Napi::Env env, std::vector<napi_value> &args) {
                // Check the track is not closed
                if(instances.find(this) == instances.end())
                    throw ThreadSafeCallback::CancelException();

                // This will run in main thread and needs to construct the
                // arguments for the call
                if (std::holds_alternative<rtc::binary>(message)) // Track will always receive messages as binary
                {
                    auto bin = std::get<rtc::binary>(std::move(message));
                    args = {Napi::Buffer<std::byte>::Copy(env, bin.data(), bin.size())};
                }
            }); });
}
