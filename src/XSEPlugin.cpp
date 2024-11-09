#include "ENBAOToggler.h"
#include "Events.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:

		ENBAOToggler::GetSingleton()->g_ENB = reinterpret_cast<ENB_API::ENBSDKALT1001*>(ENB_API::RequestENBAPI(ENB_API::SDKVersion::V1001));
		if (ENBAOToggler::GetSingleton()->g_ENB) {
			logger::info("Obtained ENB API");
			ENBAOToggler::GetSingleton()->g_ENB->SetCallbackFunction([](ENBCallbackType calltype) {
				switch (calltype) {
				case ENBCallbackType::ENBCallback_EndFrame:
					ENBAOToggler::GetSingleton()->UpdateENBParams();
					break;
				case ENBCallbackType::ENBCallback_BeginFrame:
					break;
				case ENBCallbackType::ENBCallback_PostLoad:
					ENBAOToggler::GetSingleton()->LoadJSON();
					break;
				case ENBCallbackType::ENBCallback_PreSave:
					ENBAOToggler::GetSingleton()->SaveJSON();
					break;
				default:
					ENBAOToggler::GetSingleton()->UpdateUI();
					break;
				}
			});
		} else
			logger::info("Unable to acquire ENB API");

		break;
	case SKSE::MessagingInterface::kNewGame:
		ENBAOToggler::GetSingleton()->LoadJSON();
		break;

	case SKSE::MessagingInterface::kPostLoadGame:
		ENBAOToggler::GetSingleton()->LoadJSON();
		break;
	}
}

void Init()
{
	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);
	Events::Register();
	ENBAOToggler::GetSingleton()->LoadJSON();
}
void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format("{}.log"sv, SKSE::PluginDeclaration::GetSingleton()->GetName());
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {};
#endif

	InitializeLog();

	logger::info("Loaded plugin");

	SKSE::Init(skse);

	Init();

	return true;
}