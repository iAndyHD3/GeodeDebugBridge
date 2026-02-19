#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <glaze/glaze.hpp>

#include <set>
#include <mutex>
#include <thread>
#include <deque>

#include "Geode/DefaultInclude.hpp"
#include "Geode/loader/Log.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/web.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

using WS_Server = websocketpp::server<websocketpp::config::asio>;
using connection_hdl = websocketpp::connection_hdl;

struct LogEntry {
    std::string severity;
    int32_t nestCount{};
    std::string thread;
    std::string source;
    std::string modID;
    std::string timestamp;
    std::string tag;
    std::string message;
};

$execute {
    static WS_Server wsServer;
    static std::set<connection_hdl, std::owner_less<connection_hdl>> activeConnections;
    static std::deque<std::string> logHistory;
    static constexpr size_t MAX_HISTORY = 300;
    static std::mutex mutex;
    static std::string jsonHolder;

    constexpr int startPort = 51500;
    constexpr int maxPorts = 15;

    wsServer.set_access_channels(websocketpp::log::alevel::none);
    wsServer.set_error_channels(websocketpp::log::elevel::none);

    wsServer.init_asio();


    // Find an available port in the 51500 - 51515 range
    static uint16_t activePort = startPort;
    websocketpp::lib::error_code ec;
    
    while (activePort <= startPort + maxPorts) {
        wsServer.listen(websocketpp::lib::asio::ip::tcp::endpoint(
            websocketpp::lib::asio::ip::address_v4::any(), activePort), ec);
        if (!ec) break; // Successfully bound!
        activePort++;
    }

    if (ec) {
        log::error("Debug Bridge: Failed to bind to any port in range 51500-51515");
        return;
    }

    wsServer.set_open_handler([](connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(mutex);

        // 1. Send connection handshake identifying the instance
        std::string handshake = fmt::format(
            R"({{"type":"handshake", "platform":"{}", "port":{}}})", 
            GEODE_PLATFORM_NAME, activePort
        );
        websocketpp::lib::error_code send_ec;
        wsServer.send(hdl, handshake, websocketpp::frame::opcode::text, send_ec);

        // 2. Send the missed log history
        for (const auto& msg : logHistory) {
            wsServer.send(hdl, msg, websocketpp::frame::opcode::text, send_ec);
        }

        activeConnections.insert(hdl);
        log::info("Debug Bridge: New client connected (sent {} missed logs)", logHistory.size());
    });

    wsServer.set_close_handler([](connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(mutex);
        activeConnections.erase(hdl);
        log::info("Debug Bridge: Client disconnected");
    });

    wsServer.start_accept();
    log::warn("Debug Bridge: Listening on ws://0.0.0.0:{} (Platform: {})", activePort, GEODE_PLATFORM_NAME);

    static std::thread wsThread([]() {
        wsServer.run();
    });

    jsonHolder.reserve(1024);

    log::LogEvent().listen([](const log::BorrowedLog& log) -> bool {
        std::string severityStr;
        switch (log.m_severity.m_value) {
            case 0: severityStr = "DEBUG"; break;
            case 1: severityStr = "INFO"; break;
            case 2: severityStr = "WARN"; break;
            case 3: severityStr = "ERROR"; break;
            default: severityStr = "UNKNOWN"; break;
        }

        std::string tag;
        std::string message = std::string(log.m_content);
        if (!log.m_content.empty() && log.m_content.front() == '[') {
            auto closePos = log.m_content.find(']');
            if (closePos != std::string_view::npos && closePos > 0) {
                tag = std::string(log.m_content.substr(1, closePos - 1));
                size_t msgStart = closePos + 1;
                if (msgStart < log.m_content.size() && log.m_content[msgStart] == ' ') {
                    ++msgStart;
                }
                message = std::string(log.m_content.substr(msgStart));
            }
        }

        LogEntry entry{
            .severity  = std::move(severityStr),
            .nestCount = log.m_nestCount,
            .thread    = std::string(log.m_thread),
            .source    = std::string(log.m_source),
            .modID     = log.m_mod ? std::string(log.m_mod->getID()) : "<system>",
            .timestamp = log.m_time.toString(true),
            .tag       = std::move(tag),
            .message   = std::move(message),
        };

        [[maybe_unused]] auto glz_ec = glz::write_json(entry, jsonHolder);

        std::lock_guard<std::mutex> lock(mutex);

        logHistory.push_back(jsonHolder);
        if (logHistory.size() > MAX_HISTORY) {
            logHistory.pop_front();
        }
        
        if(jsonHolder.max_size() > 1024) {
            jsonHolder.shrink_to_fit();
            jsonHolder.reserve(1024);
        }

        for (auto const& hdl : activeConnections) {
            websocketpp::lib::error_code send_ec;
            wsServer.send(hdl, jsonHolder, websocketpp::frame::opcode::text, send_ec);
        }

        return false;
    }).leak();
}

$on_game(Loaded) {
    listenForKeybindSettingPresses("open-web-log-browser", [](Keybind const& keybind, bool down, bool repeat, double timestamp) {
        log::info("here");
        if (down && !repeat) {
        log::info("here2");

            auto path = Mod::get()->getResourcesDir() / "Geode Debug Bridge.html";
            if(!std::filesystem::exists(path)) {
        log::info("here3");
                return geode::Notification::create("HTML File not found!", NotificationIcon::Error)->show();
            }
                    log::info("here4");

            geode::utils::web::openLinkInBrowser(path.string());
        }
    });
}