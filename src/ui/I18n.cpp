#include "ui/I18n.hpp"

#include <utility>

namespace arcane::ui {

I18n& I18n::instance()
{
    static I18n inst;
    return inst;
}

I18n::I18n()
{
    seed();
}

void I18n::setLanguage(Lang lang)
{
    if (m_lang_ == lang) {
        return;
    }
    m_lang_ = lang;
    emit languageChanged(lang);
}

QString I18n::tr(const QString& key) const
{
    const auto it = map_.constFind(key);
    if (it == map_.constEnd()) {
        return key;  // 找不到时返回 key 本身，便于排查。
    }
    return m_lang_ == Lang::Chinese ? it->first : it->second;
}

QString I18n::tr(const QString& key, const QStringList& args) const
{
    QString template_ = tr(key);
    for (int i = 0; i < args.size(); ++i) {
        template_ = template_.arg(args.at(i));
    }
    return template_;
}

void I18n::seedPair(const QString& key, const QString& zh, const QString& en)
{
    map_.insert(key, qMakePair(zh, en));
}

void I18n::seed()
{
    using K = const char*;
    // ---------------- 快捷动作按钮 ----------------
    seedPair("action.talk",          "交谈",        "Talk");
    seedPair("action.study",         "学习",        "Study");
    seedPair("action.rest",          "休息",        "Rest");
    seedPair("action.marauders_map", "活点地图",    "Marauder's Map");
    seedPair("action.night_patrol",  "夜游",        "Night Patrol");

    // ---------------- 面板标题 ----------------
    seedPair("title.app",       "霍格沃兹在线",       "Hogwarts Online");
    seedPair("title.housecup",  "学院杯 ⏳",          "House Cup ⏳");
    seedPair("title.status",    "学生状态",           "Student Status");
    seedPair("title.chat",      "校园聊天与事件",     "Campus Chat & Events");
    seedPair("title.inventory", "背包",               "Inventory");
    seedPair("title.classes",   "今日课程",           "Today Classes");
    seedPair("title.members",   "在线校园",           "Online Campus");
    seedPair("title.location",  "当前位置",           "Current Location");
    seedPair("title.actions",   "快捷动作",           "Quick Actions");
    seedPair("title.move",      "前往",               "Move To");
    seedPair("title.here",      "此地角色",           "People Here");

    // ---------------- 菜单 ----------------
    seedPair("menu.character", "角色",          "Character");
    seedPair("menu.campus",    "校园",          "Campus");
    seedPair("menu.social",    "社交",          "Social");
    seedPair("menu.system",    "系统",          "System");
    seedPair("menu.story",     "剧情",          "Story");

    seedPair("menu.character.profile",       "个人资料",         "Profile");
    seedPair("menu.character.relationships", "人际关系",         "Relationships");
    seedPair("menu.campus.map",              "活点地图",         "Marauder's Map");
    seedPair("menu.campus.nightpatrol",      "夜游（独自）",     "Night Patrol (Solo)");
    seedPair("menu.campus.courses",          "课程",             "Courses");
    seedPair("menu.campus.activities",       "活动",             "Activities");
    seedPair("menu.social.friends",          "好友",             "Friends");
    seedPair("menu.social.owl",              "猫头鹰邮件",       "Owl Mail");
    seedPair("menu.system.refreshinv",       "刷新背包",         "Refresh Inventory");
    seedPair("menu.system.aisettings",       "AI 设置...",       "AI Settings...");
    seedPair("menu.system.logout",           "退出登录",         "Logout");
    seedPair("menu.system.lang",             "语言：中文",       "Language: English");

    // ---------------- 剧情选择 ----------------
    seedPair("story.viewrep",        "查看风评",             "View Reputation");
    seedPair("story.share",          "与赫敏分享笔记",       "Share notes with Hermione");
    seedPair("story.restricted",     "潜入禁书区",           "Sneak into the Restricted Section");
    seedPair("story.standup",        "为朋友出头",           "Stand up for a friend");
    seedPair("story.cheat",          "魔药考试作弊",         "Cheat on the Potions exam");
    seedPair("story.hagrid",         "帮海格照顾神奇动物",   "Help Hagrid with creatures");
    seedPair("story.duel",           "与马尔福决斗",         "Duel Draco Malfoy");
    seedPair("story.study",          "图书馆熬夜学习",       "Study late in the library");

    // ---------------- 登录页 ----------------
    seedPair("login.title",       "霍格沃兹在线",         "Hogwarts Online");
    seedPair("login.subtitle",    "秋学期 · 第三周 · 学生入口", "Autumn Term - Week Three - Student Entrance");
    seedPair("login.studentname", "学生姓名",             "Student Name");
    seedPair("login.house",       "学院",                 "House");
    seedPair("login.enter",       "进入校园",             "Enter Campus");
    seedPair("login.enroll",      "新生？前往注册",       "New Student? Enroll Here");
    seedPair("login.hint",        "今日：大礼堂早餐开放，首节课 09:00 开始。",
                                  "Today: Breakfast is open in the Great Hall. First class begins at 09:00.");

    // ---------------- 时钟 / 时段 ----------------
    seedPair("time.autumn",  "秋学期 · 第三周",     "Autumn Term - Week Three");
    seedPair("time.weather", "天气：晴",             "Weather: Sunny");
    seedPair("time.period",  "时段：{0}",           "Period: {0}");
    seedPair("time.weekday.monday",    "星期一",     "Monday");
    seedPair("time.weekday.tuesday",   "星期二",     "Tuesday");
    seedPair("time.weekday.wednesday", "星期三",     "Wednesday");
    seedPair("time.weekday.thursday",  "星期四",     "Thursday");
    seedPair("time.weekday.friday",    "星期五",     "Friday");
    seedPair("time.weekday.saturday",  "星期六",     "Saturday");
    seedPair("time.weekday.sunday",    "星期日",     "Sunday");

    // ---------------- 在线校园（成员列表） ----------------
    seedPair("members.players",    "其他学生",       "Players");
    seedPair("members.professors", "教授",           "Professors");
    seedPair("members.studentnpcs","学生 NPC",       "Student NPCs");
    seedPair("members.ghosts",     "幽灵",           "Ghosts");
    seedPair("members.shops",      "商店 NPC",       "Shop NPCs");
    seedPair("members.privatechat", "私聊",          "Private Chat");
    seedPair("members.profile",     "查看档案",      "View Profile");
    seedPair("members.col.name",    "姓名",          "Name");
    seedPair("members.col.place",   "地点",          "Place");
    seedPair("members.col.state",   "状态",          "State");
    seedPair("members.col.house",   "学院",          "House");

    // ---------------- NPC 名 / 状态 ----------------
    seedPair("npc.hermione",     "赫敏·格兰杰",     "Hermione Granger");
    seedPair("npc.gareth",       "加雷斯·韦斯莱",   "Gareth Weasley");
    seedPair("npc.luna",         "卢娜·亚斯特",     "Luna Aster");
    seedPair("npc.cedric",       "塞德里克·瓦尔",   "Cedric Vale");
    seedPair("npc.mcgonagall",   "麦格教授",        "Professor McGonagall");
    seedPair("npc.grey",         "灰女士",          "The Grey Lady");
    seedPair("npc.ollivander",   "魔杖店店员",      "Wand Shop Clerk");
    seedPair("npc.status.online",    "在线",        "Online");
    seedPair("npc.status.teaching",  "授课中",      "Teaching");
    seedPair("npc.status.reading",   "阅读中",      "Reading");
    seedPair("npc.status.chatting",  "聊天中",      "Chatting");
    seedPair("npc.status.wandering", "游荡中",      "Wandering");
    seedPair("npc.status.available", "可服务",      "Available");
    seedPair("npc.role.staff",       "教职工",      "Staff");
    seedPair("npc.role.none",        "无",          "None");

    // ---------------- 学院 ----------------
    seedPair("house.gryffindor", "格兰芬多",  "Gryffindor");
    seedPair("house.slytherin",  "斯莱特林",  "Slytherin");
    seedPair("house.ravenclaw",  "拉文克劳",  "Ravenclaw");
    seedPair("house.hufflepuff", "赫奇帕奇",  "Hufflepuff");

    // ---------------- 地点 ----------------
    seedPair("loc.greathall", "大礼堂",  "Great Hall");
    seedPair("loc.classroom", "教室",    "Classroom");
    seedPair("loc.library",   "图书馆",  "Library");
    seedPair("loc.courtyard", "庭院",    "Courtyard");
    seedPair("loc.hogsmeade", "霍格莫德","Hogsmeade");

    // ---------------- 课程 ----------------
    seedPair("class.transfiguration", "变形术",             "Transfiguration");
    seedPair("class.charms",          "魔咒课",             "Charms");
    seedPair("class.herbology",       "草药学",             "Herbology");
    seedPair("class.defense",         "黑魔法防御术",       "Defense Against the Dark Arts");
    seedPair("class.inprogress",      "进行中",             "In Progress");
    seedPair("class.notstarted",      "未开始",             "Not Started");
    seedPair("class.goto",            "前往教室",           "Go to Classroom");
    seedPair("class.details",         "详情",               "Details");
    seedPair("class.details2",        "课程详情",           "Course Details");
    seedPair("class.professor",       "教授信息",           "Professor Info");

    // ---------------- 背包物品 ----------------
    seedPair("item.wand",       "魔杖",           "Wand");
    seedPair("item.chocolate",  "巧克力蛙",       "Chocolate Frog");
    seedPair("item.butterbeer", "黄油啤酒",       "Butterbeer");
    seedPair("item.potion",     "药剂",           "Potion");
    seedPair("item.quill",      "羽毛笔",         "Quill");
    seedPair("item.galleon",    "加隆",           "Galleons");
    seedPair("item.inspect",    "检视",           "Inspect");
    seedPair("item.gift",       "赠予 NPC",       "Gift to NPC");

    // ---------------- 学生状态字段 ----------------
    seedPair("status.name",         "姓名",        "Name");
    seedPair("status.house",        "学院",        "House");
    seedPair("status.year",         "年级",        "Year");
    seedPair("status.location",     "地点",        "Location");
    seedPair("status.galleons",     "加隆",        "Galleons");
    seedPair("status.housepoints",  "学院分",      "House Points");
    seedPair("status.state",        "状态",        "State");
    seedPair("status.currentclass", "当前课程",    "Current Class");
    seedPair("status.breakfast",    "早餐",        "Breakfast");

    // ---------------- 聊天 ----------------
    seedPair("chat.placeholder",     "向当前校园频道留言...",  "Write to the current campus channel...");
    seedPair("chat.send",            "发送",                  "Send");
    seedPair("chat.channel.location","地点",                  "Location");
    seedPair("chat.channel.world",   "世界",                  "World");
    seedPair("chat.channel.party",   "组队",                  "Party");
    seedPair("chat.system",          "[系统]",                "[System]");
    seedPair("chat.feedback.disconnected", "已断开 · 网络层预留", "Disconnected · Network layer reserved");

    // ---------------- 聊天种子消息 ----------------
    seedPair("chat.seed.breakfast",
             "[系统]<br>早餐时间已开始。<br>大礼堂现已开放。",
             "[System]<br>Breakfast time has started.<br>Great Hall is now open.");
    seedPair("chat.seed.class",
             "麦格教授：变形课还有十五分钟开始。<br>地点：变形术教室。",
             "Professor McGonagall: Transfiguration class begins in fifteen minutes.<br>Location: Transfiguration Classroom.");
    seedPair("chat.seed.library",
             "[图书馆] 赫敏·格兰杰：<br>今天图书馆格外安静。",
             "[Library] Hermione Granger:<br>The library is unusually quiet today.");
    seedPair("chat.seed.enter_class",
             "{0} 加雷斯·韦斯莱 走进了变形术教室。",
             "{0} Gareth Weasley entered the Transfiguration Classroom.");
    seedPair("chat.seed.lesson_start",
             "{0} 麦格教授开始授课。",
             "{0} Professor McGonagall started the lesson.");

    // ---------------- 学院沙漏 / 学院排名 ----------------
    seedPair("ranking.recent",        "暂无最新变化",         "No recent point changes.");
    seedPair("ranking.delta.format",  "{0} {1}{2} 分 — {3}", "{0} {1}{2} pts — {3}");
    seedPair("ranking.rank.1",        "第 1 名",              "1st");
    seedPair("ranking.rank.2",        "第 2 名",              "2nd");
    seedPair("ranking.rank.3",        "第 3 名",              "3rd");
    seedPair("ranking.rank.4",        "第 4 名",              "4th");
    seedPair("ranking.reason.manual_add",      "手动加分",       "Manual house points award");
    seedPair("ranking.reason.manual_deduct",   "手动扣分",       "Manual house points deduction");
    seedPair("ranking.reason.curfew",          "宵禁违规 @ {0}", "Curfew violation at {0}");

    // ---------------- 富文本日志（控制器用） ----------------
    seedPair("log.housepoints.earns", "获得", "earns");
    seedPair("log.housepoints.loses", "失去", "loses");
    seedPair("log.narrative.label",   "📜 剧情选择", "📜 Choice");
    seedPair("log.narrative.consequence", "后果：", "Consequence: ");
    seedPair("log.reputation.label",  "📖 风评", "📖 Reputation");
    seedPair("log.reputation.peer",   "同学",   "Peers");
    seedPair("log.reputation.teacher","老师",   "Teachers");
    seedPair("log.reputation.house",  "室友",   "Housemates");
    seedPair("log.reputation.relationships", "关系：", "Relationships:");
}

} // namespace arcane::ui