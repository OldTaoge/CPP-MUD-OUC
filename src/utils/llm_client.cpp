#include "llm_client.hpp"

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <functional>

#include "../vendor/nlohmann/json.hpp"
using nlohmann::json;

static std::string GetEnvOrDefault(const char* name, const char* defv) {
    const char* v = std::getenv(name);
    if (!v || !*v) return defv ? std::string(defv) : std::string();
    return std::string(v);
}

std::string RequestOpenAISuggestion(const std::string& prompt) {
#ifndef _WIN32
    return std::string("[LLM错误] 仅在Windows/WinHTTP上实现");
#else
    std::string api_key = GetEnvOrDefault("OPENAI_API_KEY", "sk-monvkkdrrtrzxloqizjodkdhiyywmmecjwgoxenigkndddwv");
    if (api_key.empty()) {
        return std::string("[LLM错误] 未设置OPENAI_API_KEY环境变量");
    }
    std::string api_base = GetEnvOrDefault("OPENAI_API_BASE", "https://api.siliconflow.cn");
    std::string model = GetEnvOrDefault("OPENAI_MODEL", "deepseek-ai/DeepSeek-V3");

    // 仅支持标准基址，简单处理：https://host[:port]
    std::wstring host_w;
    INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;
    bool use_https = true;
    {
        // 粗略解析 host 与端口
        std::string base = api_base;
        if (base.rfind("https://", 0) == 0) {
            base = base.substr(8);
            use_https = true;
        } else if (base.rfind("http://", 0) == 0) {
            base = base.substr(7);
            use_https = false;
            port = INTERNET_DEFAULT_HTTP_PORT;
        }
        auto slash = base.find('/');
        if (slash != std::string::npos) base = base.substr(0, slash);
        auto colon = base.find(':');
        if (colon != std::string::npos) {
            port = static_cast<INTERNET_PORT>(std::stoi(base.substr(colon + 1)));
            base = base.substr(0, colon);
        }
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, base.c_str(), -1, NULL, 0);
        host_w.resize(size_needed - 1);
        MultiByteToWideChar(CP_UTF8, 0, base.c_str(), -1, &host_w[0], size_needed);
    }

    HINTERNET hSession = WinHttpOpen(L"CPP_MUD_OUC/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return std::string("[LLM错误] WinHttpOpen失败");
    
    // 设置超时：连接30秒，发送30秒，接收60秒
    DWORD timeout = 30000; // 30秒
    WinHttpSetTimeouts(hSession, timeout, timeout, timeout, 60000);

    HINTERNET hConnect = WinHttpConnect(hSession, host_w.c_str(), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return std::string("[LLM错误] WinHttpConnect失败");
    }

    // 路径固定为 /v1/chat/completions
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/v1/chat/completions",
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            use_https ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return std::string("[LLM错误] WinHttpOpenRequest失败");
    }

    // 组装JSON请求体
    json body;
    body["model"] = model;
    body["temperature"] = 0.3;
    body["messages"] = json::array({
        json{{"role", "system"}, {"content", "你是一个MUD地牢探索游戏的智能向导，提供简洁的下一步行动建议。"}},
        json{{"role", "user"}, {"content", prompt}}
    });
    std::string body_str = body.dump();

    // 请求头
    std::wstring headers = L"Content-Type: application/json\r\nAuthorization: Bearer ";
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, api_key.c_str(), -1, NULL, 0);
        std::wstring key_w; key_w.resize(size_needed - 1);
        MultiByteToWideChar(CP_UTF8, 0, api_key.c_str(), -1, &key_w[0], size_needed);
        headers += key_w;
    }

    BOOL bResults = WinHttpSendRequest(hRequest,
                                       headers.c_str(), (DWORD)headers.size(),
                                       (LPVOID)body_str.data(), (DWORD)body_str.size(),
                                       (DWORD)body_str.size(), 0);
    if (!bResults) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return std::string("[LLM错误] WinHttpSendRequest失败");
    }

    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return std::string("[LLM错误] WinHttpReceiveResponse失败");
    }

    std::string response;
    DWORD dwSize = 0;
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        if (dwSize == 0) break;
        std::string buffer; buffer.resize(dwSize);
        DWORD dwDownloaded = 0;
        if (!WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded)) break;
        buffer.resize(dwDownloaded);
        response += buffer;
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    try {
        auto j = json::parse(response);
        if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty()) {
            auto &c = j["choices"][0];
            if (c.contains("message") && c["message"].contains("content")) {
                return c["message"]["content"].get<std::string>();
            }
        }
        if (j.contains("error") && j["error"].contains("message")) {
            return std::string("[LLM错误] ") + j["error"]["message"].get<std::string>();
        }
        return std::string("[LLM错误] 未能解析LLM响应");
    } catch (...) {
        return std::string("[LLM错误] 响应解析异常");
    }
#endif
}

bool StreamOpenAISuggestion(const std::string& prompt,
                            const std::function<void(const std::string&)>& on_delta) {
#ifndef _WIN32
    if (on_delta) on_delta("[LLM错误] 仅在Windows/WinHTTP上实现");
    return false;
#else
    std::string api_key = GetEnvOrDefault("OPENAI_API_KEY", "sk-monvkkdrrtrzxloqizjodkdhiyywmmecjwgoxenigkndddwv");
    if (api_key.empty()) {
        if (on_delta) on_delta("[LLM错误] 未设置OPENAI_API_KEY环境变量");
        return false;
    }
    std::string api_base = GetEnvOrDefault("OPENAI_API_BASE", "https://api.siliconflow.cn");
    std::string model = GetEnvOrDefault("OPENAI_MODEL", "deepseek-ai/DeepSeek-V3");
    
    // 解析主机端口
    std::wstring host_w;
    INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;
    bool use_https = true;
    {
        std::string base = api_base;
        if (base.rfind("https://", 0) == 0) {
            base = base.substr(8);
            use_https = true;
        } else if (base.rfind("http://", 0) == 0) {
            base = base.substr(7);
            use_https = false;
            port = INTERNET_DEFAULT_HTTP_PORT;
        }
        auto slash = base.find('/');
        if (slash != std::string::npos) base = base.substr(0, slash);
        auto colon = base.find(':');
        if (colon != std::string::npos) {
            port = static_cast<INTERNET_PORT>(std::stoi(base.substr(colon + 1)));
            base = base.substr(0, colon);
        }
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, base.c_str(), -1, NULL, 0);
        host_w.resize(size_needed - 1);
        MultiByteToWideChar(CP_UTF8, 0, base.c_str(), -1, &host_w[0], size_needed);
    }

    HINTERNET hSession = WinHttpOpen(L"CPP_MUD_OUC/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { if (on_delta) on_delta("[LLM错误] WinHttpOpen失败"); return false; }
    
    // 设置超时：连接30秒，发送30秒，接收60秒
    DWORD timeout = 30000; // 30秒
    WinHttpSetTimeouts(hSession, timeout, timeout, timeout, 60000);
    HINTERNET hConnect = WinHttpConnect(hSession, host_w.c_str(), port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); if (on_delta) on_delta("[LLM错误] WinHttpConnect失败"); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/v1/chat/completions",
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            use_https ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        if (on_delta) on_delta("[LLM错误] WinHttpOpenRequest失败");
        return false;
    }

    // 构造请求，开启流式
    json body;
    body["model"] = model;
    body["temperature"] = 0.7;
    body["stream"] = true;
    // 注释掉 enable_thinking 以减少延迟
    // body["enable_thinking"] = true;
    body["messages"] = json::array({
        json{{"role", "system"}, {"content", "你是一个MUD地牢探索游戏的智能向导，提供简洁的下一步行动建议。"}},
        json{{"role", "user"}, {"content", prompt}}
    });
    std::string body_str = body.dump();

    std::wstring headers = L"Content-Type: application/json\r\nAuthorization: Bearer ";
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, api_key.c_str(), -1, NULL, 0);
        std::wstring key_w; key_w.resize(size_needed - 1);
        MultiByteToWideChar(CP_UTF8, 0, api_key.c_str(), -1, &key_w[0], size_needed);
        headers += key_w;
    }

    BOOL bResults = WinHttpSendRequest(hRequest,
                                       headers.c_str(), (DWORD)headers.size(),
                                       (LPVOID)body_str.data(), (DWORD)body_str.size(),
                                       (DWORD)body_str.size(), 0);
    if (!bResults) {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        if (on_delta) on_delta("[LLM错误] WinHttpSendRequest失败");
        return false;
    }
    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults) {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        if (on_delta) on_delta("[LLM错误] WinHttpReceiveResponse失败");
        return false;
    }

    // 逐块读取，解析SSE风格的数据: 每行以 "data: {json}"，末尾 "data: [DONE]"
    std::string chunk;
    DWORD dwSize = 0;
    while (true) {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        if (dwSize == 0) break;
        std::string buffer; buffer.resize(dwSize);
        DWORD dwDownloaded = 0;
        if (!WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded)) break;
        buffer.resize(dwDownloaded);
        chunk += buffer;

        // 按行处理
        size_t pos = 0;
        while (true) {
            size_t nl = chunk.find('\n', pos);
            if (nl == std::string::npos) {
                // 保留未完整行
                chunk = chunk.substr(pos);
                break;
            }
            std::string line = chunk.substr(pos, nl - pos);
            pos = nl + 1;

            // 去除\r
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.rfind("data:", 0) != 0) continue;
            std::string payload = line.substr(5);
            // 去掉前导空格
            while (!payload.empty() && (payload[0] == ' ')) payload.erase(payload.begin());
            if (payload == "[DONE]") {
                if (on_delta) on_delta("");
                break;
            }
            try {
                auto j = json::parse(payload);
                if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty()) {
                    const auto &delta = j["choices"][0]["delta"];
                    if (delta.contains("content") && !delta["content"].is_null()) {
                        if (on_delta) on_delta(delta["content"].get<std::string>());
                    }
                }
            } catch (...) {
                // 忽略解析错误的行
            }
        }
    }

    WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
    return true;
#endif
}


