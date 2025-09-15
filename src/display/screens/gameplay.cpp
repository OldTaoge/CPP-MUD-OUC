// =============================================
// 文件: gameplay.cpp
// 描述: 游戏内主界面实现。负责地图渲染、消息呈现与基本输入处理。
// =============================================
#include "gameplay.hpp"
#include "../../core/game.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/terminal.hpp>
#include <sstream>
#include <algorithm>
#include <set>
#include <thread>
#include <functional>

#include "../../utils/llm_client.hpp"
#include "../../utils/global_settings.hpp"

// 构造：初始化主游戏界面（地图/消息/状态与快捷操作）
GameplayScreen::GameplayScreen(Game* game) : game_(game) {

    // 初始化地图显示
    current_map_lines_ = {"Loading map..."};
    current_block_info_ = "Initializing...";
    
    // 如果游戏对象存在，立即更新地图
    if (game_) {
        UpdateMapDisplay();
    }
    
    // 初始化玩家基础数值，避免未定义显示
    player_level_ = 0;
    player_experience_ = 0;

    // 创建UI组件
    RebuildBottomButtons();

    // 顶栏：仅保留中间标题和右侧状态
    auto header_renderer = ftxui::Renderer([this] {
        int header_h = 3;
        std::string right_status = "Lv." + std::to_string(player_level_) +
                                   "  HP:" + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_);
        auto center_title = ftxui::text("<<原神>>MUD版") | ftxui::bold | ftxui::color(ftxui::Color::Yellow);
        auto right = ftxui::text(right_status) | ftxui::color(ftxui::Color::Cyan);
        auto bar = ftxui::hbox({
                   ftxui::filler(),
                   center_title,
                   ftxui::filler(),
                   right,
               }) | ftxui::border | ftxui::color(ftxui::Color::Blue);
        return bar | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, header_h);
    });

    // 中部：左“游戏消息”/右“游戏状态（详细）”
    auto main_renderer = ftxui::Renderer([this] {
        auto dims = ftxui::Terminal::Size();
        int header_h = 3;
        int bottom_h = 3;
        int main_h = std::max(5, dims.dimy - header_h - bottom_h);
        int message_h = std::max(5, main_h * 6 / 10 - 1); // 为消息预留约 60% 的高度，减1行给队伍区域

        // 左侧：地图显示 - 优化美观度
        std::vector<ftxui::Element> left;
        
        // 地图标题 - 显示当前区块名称
        std::string currentBlockName = "未知区域";
        auto currentBlock = game_->getMapManager().getCurrentBlock();
        if (currentBlock) {
            currentBlockName = currentBlock->getName();
            MaybeShowBlockStory(currentBlockName);
        }
        left.push_back(ftxui::text("[地图] " + currentBlockName) | ftxui::bold | ftxui::color(ftxui::Color::Cyan));
        left.push_back(ftxui::separator());
        
        // 地图内容 - 使用等宽字体和颜色，为交互元素添加精确的单位置高亮
        std::vector<ftxui::Element> map_content;
        for (const auto& line : current_map_lines_) {
            // 检查是否为地图网格区域（包含边框字符的行，但不是图例或标题）
            bool isMapGrid = (line.find("+") != std::string::npos || line.find("-") != std::string::npos || 
                             line.find("|") != std::string::npos) && 
                             line.find("=== 地图图例 ===") == std::string::npos && 
                             line.find("P =") == std::string::npos && line.find("I =") == std::string::npos && 
                             line.find("N =") == std::string::npos && line.find("S =") == std::string::npos && 
                             line.find("M =") == std::string::npos && line.find("^v>< =") == std::string::npos && 
                             line.find("# =") == std::string::npos && line.find(". =") == std::string::npos &&
                             line.length() > 5; // 确保不是简短的标题行，但允许区块名称显示
            
            if (isMapGrid) {
                // 地图网格区域 - 为每个字符位置单独应用样式
                std::vector<ftxui::Element> styled_chars;
                for (size_t i = 0; i < line.length(); ++i) {
                    char c = line[i];
                    ftxui::Element char_element = ftxui::text(std::string(1, c));
                    
                    // 根据字符类型应用不同的样式，确保良好的对比度和可读性
                    if (c == '+' || c == '-' || c == '|') {
                        // 边框字符 - 青色
                        char_element = char_element | ftxui::color(ftxui::Color::Cyan) | ftxui::bold;
                    } else if (c == 'P') {
                        // 玩家位置 - 白色文字配深蓝色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Blue) | ftxui::bold;
                    } else if (c == 'I') {
                        // 物品 - 白色文字配深绿色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Green) | ftxui::bold;
                    } else if (c == 'N') {
                        // NPC - 白色文字配深青色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Cyan) | ftxui::bold;
                    } else if (c == 'S') {
                        // 神像 - 黑色文字配黄色背景
                        char_element = char_element | ftxui::color(ftxui::Color::Black) | ftxui::bgcolor(ftxui::Color::Yellow) | ftxui::bold;
                    } else if (c == 'M') {
                        // 怪物 - 白色文字配红色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Red) | ftxui::bold;
                    } else if (c == '^' || c == 'v' || c == '>' || c == '<') {
                        // 出口 - 白色文字配紫色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Magenta) | ftxui::bold;
                    } else if (c == '#') {
                        // 墙壁 - 灰色
                        char_element = char_element | ftxui::color(ftxui::Color::GrayLight) | ftxui::bold;
                    } else if (c == '.') {
                        // 空地 - 白色
                        char_element = char_element | ftxui::color(ftxui::Color::White);
                    } else {
                        // 其他字符 - 青色
                        char_element = char_element | ftxui::color(ftxui::Color::Cyan) | ftxui::bold;
                    }
                    
                    styled_chars.push_back(char_element);
                }
                
                // 将样式化的字符组合成一行
                auto styled_line = ftxui::hbox(styled_chars);
                map_content.push_back(styled_line);
            } else if (line.find("=== 地图图例 ===") != std::string::npos) {
                // 图例标题 - 使用黄色
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::Yellow) | ftxui::bold;
                map_content.push_back(styled_line);
            } else if (line.find("P =") != std::string::npos || line.find("I =") != std::string::npos || 
                      line.find("N =") != std::string::npos || line.find("S =") != std::string::npos || 
                      line.find("M =") != std::string::npos || line.find("^v>< =") != std::string::npos || 
                      line.find("# =") != std::string::npos || line.find(". =") != std::string::npos) {
                // 图例说明 - 使用普通颜色，不高亮
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::GrayLight);
                map_content.push_back(styled_line);
            } else {
                // 其他内容 - 使用绿色
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::Green);
                map_content.push_back(styled_line);
            }
        }
        
        // 将地图内容放在一个带边框的容器中
        auto map_box = ftxui::vbox(map_content) | ftxui::border | ftxui::color(ftxui::Color::Green);
        left.push_back(map_box);
        
        // 区块信息 - 使用更美观的样式
        if (!current_block_info_.empty()) {
            left.push_back(ftxui::separator());
            left.push_back(ftxui::text("[位置] 位置信息") | ftxui::bold | ftxui::color(ftxui::Color::Yellow));
            left.push_back(ftxui::paragraph(current_block_info_) | ftxui::color(ftxui::Color::White));
        }
        
        auto left_box = ftxui::vbox(left) | ftxui::border | ftxui::color(ftxui::Color::Blue);

        // 右侧：游戏消息（固定高度视口） + 游戏状态（玩家/队伍） - 优化美观度
        std::vector<ftxui::Element> right;
        
        // 游戏消息区域
        right.push_back(ftxui::text("[消息] 游戏消息") | ftxui::bold | ftxui::color(ftxui::Color::Magenta));
        right.push_back(ftxui::separator());
        std::vector<ftxui::Element> msg_lines;
        if (game_messages_.empty()) {
            msg_lines.push_back(ftxui::text("暂无消息") | ftxui::color(ftxui::Color::GrayLight));
        } else {
            // 为了保证新增消息在存在多行换行时也能立即可见，按时间倒序显示（最新在上）
            for (auto it = game_messages_.rbegin(); it != game_messages_.rend(); ++it) {
                const auto& m = *it;
                // 为不同类型的消息添加不同颜色
                auto styled_msg = ftxui::paragraph(m);
                if (m.find("胜利") != std::string::npos || m.find("成功") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Green);
                } else if (m.find("失败") != std::string::npos || m.find("错误") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Red);
                } else if (m.find("获得") != std::string::npos || m.find("奖励") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Yellow);
                } else {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::White);
                }
                msg_lines.push_back(styled_msg);
            }
        }
        auto messages_view = ftxui::vbox(msg_lines) | ftxui::vscroll_indicator | ftxui::yframe |
                             ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, message_h);
        right.push_back(messages_view);
        
        // 游戏状态区域
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("[状态] 游戏状态") | ftxui::bold | ftxui::color(ftxui::Color::Cyan));
        right.push_back(ftxui::separator());
        
        // 玩家信息 - 使用更美观的显示
        auto activeMember = game_->getPlayer().getActiveMember();
        std::string activeName = activeMember ? activeMember->getName() : "无";
        right.push_back(ftxui::paragraph("上场角色: " + activeName) | ftxui::color(ftxui::Color::White));
        right.push_back(ftxui::paragraph("等级: Lv." + std::to_string(player_level_)) | ftxui::color(ftxui::Color::Yellow));
        
        // HP显示 - 使用颜色表示血量状态
        auto hp_color = (player_hp_ > player_max_hp_ * 0.5) ? ftxui::Color::Green : 
                       (player_hp_ > player_max_hp_ * 0.25) ? ftxui::Color::Yellow : ftxui::Color::Red;
        right.push_back(ftxui::paragraph("HP: " + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_)) | ftxui::color(hp_color));
        right.push_back(ftxui::paragraph(player_status_) | ftxui::color(ftxui::Color::GrayLight));
        
        // 队伍成员区域
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("[队伍] 队伍成员") | ftxui::bold | ftxui::color(ftxui::Color::Blue));
        right.push_back(ftxui::separator());
        if (team_members_.empty()) {
            right.push_back(ftxui::text("暂无队伍成员") | ftxui::color(ftxui::Color::GrayLight));
        } else {
            for (const auto& member : team_members_) {
                // 为队伍成员添加颜色
                right.push_back(ftxui::paragraph("* " + member) | ftxui::color(ftxui::Color::White));
            }
        }
        
        auto right_box = ftxui::vbox(right) | ftxui::border | ftxui::color(ftxui::Color::Magenta);

        // 按 7:3 比例分配宽度
        int left_w = std::max(20, dims.dimx * 5 / 10);
        int right_w = std::max(10, dims.dimx - left_w);
        auto left_sized = left_box | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, left_w);
        auto right_sized = right_box | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, right_w);

        return ftxui::hbox({ left_sized, right_sized }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, main_h);
    });

    // 底部：游戏操作选项（按钮 + 提示）
    auto bottom_container = ftxui::Container::Horizontal(bottom_action_buttons_);
    auto bottom_renderer = ftxui::Renderer(bottom_container, [this] {
        int bottom_h = 6; // 底栏约 10%
        std::vector<ftxui::Element> btns;
        for (auto& b : bottom_action_buttons_) {
            btns.push_back(b->Render());
            btns.push_back(ftxui::text(" "));
        }
        
        // 操作提示 - 使用更美观的样式
        auto help = ftxui::text("W/A/S/D 移动  空格 交互  Q/E 切换队友  Esc 退出") |
                   ftxui::color(ftxui::Color::GrayLight);
        
        auto bar = ftxui::hbox({ ftxui::hbox(btns), ftxui::filler(), help }) | 
                  ftxui::border | ftxui::color(ftxui::Color::Green);
        return bar | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, bottom_h);
    });

    // 主组件（顶栏 / 中部 / 底部）
    component_ = ftxui::Container::Vertical({ header_renderer, main_renderer, bottom_renderer });

    // 移除工具菜单叠加层和相关事件处理
    // component_ |= ftxui::CatchEvent([this](ftxui::Event event) { ... });
    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        // 仅保留游戏控制快捷键
        if (event == ftxui::Event::Character('w') || event == ftxui::Event::ArrowUp) {
            HandleGameCommand("move north");
            return true;
        } else if (event == ftxui::Event::Character('s') || event == ftxui::Event::ArrowDown) {
            HandleGameCommand("move south");
            return true;
        } else if (event == ftxui::Event::Character('a') || event == ftxui::Event::ArrowLeft) {
            HandleGameCommand("move west");
            return true;
        } else if (event == ftxui::Event::Character('d') || event == ftxui::Event::ArrowRight) {
            HandleGameCommand("move east");
            return true;
        } else if (event == ftxui::Event::Character(' ')) {
            HandleGameCommand("interact");
            return true;
        } else if (event == ftxui::Event::Character('q') || event == ftxui::Event::Character('Q')) {
            HandleGameCommand("switch_next_member");
            return true;
        } else if (event == ftxui::Event::Character('e') || event == ftxui::Event::Character('E')) {
            HandleGameCommand("switch_prev_member");
            return true;
        } else if (event == ftxui::Event::Escape) {
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::QUIT_GAME));
            }
            return true;
        }
        return false;
    });
}

// 返回该界面的根组件供外部挂载
ftxui::Component GameplayScreen::GetComponent() {
    return component_;
}

// 同步玩家基础信息（等级/经验/当前上场角色 HP）
void GameplayScreen::UpdatePlayerInfo(const Player& player) {
    player_name_ = player.name;
    auto active = player.getActiveMember();
    player_hp_ = active ? active->getCurrentHealth() : 0;
    player_max_hp_ = active ? active->getTotalHealth() : 0;
    player_level_ = player.level;
    player_experience_ = player.experience;
    
    std::stringstream ss;
    ss << "等级: " << player.level << " 经验: " << player.experience;
    player_status_ = ss.str();
}

// 追加一条聊天消息（保留最多 20 条）
void GameplayScreen::AddChatMessage(const std::string& message) {
    chat_messages_.push_back(message);
    // 限制消息数量
    if (chat_messages_.size() > 20) {
        chat_messages_.erase(chat_messages_.begin());
    }
}

// 追加一条系统/游戏状态消息（保留最多 20 条）
void GameplayScreen::UpdateGameStatus(const std::string& status) {
    game_messages_.push_back(status);
    // 限制消息数量
    if (game_messages_.size() > 20) {
        game_messages_.erase(game_messages_.begin());
    }
}

// 替换队伍成员显示数据
void GameplayScreen::UpdateTeamStatus(const std::vector<std::string>& teamMembers) {
    team_members_ = teamMembers;
}

// 重新生成队伍成员信息（含 HP）并刷新右侧显示
void GameplayScreen::RefreshTeamDisplay() {
    if (!game_) return;
    
    std::vector<std::string> teamMemberNames;
    const auto& player = game_->getPlayer();
    for (const auto& member : player.teamMembers) {
        if (member) {
            std::string memberInfo = member->getName() + " (HP: " + 
                                   std::to_string(member->getCurrentHealth()) + "/" + 
                                   std::to_string(member->getTotalHealth()) + ")";
            teamMemberNames.push_back(memberInfo);
        }
    }
    UpdateTeamStatus(teamMemberNames);
}

// 清空所有消息（系统与聊天）
void GameplayScreen::ClearAllMessages() {
    game_messages_.clear();
    chat_messages_.clear();
    // 重置完成提示标记，便于新一轮游玩再次提示
    completion_announced_ = false;
}

// 刷新地图显示与当前位置说明，必要时触发首次到访剧情提示
void GameplayScreen::UpdateMapDisplay() {
    if (!game_) {
        current_map_lines_ = {"Map not available"};
        current_block_info_ = "Game not initialized";
        return;
    }
    
    try {
        // 获取当前区块地图
        const auto& mapManager = game_->getMapManager();
        current_map_lines_ = mapManager.renderCurrentBlock();
        
        // 获取当前位置信息
        current_block_info_ = mapManager.getCurrentCellInfo();
        
        // 检查是否地图（本章）探索完成，若首次达成则提示
        if (!completion_announced_ && mapManager.isMapCompleted()) {
            UpdateGameStatus(" ");
            UpdateGameStatus("===============================");
            UpdateGameStatus("恭喜你已经完成本章探索，请期待软件更新");
            UpdateGameStatus("===============================");
            completion_announced_ = true;
        }
        // 进入区块时尝试触发剧情
        auto currentBlock = mapManager.getCurrentBlock();
        if (currentBlock) {
            MaybeShowBlockStory(currentBlock->getName());
        }
        
    } catch (const std::exception& e) {
        current_map_lines_ = {"Map error"};
        current_block_info_ = "Error: " + std::string(e.what());
    }
}


// 处理核心游戏指令：移动/切换队友/交互等
void GameplayScreen::HandleGameCommand(const std::string& command) {
    if (!game_) {
        UpdateGameStatus("游戏对象未初始化");
        return;
    }
    
    if (command == "move north") {
        if (game_->movePlayer(0, -1)) {
            UpdatePlayerInfo(game_->getPlayer());
            UpdateMapDisplay();  // 实时更新地图
        } else {
            UpdateGameStatus("无法向北移动");
        }
    } else if (command == "move south") {
        if (game_->movePlayer(0, 1)) {
            UpdatePlayerInfo(game_->getPlayer());
            UpdateMapDisplay();  // 实时更新地图
        } else {
            UpdateGameStatus("无法向南移动");
        }
    } else if (command == "move west") {
        if (game_->movePlayer(-1, 0)) {
            UpdatePlayerInfo(game_->getPlayer());
            UpdateMapDisplay();  // 实时更新地图
        } else {
            UpdateGameStatus("无法向西移动");
        }
    } else if (command == "move east") {
        if (game_->movePlayer(1, 0)) {
            UpdatePlayerInfo(game_->getPlayer());
            UpdateMapDisplay();  // 实时更新地图
        } else {
            UpdateGameStatus("无法向东移动");
        }
    } else if (command == "switch_next_member") {
        auto& player = game_->getPlayer();
        if (player.switchToNextActiveMember()) {
            UpdatePlayerInfo(player);
            auto activeMember = player.getActiveMember();
            if (activeMember) {
                UpdateGameStatus("切换到队友: " + activeMember->getName());
            }
            
            // 更新队伍状态显示
            RefreshTeamDisplay();
        } else {
            UpdateGameStatus("无法切换队友");
        }
    } else if (command == "switch_prev_member") {
        auto& player = game_->getPlayer();
        if (player.switchToPreviousActiveMember()) {
            UpdatePlayerInfo(player);
            auto activeMember = player.getActiveMember();
            if (activeMember) {
                UpdateGameStatus("切换到队友: " + activeMember->getName());
            }
            
            // 更新队伍状态显示
            RefreshTeamDisplay();
        } else {
            UpdateGameStatus("无法切换队友");
        }
    } else if (command == "interact") {
        auto interactions = game_->getAvailableMapInteractions();
        if (!interactions.empty()) {
            // 尝试第一个可用的交互
            auto result = game_->interactWithMap(interactions[0]);
            if (result.message == "OPEN_SHOP") {
                if (navigation_callback_) {
                    navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Shop"));
                }
                return; // 切换界面
            }
            UpdateGameStatus(result.message);
            if (result.success) {
                UpdatePlayerInfo(game_->getPlayer());
                UpdateMapDisplay();  // 交互后更新地图
                
                // 更新队伍状态显示（重要：处理新角色加入的情况）
                RefreshTeamDisplay();
            }
        } else {
            UpdateGameStatus("当前位置没有可交互的内容");
        }
    } else {
        UpdateGameStatus("执行命令: " + command);
    }
}

// 首次进入区块时显示简要剧情/提示
void GameplayScreen::MaybeShowBlockStory(const std::string& block_name) {
    if (block_name.empty()) return;
    if (visited_blocks_.count(block_name)) return;
    visited_blocks_.insert(block_name);

    // 根据区块名称推送首访剧情简介（改编为当前MUD设计的提示风格）
    if (block_name.find("新手教学区") != std::string::npos) {
        UpdateGameStatus(" ");
        UpdateGameStatus("提示：W/A/S/D 移动，空格交互");
        UpdateGameStatus("你与同伴从坠星山谷出发，沿途探索前行。");
        UpdateGameStatus("[剧情·启程·坠星山谷]");
        UpdateGameStatus(" ");
    } else if (block_name.find("安柏的营地") != std::string::npos) {
        UpdateGameStatus(" ");
        UpdateGameStatus("少女名为安柏，身为侦察骑士的她提出护送你们前往蒙德");
        UpdateGameStatus("一位轻盈的少女突然出现，挡住了你的去路。");
        UpdateGameStatus("[剧情·星落湖]");
        UpdateGameStatus(" ");
    } else if (block_name.find("史莱姆栖息地") != std::string::npos) {
        UpdateGameStatus(" ");
        UpdateGameStatus("提示：到达魔物面前，使用空格键击败它");
        UpdateGameStatus("林间回荡呢喃，你短暂目击神秘身影与巨兽的交流。");
        UpdateGameStatus("[剧情·林间回响]");
        UpdateGameStatus(" ");
    } else if (block_name.find("蒙德城") != std::string::npos) {
        UpdateGameStatus(" ");
        UpdateGameStatus("提示：在设置/背包/队伍界面完善配置后再继续主线");
        UpdateGameStatus("你抵达城邦入口，冒险服务与补给在城内更完善。");
        UpdateGameStatus("[剧情·自由之都·城门]");
        UpdateGameStatus(" ");
    } else if (block_name.find("千风神殿") != std::string::npos) {
        UpdateGameStatus(" ");
        UpdateGameStatus("提示：观察环境要素，借助交互点推进");
        UpdateGameStatus("遗迹深处存在风之结晶的残留，需破坏以削弱异动。");
        UpdateGameStatus("[剧情·遗祠调查·千风神殿]");
        UpdateGameStatus(" ");
    } else if (block_name.find("风啸山坡") != std::string::npos) {
        UpdateGameStatus(" ");
        UpdateGameStatus("提示：元素克制与队伍切换能更高效地推进");
        UpdateGameStatus("与学识之士协作，确认雷元素设施并清理结晶源。");
        UpdateGameStatus("[剧情·遗祠调查·风啸山坡]");
        UpdateGameStatus(" ");
    } else if (block_name.find("七天神像") != std::string::npos) {
        UpdateGameStatus(" ");
        UpdateGameStatus("提示：在神像附近多尝试交互，留意系统反馈");
        UpdateGameStatus("你感到体内与风产生呼应。或许应沿大道前往城邦打听情报。");
        UpdateGameStatus("[剧情·神像回应]");
        UpdateGameStatus(" ");
    }
}

// 重新构建底部按钮列表（用于设置更改后刷新）
void GameplayScreen::RebuildBottomButtons() {
    // 清空现有按钮
    bottom_action_buttons_.clear();
    
    // 重新创建所有按钮
    bottom_action_buttons_.push_back(ftxui::Button("交互", [this] { HandleGameCommand("interact"); }));
    bottom_action_buttons_.push_back(ftxui::Button("背包", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Inventory"));
    }));
    bottom_action_buttons_.push_back(ftxui::Button("队伍", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Team"));
    }));
    bottom_action_buttons_.push_back(ftxui::Button("地图", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Map"));
    }));
    bottom_action_buttons_.push_back(ftxui::Button("设置", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "Settings"));
    }));
    
    // 根据当前AI设置状态决定是否添加智能建议按钮
    if (GlobalSettings::IsAIEnabled()) {
        bottom_action_buttons_.push_back(ftxui::Button("智能建议", [this] {
            // 采集上下文并调用LLM（避免阻塞UI，使用后台线程，完成后刷新消息）
            UpdateGameStatus("[AI] 正在分析…");
            auto context_builder = [this]() {
                std::stringstream ss;
                ss << "玩家: " << player_name_ << "\n";
                ss << "等级:" << player_level_ << ", HP:" << player_hp_ << "/" << player_max_hp_ << "\n";
                ss << "位置: " << current_block_info_ << "\n";
                ss << "近期消息:\n";
                int cnt = 0;
                for (auto it = game_messages_.rbegin(); it != game_messages_.rend() && cnt < 6; ++it, ++cnt) {
                    ss << "- " << *it << "\n";
                }
                ss << "请给出下一步操作建议。";
                return ss.str();
            };
            std::thread([this, context_builder]() {
                auto suggestion = RequestOpenAISuggestion(context_builder());
                last_llm_suggestion_ = suggestion;
                UpdateGameStatus(std::string("[AI建议] ") + suggestion);
            }).detach();
        }));
    }
    
    bottom_action_buttons_.push_back(ftxui::Button("保存", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SAVE_GAME));
    }));
    bottom_action_buttons_.push_back(ftxui::Button("退出", [this] {
        if (navigation_callback_) navigation_callback_(NavigationRequest(NavigationAction::SWITCH_SCREEN, "MainMenu"));
    }));
    
    // 重新创建底部容器和渲染器
    auto bottom_container = ftxui::Container::Horizontal(bottom_action_buttons_);
    auto bottom_renderer = ftxui::Renderer(bottom_container, [this] {
        int bottom_h = 6; // 底栏约 10%
        std::vector<ftxui::Element> btns;
        for (auto& b : bottom_action_buttons_) {
            btns.push_back(b->Render());
            btns.push_back(ftxui::text(" "));
        }
        
        // 操作提示 - 使用更美观的样式
        auto help = ftxui::text("W/A/S/D 移动  空格 交互  Q/E 切换队友  Esc 退出") |
                   ftxui::color(ftxui::Color::GrayLight);
        
        auto bar = ftxui::hbox({ ftxui::hbox(btns), ftxui::filler(), help }) | 
                  ftxui::border | ftxui::color(ftxui::Color::Green);
        return bar | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, bottom_h);
    });
    
    // 重新创建主组件
    auto header_renderer = ftxui::Renderer([this] {
        int header_h = 3;
        std::string right_status = "Lv." + std::to_string(player_level_) +
                                   "  HP:" + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_);
        auto center_title = ftxui::text("<<原神>>MUD版") | ftxui::bold | ftxui::color(ftxui::Color::Yellow);
        auto right = ftxui::text(right_status) | ftxui::color(ftxui::Color::Cyan);
        auto bar = ftxui::hbox({
                   ftxui::filler(),
                   center_title,
                   ftxui::filler(),
                   right,
               }) | ftxui::border | ftxui::color(ftxui::Color::Blue);
        return bar | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, header_h);
    });

    // 中部：左"游戏消息"/右"游戏状态（详细）"
    auto main_renderer = ftxui::Renderer([this] {
        auto dims = ftxui::Terminal::Size();
        int header_h = 3;
        int bottom_h = 3;
        int main_h = std::max(5, dims.dimy - header_h - bottom_h);
        int message_h = std::max(5, main_h * 6 / 10 - 1); // 为消息预留约 60% 的高度，减1行给队伍区域

        // 左侧：地图显示 - 优化美观度
        std::vector<ftxui::Element> left;
        
        // 地图标题 - 显示当前区块名称
        std::string currentBlockName = "未知区域";
        auto currentBlock = game_->getMapManager().getCurrentBlock();
        if (currentBlock) {
            currentBlockName = currentBlock->getName();
            MaybeShowBlockStory(currentBlockName);
        }
        left.push_back(ftxui::text("[地图] " + currentBlockName) | ftxui::bold | ftxui::color(ftxui::Color::Cyan));
        left.push_back(ftxui::separator());
        
        // 地图内容 - 使用等宽字体和颜色，为交互元素添加精确的单位置高亮
        std::vector<ftxui::Element> map_content;
        for (const auto& line : current_map_lines_) {
            // 检查是否为地图网格区域（包含边框字符的行，但不是图例或标题）
            bool isMapGrid = (line.find("+") != std::string::npos || line.find("-") != std::string::npos || 
                             line.find("|") != std::string::npos) && 
                             line.find("=== 地图图例 ===") == std::string::npos && 
                             line.find("P =") == std::string::npos && line.find("I =") == std::string::npos && 
                             line.find("N =") == std::string::npos && line.find("S =") == std::string::npos && 
                             line.find("M =") == std::string::npos && line.find("^v>< =") == std::string::npos && 
                             line.find("# =") == std::string::npos && line.find(". =") == std::string::npos &&
                             line.length() > 5; // 确保不是简短的标题行，但允许区块名称显示
            
            if (isMapGrid) {
                // 地图网格区域 - 为每个字符位置单独应用样式
                std::vector<ftxui::Element> styled_chars;
                for (size_t i = 0; i < line.length(); ++i) {
                    char c = line[i];
                    ftxui::Element char_element = ftxui::text(std::string(1, c));
                    
                    // 根据字符类型应用不同的样式，确保良好的对比度和可读性
                    if (c == '+' || c == '-' || c == '|') {
                        // 边框字符 - 青色
                        char_element = char_element | ftxui::color(ftxui::Color::Cyan) | ftxui::bold;
                    } else if (c == 'P') {
                        // 玩家位置 - 白色文字配深蓝色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Blue) | ftxui::bold;
                    } else if (c == 'I') {
                        // 物品 - 白色文字配深绿色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Green) | ftxui::bold;
                    } else if (c == 'N') {
                        // NPC - 白色文字配深青色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Cyan) | ftxui::bold;
                    } else if (c == 'S') {
                        // 神像 - 黑色文字配黄色背景
                        char_element = char_element | ftxui::color(ftxui::Color::Black) | ftxui::bgcolor(ftxui::Color::Yellow) | ftxui::bold;
                    } else if (c == 'M') {
                        // 怪物 - 白色文字配红色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Red) | ftxui::bold;
                    } else if (c == '^' || c == 'v' || c == '>' || c == '<') {
                        // 出口 - 白色文字配紫色背景
                        char_element = char_element | ftxui::color(ftxui::Color::White) | ftxui::bgcolor(ftxui::Color::Magenta) | ftxui::bold;
                    } else if (c == '#') {
                        // 墙壁 - 灰色
                        char_element = char_element | ftxui::color(ftxui::Color::GrayLight) | ftxui::bold;
                    } else if (c == '.') {
                        // 空地 - 白色
                        char_element = char_element | ftxui::color(ftxui::Color::White);
                    } else {
                        // 其他字符 - 青色
                        char_element = char_element | ftxui::color(ftxui::Color::Cyan) | ftxui::bold;
                    }
                    
                    styled_chars.push_back(char_element);
                }
                
                // 将样式化的字符组合成一行
                auto styled_line = ftxui::hbox(styled_chars);
                map_content.push_back(styled_line);
            } else if (line.find("=== 地图图例 ===") != std::string::npos) {
                // 图例标题 - 使用黄色
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::Yellow) | ftxui::bold;
                map_content.push_back(styled_line);
            } else if (line.find("P =") != std::string::npos || line.find("I =") != std::string::npos || 
                      line.find("N =") != std::string::npos || line.find("S =") != std::string::npos || 
                      line.find("M =") != std::string::npos || line.find("^v>< =") != std::string::npos || 
                      line.find("# =") != std::string::npos || line.find(". =") != std::string::npos) {
                // 图例说明 - 使用普通颜色，不高亮
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::GrayLight);
                map_content.push_back(styled_line);
            } else {
                // 其他内容 - 使用绿色
                auto styled_line = ftxui::text(line) | ftxui::color(ftxui::Color::Green);
                map_content.push_back(styled_line);
            }
        }
        
        // 将地图内容放在一个带边框的容器中
        auto map_box = ftxui::vbox(map_content) | ftxui::border | ftxui::color(ftxui::Color::Green);
        left.push_back(map_box);
        
        // 区块信息 - 使用更美观的样式
        if (!current_block_info_.empty()) {
            left.push_back(ftxui::separator());
            left.push_back(ftxui::text("[位置] 位置信息") | ftxui::bold | ftxui::color(ftxui::Color::Yellow));
            left.push_back(ftxui::paragraph(current_block_info_) | ftxui::color(ftxui::Color::White));
        }
        
        auto left_box = ftxui::vbox(left) | ftxui::border | ftxui::color(ftxui::Color::Blue);

        // 右侧：游戏消息（固定高度视口） + 游戏状态（玩家/队伍） - 优化美观度
        std::vector<ftxui::Element> right;
        
        // 游戏消息区域
        right.push_back(ftxui::text("[消息] 游戏消息") | ftxui::bold | ftxui::color(ftxui::Color::Magenta));
        right.push_back(ftxui::separator());
        std::vector<ftxui::Element> msg_lines;
        if (game_messages_.empty()) {
            msg_lines.push_back(ftxui::text("暂无消息") | ftxui::color(ftxui::Color::GrayLight));
        } else {
            // 为了保证新增消息在存在多行换行时也能立即可见，按时间倒序显示（最新在上）
            for (auto it = game_messages_.rbegin(); it != game_messages_.rend(); ++it) {
                const auto& m = *it;
                // 为不同类型的消息添加不同颜色
                auto styled_msg = ftxui::paragraph(m);
                if (m.find("胜利") != std::string::npos || m.find("成功") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Green);
                } else if (m.find("失败") != std::string::npos || m.find("错误") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Red);
                } else if (m.find("获得") != std::string::npos || m.find("奖励") != std::string::npos) {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::Yellow);
                } else {
                    styled_msg = styled_msg | ftxui::color(ftxui::Color::White);
                }
                msg_lines.push_back(styled_msg);
            }
        }
        auto messages_view = ftxui::vbox(msg_lines) | ftxui::vscroll_indicator | ftxui::yframe |
                             ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, message_h);
        right.push_back(messages_view);
        
        // 游戏状态区域
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("[状态] 游戏状态") | ftxui::bold | ftxui::color(ftxui::Color::Cyan));
        right.push_back(ftxui::separator());
        
        // 玩家信息 - 使用更美观的显示
        auto activeMember = game_->getPlayer().getActiveMember();
        std::string activeName = activeMember ? activeMember->getName() : "无";
        right.push_back(ftxui::paragraph("上场角色: " + activeName) | ftxui::color(ftxui::Color::White));
        right.push_back(ftxui::paragraph("等级: Lv." + std::to_string(player_level_)) | ftxui::color(ftxui::Color::Yellow));
        
        // HP显示 - 使用颜色表示血量状态
        auto hp_color = (player_hp_ > player_max_hp_ * 0.5) ? ftxui::Color::Green : 
                       (player_hp_ > player_max_hp_ * 0.25) ? ftxui::Color::Yellow : ftxui::Color::Red;
        right.push_back(ftxui::paragraph("HP: " + std::to_string(player_hp_) + "/" + std::to_string(player_max_hp_)) | ftxui::color(hp_color));
        right.push_back(ftxui::paragraph(player_status_) | ftxui::color(ftxui::Color::GrayLight));
        
        // 队伍成员区域
        right.push_back(ftxui::separator());
        right.push_back(ftxui::text("[队伍] 队伍成员") | ftxui::bold | ftxui::color(ftxui::Color::Blue));
        right.push_back(ftxui::separator());
        if (team_members_.empty()) {
            right.push_back(ftxui::text("暂无队伍成员") | ftxui::color(ftxui::Color::GrayLight));
        } else {
            for (const auto& member : team_members_) {
                // 为队伍成员添加颜色
                right.push_back(ftxui::paragraph("* " + member) | ftxui::color(ftxui::Color::White));
            }
        }
        
        auto right_box = ftxui::vbox(right) | ftxui::border | ftxui::color(ftxui::Color::Magenta);

        // 按 7:3 比例分配宽度
        int left_w = std::max(20, dims.dimx * 5 / 10);
        int right_w = std::max(10, dims.dimx - left_w);
        auto left_sized = left_box | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, left_w);
        auto right_sized = right_box | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, right_w);

        return ftxui::hbox({ left_sized, right_sized }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, main_h);
    });

    // 主组件（顶栏 / 中部 / 底部）
    component_ = ftxui::Container::Vertical({ header_renderer, main_renderer, bottom_renderer });

    // 移除工具菜单叠加层和相关事件处理
    // component_ |= ftxui::CatchEvent([this](ftxui::Event event) { ... });
    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        // 仅保留游戏控制快捷键
        if (event == ftxui::Event::Character('w') || event == ftxui::Event::ArrowUp) {
            HandleGameCommand("move north");
            return true;
        } else if (event == ftxui::Event::Character('s') || event == ftxui::Event::ArrowDown) {
            HandleGameCommand("move south");
            return true;
        } else if (event == ftxui::Event::Character('a') || event == ftxui::Event::ArrowLeft) {
            HandleGameCommand("move west");
            return true;
        } else if (event == ftxui::Event::Character('d') || event == ftxui::Event::ArrowRight) {
            HandleGameCommand("move east");
            return true;
        } else if (event == ftxui::Event::Character(' ')) {
            HandleGameCommand("interact");
            return true;
        } else if (event == ftxui::Event::Character('q') || event == ftxui::Event::Character('Q')) {
            HandleGameCommand("switch_next_member");
            return true;
        } else if (event == ftxui::Event::Character('e') || event == ftxui::Event::Character('E')) {
            HandleGameCommand("switch_prev_member");
            return true;
        } else if (event == ftxui::Event::Escape) {
            if (navigation_callback_) {
                navigation_callback_(NavigationRequest(NavigationAction::QUIT_GAME));
            }
            return true;
        }
        return false;
    });
}