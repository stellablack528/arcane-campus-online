#include "core/LocationData.h"

QList<LocationInfo> LocationPresets::all()
{
    return {

        // ===== 霍格沃茨城堡 =====
        {"great_hall", "Great Hall", "礼堂",
         "The heart of Hogwarts. Floating candles, enchanted ceiling, "
         "and four long house tables where students gather for feasts.",
         "霍格沃茨的心脏。漂浮的蜡烛、魔法天花板、四张学院长桌——"
         "学生们在此享受盛宴。",
         "🏛", LocationCategory::Hogwarts},

        {"library", "Library", "图书馆",
         "Thousands of magical books, restricted section guarded by Madam Pince. "
         "Perfect for research, homework, and quiet study.",
         "上千本魔法书籍，禁书区由平斯夫人严密看守。适合研究和安静自习。",
         "📚", LocationCategory::Hogwarts},

        {"astronomy_tower", "Astronomy Tower", "天文塔",
         "The tallest tower of Hogwarts. Observatory at the top with telescopes "
         "for night-sky studies. Windy, cold, and breathtaking.",
         "霍格沃茨最高的塔楼。塔顶有观星台和望远镜，适合夜间天文课。"
         "风大、寒冷、景色绝美。",
         "🔭", LocationCategory::Hogwarts},

        {"gryffindor_common", "Gryffindor Common Room", "格兰芬多公共休息室",
         "Behind the portrait of the Fat Lady. Cozy red sofas, roaring fireplace, "
         "and the spirit of bravery and chivalry.",
         "胖夫人画像背后。舒适的红色沙发、噼啪作响的壁炉、勇气与侠义的精神。",
         "🦁", LocationCategory::Hogwarts},

        {"ravenclaw_common", "Ravenclaw Common Room", "拉文克劳公共休息室",
         "At the top of Ravenclaw Tower. Starry ceiling, bookshelves everywhere, "
         "and a bronze eagle doorknocker that asks riddles.",
         "位于拉文克劳塔顶。星空天花板、满墙书架、"
         "需要回答铜鹰门环的谜语才能进入。",
         "🦅", LocationCategory::Hogwarts},

        {"hufflepuff_common", "Hufflepuff Common Room", "赫奇帕奇公共休息室",
         "Near the kitchens. Round, earthy, filled with plants and the scent of "
         "baking. The coziest of all common rooms.",
         "靠近厨房。圆形空间，充满绿植和烘焙香，是全霍格沃茨最温馨的公共休息室。",
         "🦡", LocationCategory::Hogwarts},

        {"slytherin_common", "Slytherin Common Room", "斯莱特林公共休息室",
         "In the dungeons beneath the Black Lake. Greenish light filters through "
         "the windows, showing merpeople and giant squid swimming by.",
         "在黑湖之下的地牢中。窗外透入幽绿的光，人鱼和巨乌贼从窗前游过。",
         "🐍", LocationCategory::Hogwarts},

        {"room_of_requirement", "Room of Requirement", "有求必应屋",
         "Appears only when someone needs it. It becomes whatever the seeker requires "
         "— training room, hideout, or lost-item repository.",
         "仅在有人迫切需要时出现。按所求之人变成训练室、藏身所或物品回收间。",
         "🚪", LocationCategory::Hogwarts},

        {"chamber_of_secrets", "Chamber of Secrets", "密室",
         "Hidden deep beneath the school, built by Salazar Slytherin. "
         "Rumored to house an ancient danger. Tread carefully.",
         "深埋在学校地底，由萨拉查·斯莱特林建造。据说藏有远古的危险。小心前行。",
         "🐲", LocationCategory::Hogwarts},

        {"transfiguration_class", "Transfiguration Classroom", "变形术教室",
         "Professor McGonagall's classroom. Strict atmosphere, high expectations, "
         "and rows of desks where magic transforms the ordinary.",
         "麦格教授的教室。严苛的氛围、极高的期望，一排排桌椅见证平凡化非凡的魔法。",
         "✨", LocationCategory::Hogwarts},

        // ===== 禁林 =====
        {"forbidden_forest", "Forbidden Forest", "禁林",
         "A vast, dark woodland bordering Hogwarts. Home to centaurs, unicorns, "
         "Acromantulas, and many creatures best not disturbed.",
         "霍格沃茨边界外的广袤黑森林。马人、独角兽、八眼巨蛛的家园——"
         "许多生物最好不要惊动。",
         "🌲", LocationCategory::Forbidden},

        {"hagrid_hut", "Hagrid's Hut", "海格的小屋",
         "At the edge of the Forbidden Forest. A warm, oversized cottage with "
         "rock cakes, tea, and Fang the boarhound.",
         "禁林边缘。温暖宽敞的木屋，有岩皮饼、热茶和一只叫牙牙的猎犬。",
         "🛖", LocationCategory::Forbidden},

        // ===== 霍格莫德村 =====
        {"hogsmeade", "Hogsmeade Village", "霍格莫德村",
         "The only all-wizarding village in Britain. Cobblestone streets, "
         "sweet shops, and the famous Three Broomsticks pub.",
         "英国唯一的全巫师村落。鹅卵石街道、糖果店和著名的三把扫帚酒吧。",
         "🏘", LocationCategory::Hogsmeade},

        {"three_broomsticks", "Three Broomsticks", "三把扫帚",
         "Madam Rosmerta's cosy pub. The best Butterbeer in Scotland, "
         "and a favourite hangout for Hogwarts students on weekends.",
         "罗斯默塔女士的温馨酒吧。全苏格兰最好的黄油啤酒，"
         "霍格沃茨学生周末最爱的聚集地。",
         "🍺", LocationCategory::Hogsmeade},

        {"honeydukes", "Honeydukes", "蜂蜜公爵",
         "The sweetest shop in Hogsmeade. Every kind of magical confection: "
         "Chocolate Frogs, Bertie Bott's Beans, Fizzing Whizbees, and more.",
         "霍格莫德最甜的店铺。各种魔法糖果：巧克力蛙、比比多味豆、嘶嘶蜜蜂糖……",
         "🍬", LocationCategory::Hogsmeade},

        {"zonkos", "Zonko's Joke Shop", "佐科笑话店",
         "A wonderland of magical pranks. Dungbombs, Hiccup Sweets, "
         "and everything a young prankster could dream of.",
         "魔法恶作剧天堂。粪弹、打嗝糖，年轻捣蛋鬼梦寐以求的一切。",
         "🎭", LocationCategory::Hogsmeade},

        {"shrieking_shack", "Shrieking Shack", "尖叫棚屋",
         "The most haunted building in Britain — or so people say. "
         "Connected to Hogwarts by a secret passage under the Whomping Willow.",
         "号称英国最闹鬼的建筑——至少人们是这么说的。"
         "通过打人柳下的密道与霍格沃茨相连。",
         "🏚", LocationCategory::Hogsmeade},

        // ===== 对角巷 =====
        {"diagon_alley", "Diagon Alley", "对角巷",
         "A cobbled wizarding street in London. Shops for every magical need: "
         "wands, books, cauldrons, robes, and a goblin-run bank.",
         "伦敦的鹅卵石魔法街道。满足一切魔法需求的店铺："
         "魔杖、书籍、坩埚、长袍，还有妖精经营的银行。",
         "🛒", LocationCategory::DiagonAlley},

        {"ollivanders_shop", "Ollivanders", "奥利凡德魔杖店",
         "Makers of fine wands since 382 B.C. Dusty shelves tower to the ceiling, "
         "each box holding a wand waiting for its wizard.",
         "自公元前382年起制作精良魔杖。布满灰尘的架子直达天花板，"
         "每个盒子都装着一根等待主人的魔杖。",
         "🪄", LocationCategory::DiagonAlley},

        {"gringotts", "Gringotts Wizarding Bank", "古灵阁巫师银行",
         "Run by goblins. The safest place to store your gold — guarded by "
         "dragons, enchantments, and extremely grumpy creatures.",
         "妖精经营。最安全的黄金存放处——由巨龙、魔法和脾气极坏的生物守护。",
         "🏦", LocationCategory::DiagonAlley},

        {"flourish_blotts", "Flourish & Blotts", "丽痕书店",
         "The finest magical bookshop in London. From 'Hogwarts: A History' to "
         "the latest Gilderoy Lockhart bestseller (signed copies available).",
         "伦敦最好的魔法书店。从《霍格沃茨·一段校史》到吉德罗·洛哈特最新畅销书。",
         "📖", LocationCategory::DiagonAlley},

        {"leaky_cauldron", "Leaky Cauldron", "破釜酒吧",
         "A grubby but welcoming pub connecting Muggle London to Diagon Alley. "
         "Tom the innkeeper always has a room and a hot meal.",
         "连接麻瓜伦敦和对角巷的破旧但温馨的酒吧。老板汤姆总有一间房和热乎的饭菜。",
         "🍻", LocationCategory::DiagonAlley},

        // ===== 翻倒巷 =====
        {"knockturn_alley", "Knockturn Alley", "翻倒巷",
         "A dark, twisting side-street off Diagon Alley. Shops dealing in the "
         "Dark Arts, cursed artifacts, and things better left unsaid.",
         "对角巷旁一条阴暗曲折的小巷。经营黑魔法物品、诅咒器具和不可言说的东西。",
         "🌑", LocationCategory::Knockturn},

        {"borgin_burkes", "Borgin & Burkes", "博金-博克",
         "The most notorious shop in Knockturn Alley. Antiques with dark histories, "
         "vanishing cabinets, and cursed necklaces.",
         "翻倒巷最臭名昭著的店铺。拥有黑暗历史的古董、消失柜和受诅咒的项链。",
         "💀", LocationCategory::Knockturn},

        // ===== 魔法部 =====
        {"ministry_of_magic", "Ministry of Magic", "魔法部",
         "The magical government of Britain. Underground offices, the Atrium "
         "with the Fountain of Magical Brethren, and too much paperwork.",
         "英国魔法政府。地下办公区、魔法兄弟喷泉大厅——以及山一般的文书工作。",
         "🏛", LocationCategory::Ministry},

        // ===== 圣芒戈 =====
        {"st_mungo", "St. Mungo's Hospital", "圣芒戈魔法伤病医院",
         "Magical hospital hidden behind a red-bricked department store. "
         "Floors for different ailments: Creature-Induced Injuries, Potion Accidents, "
         "and the Janus Thickey Ward for permanent spell damage.",
         "隐藏在一家红砖百货店后的魔法医院。不同楼层对应不同伤病："
         "魔法生物伤害层、魔药事故层、以及长期魔咒伤害的杰纳斯·西奇病房。",
         "🏥", LocationCategory::StMungo},

        // ===== 魁地奇 =====
        {"quidditch_pitch", "Quidditch Pitch", "魁地奇球场",
         "A vast oval pitch with three golden hoops at each end. "
         "Home to house matches, flying practice, and the roar of the crowd.",
         "巨大的椭圆形球场，两端各有三个金色球框。"
         "学院赛、飞行训练和观众欢呼的所在地。",
         "🏟", LocationCategory::Hogwarts},
    };
}

QString LocationPresets::categoryName(LocationCategory cat, bool chinese)
{
    switch (cat) {
        case LocationCategory::Hogwarts:   return chinese ? "🏰 霍格沃茨城堡"    : "🏰 Hogwarts Castle";
        case LocationCategory::Hogsmeade:  return chinese ? "🏘 霍格莫德村"      : "🏘 Hogsmeade Village";
        case LocationCategory::DiagonAlley: return chinese ? "🛒 对角巷"         : "🛒 Diagon Alley";
        case LocationCategory::Knockturn:  return chinese ? "🌑 翻倒巷"          : "🌑 Knockturn Alley";
        case LocationCategory::Forbidden:  return chinese ? "🌲 禁林"            : "🌲 Forbidden Forest";
        case LocationCategory::Ministry:   return chinese ? "🏛 魔法部"          : "🏛 Ministry of Magic";
        case LocationCategory::StMungo:    return chinese ? "🏥 圣芒戈医院"      : "🏥 St. Mungo's";
    }
    return {};
}
