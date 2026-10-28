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
        template_.replace(QStringLiteral("{%1}").arg(i), args.at(i));
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
    seedPair("title.chat",      "正在发生",           "The Living Story");
    seedPair("title.inventory", "背包",               "Inventory");
    seedPair("title.classes",   "今日课程",           "Today Classes");
    seedPair("title.members",   "在线校园",           "Online Campus");
    seedPair("title.location",  "当前位置",           "Current Location");
    seedPair("title.actions",   "快捷动作",           "Quick Actions");
    seedPair("title.move",      "前往",               "Move To");
    seedPair("title.here",      "此地角色",           "People Here");

    // ---------------- 世界主界面 / 辅助抽屉 ----------------
    seedPair("shell.world.title",       "魔法世界",                 "The Wizarding World");
    seedPair("shell.world.subtitle",    "你所看见、听见和选择的一切，都会留在这里。",
                                             "Everything you witness, say, and choose leaves a trace.");
    seedPair("drawer.scene.title",      "此刻与此地",               "Here & Now");
    seedPair("drawer.journal.title",    "随身册",                   "Field Journal");
    seedPair("drawer.scene.toggle",     "\u2630  场景",             "\u2630  Scene");
    seedPair("drawer.journal.toggle",   "随身册  \u2630",           "Journal  \u2630");
    seedPair("drawer.close",            "收起",                     "Close");
    seedPair("drawer.tab.status",       "状态",                     "Status");
    seedPair("drawer.tab.house",        "学院",                     "House");
    seedPair("drawer.tab.classes",      "课程",                     "Classes");
    seedPair("drawer.tab.inventory",    "背包",                     "Bag");

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
    seedPair("login.subtitle",    "新生入学 · 从对角巷开始", "First Year · Begin in Diagon Alley");
    seedPair("login.studentname", "已注册用户名",           "Existing Username");
    seedPair("login.house",       "学院",                 "House");
    seedPair("login.enter",       "继续已有角色",         "Continue Existing Character");
    seedPair("login.enroll",      "创建新生角色",         "Create a First-Year Character");
    seedPair("login.hint",        "新生从对角巷开始，进度会自动保存在本机；已有角色用用户名和学院继续。",
                                  "First years begin in Diagon Alley. Progress saves locally; returning players use their username and house.");
    seedPair("login.database_unavailable", "无法打开本地存档或已配置的数据库，新生剧情暂时无法启动。",
                                            "Could not open the local save or configured database. The first-year story cannot start.");
    seedPair("login.name_required", "请填写注册时的用户名；新玩家请点击“创建新生角色”。",
                                     "Enter your registered username, or create a first-year character.");

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
    seedPair("npc.rowan",        "罗温·维尔",       "Rowan Vale");
    seedPair("npc.bramble",      "布兰布尔先生",     "Mr. Bramble");
    seedPair("npc.deputyhead",   "副校长",           "Deputy Head");
    seedPair("npc.sortinghat",   "分院帽",           "Sorting Hat");
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
    seedPair("loc.diagonalley", "对角巷", "Diagon Alley");
    seedPair("loc.kingscross", "国王十字车站", "King's Cross Station");
    seedPair("loc.platform934", "九又四分之三站台", "Platform Nine and Three-Quarters");
    seedPair("loc.hogwartsexpress", "霍格沃茨特快", "Hogwarts Express");
    seedPair("loc.hogsmeadestation", "霍格莫德车站", "Hogsmeade Station");
    seedPair("loc.blacklakecrossing", "黑湖渡船", "Crossing the Black Lake");
    seedPair("loc.entrancehall", "礼堂前厅", "Entrance Hall");
    seedPair("loc.sortingstool", "分院帽前", "Before the Sorting Hat");
    seedPair("loc.housetables", "学院长桌", "House Tables");

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
    seedPair("status.year1",        "一年级",      "Year 1");
    seedPair("status.location",     "地点",        "Location");
    seedPair("status.galleons",     "加隆",        "Galleons");
    seedPair("status.housepoints",  "学院分",      "House Points");
    seedPair("status.state",        "状态",        "State");
    seedPair("status.currentclass", "当前课程",    "Current Class");
    seedPair("status.breakfast",    "早餐",        "Breakfast");
    seedPair("status.shopping",     "采购入学用品", "Shopping for school supplies");
    seedPair("status.shoppingcomplete", "新生用品已备齐", "First-year supplies complete");
    seedPair("status.findingplatform", "寻找隐藏站台", "Finding the hidden platform");
    seedPair("status.boardingtrain", "准备登上霍格沃茨特快", "Boarding the Hogwarts Express");
    seedPair("status.travelling", "乘车前往霍格沃茨", "Travelling to Hogwarts");
    seedPair("status.followingfirstyears", "跟随一年级新生", "Following the first-years");
    seedPair("status.journeycomplete", "已抵达霍格沃茨", "Journey to Hogwarts complete");
    seedPair("status.crossingblacklake", "乘船穿过黑湖", "Crossing the Black Lake");
    seedPair("status.waitingsorting", "等待分院", "Waiting for the Sorting");
    seedPair("status.sortingceremony", "参加分院仪式", "Attending the Sorting Ceremony");
    seedPair("status.joininghousetable", "走向学院长桌", "Joining the house table");
    seedPair("status.freeexploration", "自主探索", "Free exploration");
    seedPair("status.attendingclass", "正在上魔咒课", "Attending Charms class");
    seedPair("loc.charmsclassroom", "魔咒课教室", "Charms Classroom");
    seedPair("status.none",         "暂无",         "None");

    // ---------------- 聊天 ----------------
    seedPair("chat.placeholder",     "向当前校园频道留言...",  "Write to the current campus channel...");
    seedPair("chat.send",            "发送",                  "Send");
    seedPair("chat.channel.location","地点",                  "Location");
    seedPair("chat.channel.world",   "世界",                  "World");
    seedPair("chat.channel.party",   "组队",                  "Party");
    seedPair("chat.system",          "[系统]",                "[System]");
    seedPair("chat.you",             "我",                    "You");
    seedPair("chat.kind.narration",  "旁白",                  "Narration");
    seedPair("chat.kind.world",      "世界",                  "World");
    seedPair("chat.kind.notice",     "校园通知",              "Campus Notice");
    seedPair("chat.feedback.disconnected", "已断开 · 网络层预留", "Disconnected · Network layer reserved");

    // ---------------- 聊天种子消息 ----------------
    seedPair("chat.seed.breakfast",
             "早餐时间已经开始，大礼堂的长桌正在等待学生入席。",
             "Breakfast has begun. The long tables in the Great Hall await the students.");
    seedPair("chat.seed.class",
             "变形课还有十五分钟开始。别让我看见任何人迟到。",
             "Transfiguration begins in fifteen minutes. I expect no one to be late.");
    seedPair("chat.seed.library",
             "今天图书馆格外安静……如果你在找魔咒史资料，我旁边还有位置。",
             "The library is unusually quiet today... If you need Charms history, there is a seat beside me.");
    seedPair("chat.seed.enter_class",
             "加雷斯·韦斯莱推开教室的门，匆匆在后排坐下。",
             "Gareth Weasley slips through the classroom door and takes a seat at the back.");
    seedPair("chat.seed.lesson_start",
             "教室门在身后轻轻合拢。麦格教授抬起魔杖，谈话声随即安静下来。",
             "The classroom door clicks shut. Professor McGonagall raises her wand, and the murmurs fade.");

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
