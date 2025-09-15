// =============================================
// 文件: llm_client.hpp
// 描述: 调用OpenAI Chat Completions API的简单客户端（WinHTTP）
// 依赖: Windows WinHTTP, nlohmann::json
// =============================================

#pragma once

#include <string>
#include <functional>

// 简单同步客户端：将上下文转成提示词，返回模型建议文本。
// - 使用环境变量 OPENAI_API_KEY 读取密钥
// - 可用 OPENAI_API_BASE 覆盖Base URL（默认 https://api.openai.com）
// - 可用 OPENAI_MODEL 覆盖模型名（默认 gpt-4o-mini）
//
// 返回：成功时为建议文本；失败时返回以 "[LLM错误] " 开头的错误信息，便于直接显示到消息窗口。
std::string RequestOpenAISuggestion(const std::string& prompt);

// 流式传输：逐步回调增量文本。返回是否成功发起请求（同步返回）。
// on_delta: 每次解析到一段文本即回调；当流结束时会以空字符串回调一次表示完成。
bool StreamOpenAISuggestion(const std::string& prompt,
                            const std::function<void(const std::string&)>& on_delta);


