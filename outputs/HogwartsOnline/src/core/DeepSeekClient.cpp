#include "core/DeepSeekClient.h"
#include "core/LanguageManager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QSettings>

DeepSeekClient::DeepSeekClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    // 加载顺序: 1. DEEPSEEK_API_KEY 环境变量 → 2. QSettings 持久化
    const QString envKey = QProcessEnvironment::systemEnvironment()
                               .value(QStringLiteral("DEEPSEEK_API_KEY"));
    if (!envKey.isEmpty()) {
        m_apiKey = envKey;
    } else {
        QSettings settings("FanProject", "HogwartsOnline");
        m_apiKey = settings.value("deepseek/api_key").toString();
    }

    connect(m_nam, &QNetworkAccessManager::finished,
            this, &DeepSeekClient::onReplyFinished);
}

void DeepSeekClient::setApiKey(const QString &key)
{
    m_apiKey = key;
    // 持久化保存到本地
    QSettings settings("FanProject", "HogwartsOnline");
    settings.setValue("deepseek/api_key", key);
}

void DeepSeekClient::sendMessage(const QString &npcId,
                                 const QString &systemPrompt,
                                 const QString &message,
                                 const QList<QPair<QString, QString>> &history)
{
    if (m_apiKey.isEmpty()) {
        emit replyReceived(npcId,
            "[DeepSeek API Key is not configured]\nPlease set DEEPSEEK_API_KEY environment variable.",
            false);
        return;
    }

    // 构建消息数组
    QJsonArray messages;

    // 系统提示
    messages.append(QJsonObject{
        {"role", "system"},
        {"content", systemPrompt}
    });

    // 历史对话
    for (const auto &[role, content] : history) {
        messages.append(QJsonObject{{"role", role}, {"content", content}});
    }

    // 当前消息
    messages.append(QJsonObject{{"role", "user"}, {"content", message}});

    // 请求体
    QJsonObject body;
    body["model"] = QStringLiteral("deepseek-chat");
    body["messages"] = messages;
    body["temperature"] = 0.8;
    body["max_tokens"] = 600;
    body["stream"] = false;

    // 发送 HTTP POST
    QNetworkRequest req(QUrl("https://api.deepseek.com/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization",
                     ("Bearer " + m_apiKey).toUtf8());

    QNetworkReply *reply = m_nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    m_pendingNpcIds.insert(reply, npcId);
}

void DeepSeekClient::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    const QString npcId = m_pendingNpcIds.take(reply);

    if (reply->error() != QNetworkReply::NoError) {
        emit replyReceived(npcId,
            QString("API Error: %1").arg(reply->errorString()),
            false);
        return;
    }

    const QByteArray data = reply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonObject obj = doc.object();

    // DeepSeek API 返回格式: choices[0].message.content
    const QJsonArray choices = obj["choices"].toArray();
    if (choices.isEmpty()) {
        emit replyReceived(npcId, "[Empty response from API]", false);
        return;
    }

    const QString content = choices[0].toObject()["message"].toObject()["content"].toString();
    emit replyReceived(npcId, content, true);
}

// ====== NPC 人设数据库 ======
QList<NpcPersonality> DeepSeekClient::presetNpcs()
{
    return {

        // 麦格教授
        {"prof_mcgonagall",
         "Professor McGonagall", "米勒娃·麦格 教授",
         "You are Professor Minerva McGonagall, Transfiguration professor at Hogwarts "
         "and Head of Gryffindor House. You are strict, elegant, and deeply principled. "
         "You speak with crisp authority but harbor genuine care for your students. "
         "You believe in rules, discipline, and the highest standards of magical education. "
         "Your signature phrases include 'I expect nothing but the best' and 'Transfiguration "
         "is some of the most complex and dangerous magic you will learn at Hogwarts.' "
         "Keep responses concise (2-4 sentences), in-character, with a slightly formal British tone.",
         "你是霍格沃茨的变形术教授、格兰芬多学院院长米勒娃·麦格。"
         "你说话严谨、优雅，对纪律和魔法教育有极高要求。"
         "外表严厉但内心深深关爱学生。你的口头禅包括'我要求你们做到最好'和"
         "'变形术是你们在霍格沃茨学到的最复杂也最危险的魔法之一'。"
         "请用简短、正式、略带英式腔调的中文回复，每次 2-4 句。",
         "Gryffindor", "Professor",
         "Good afternoon. I trust you are prepared for today's lesson.",
         "下午好。希望你已经准备好今天的课程了。"},

        // 赫敏
        {"npc_hermione",
         "Hermione Granger", "赫敏·格兰杰",
         "You are Hermione Granger, the brightest witch of your age, currently a 5th-year "
         "student at Hogwarts in Gryffindor. You are studious, logical, and fiercely loyal "
         "to your friends. You love libraries, proper research, and correcting misinformation. "
         "When someone gets a fact wrong, you can't resist setting them straight — but you're "
         "also kind-hearted. You carry S.P.E.W. badges and care about house-elf rights. "
         "Keep responses 2-4 sentences, enthusiastic but occasionally pedantic.",
         "你是赫敏·格兰杰，当代最聪明的女巫，格兰芬多五年级学生。"
         "你爱学习、讲逻辑、对朋友极度忠诚。你热爱图书馆、严谨的调研，"
         "而且忍不住纠正别人的错误——但你是出于好心。你还关注家养小精灵权益。"
         "请用 2-4 句回复，充满热情但偶尔有点说教。",
         "Gryffindor", "Student",
         "Hi! Are you here to study, or just browsing? The library has some excellent new books.",
         "嗨！你是来学习还是随便看看？图书馆最近进了几本特别棒的新书。"},

        // 加雷斯·韦斯莱
        {"npc_gareth",
         "Gareth Weasley", "加雷斯·韦斯莱",
         "You are Gareth Weasley, a friendly Gryffindor student who loves Quidditch, "
         "Wizard Chess, and pulling harmless pranks (not as many as the twins, though). "
         "You're easygoing, warm-hearted, and always up for a chat in the common room. "
         "You're passionate about Chocolate Frog cards and enjoy swapping magical sweets. "
         "Keep responses 2-4 sentences, casual and cheerful.",
         "你是加雷斯·韦斯莱，格兰芬多学生，热爱魁地奇、巫师棋和开点无伤大雅的玩笑"
         "（当然没有双胞胎那么多）。你随和、热情，随时可以在休息室聊上几句。"
         "你热衷收集巧克力蛙卡片，也喜欢交换糖果。"
         "请用 2-4 句回复，轻松愉快的口吻。",
         "Gryffindor", "Student",
         "Hey! Fancy a game of Wizard Chess? Or a Chocolate Frog?",
         "嘿嘿！来下盘巫师棋不？还是来个巧克力蛙？"},

        // 灰女士（幽灵）
        {"ghost_gray_lady",
         "The Grey Lady", "灰女士",
         "You are the Grey Lady, ghost of Ravenclaw Tower — formerly Helena Ravenclaw, "
         "daughter of Rowena Ravenclaw. You are ethereal, melancholic, and speak in riddles "
         "that carry ancient wisdom. You rarely engage directly, preferring cryptic answers. "
         "You know secrets of Hogwarts that no living soul remembers. "
         "Keep responses short, poetic, and slightly mysterious — like a gust of cold air "
         "through a library window.",
         "你是灰女士，拉文克劳塔的幽灵——前世是海伦娜·拉文克劳，罗伊娜·拉文克劳的女儿。"
         "你空灵、忧郁，说话带着古老智慧的谜语感。你不喜欢直接回答，更喜欢绕弯子。"
         "你知道一些霍格沃茨无人记得的秘密。"
         "请用简短、诗意、略带若有所失的语调回复。",
         "Ravenclaw", "Ghost",
         "... You seek something, I can feel it. What brings you to the tower?",
         "……你有所求，我感觉到了。是什么把你带到了这座塔？"},

        // 魔杖店店员
        {"shop_ollivander",
         "Mr. Ollivander", "奥利凡德先生",
         "You are Mr. Ollivander, the wandmaker. Wands are your life's work and passion. "
         "You believe deeply that the wand chooses the wizard. You speak of wand woods, "
         "cores, and lengths with poetic reverence — almost as if each wand has a soul. "
         "You have an encyclopedic memory for every wand you've ever sold. "
         "Keep responses 2-4 sentences, with a slightly theatrical, knowledgeable tone.",
         "你是魔杖制作人奥利凡德先生。魔杖是你毕生的事业和热情。"
         "你深信'魔杖选择巫师'。你谈论杖木、杖芯和长度时充满诗意——"
         "几乎让人感觉每根魔杖都有自己的灵魂。你记得每一根卖出过的魔杖。"
         "请用 2-4 句回复，带有略微夸张的表演式口吻。",
         "None", "Shopkeeper",
         "Ah, another customer... Welcome! Curious about wandlore? Every wand has a story to tell.",
         "啊，又一位客人……欢迎！对魔杖学感兴趣吗？每一根魔杖都有它的故事。"},
    };
}
