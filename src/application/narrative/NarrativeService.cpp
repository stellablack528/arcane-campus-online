#include "application/narrative/NarrativeService.hpp"

#include <algorithm>
#include <string>

namespace arcane::application::service {

namespace {

// 将分数夹到 [-100, 100] 区间。
constexpr std::int16_t clamp16(int v) noexcept
{
    if (v > 100) return 100;
    if (v < -100) return -100;
    return static_cast<std::int16_t>(v);
}

// 数值 → 站位标签。
std::string standingFor(int score) noexcept
{
    if (score >= 50) return "Beloved";
    if (score >= 20) return "Liked";
    if (score >= 5)  return "Friendly";
    if (score >= -19) return "Neutral";
    if (score >= -49) return "Disliked";
    return "Shunned";
}

// 关系站位标签（综合好感+信任）。
std::string relationshipStandingFor(const do_model::RelationshipDO& r) noexcept
{
    const int avg = (r.favorability + r.trust) / 2;
    const bool isRival = std::find(r.tags.begin(), r.tags.end(), "rival") != r.tags.end();
    if (isRival && avg < 0) return "Rival";
    if (avg >= 60) return "Trusted friend";
    if (avg >= 25) return "Friend";
    if (avg >= 5)  return "Acquaintance";
    if (avg >= -19) return "Neutral";
    if (avg >= -49) return "Wary";
    return "Hostile";
}

std::string signedDelta(int v)
{
    return (v >= 0 ? "+" : "") + std::to_string(v);
}

} // namespace

NarrativeService::NarrativeService() noexcept
{
    seedCatalog();
}

void NarrativeService::seedCatalog()
{
    using CC = do_model::ChoiceConsequenceDO;
    catalog_ = {
        CC{"share_notes", "Share notes with Hermione",
           "Lend your carefully kept Transfiguration notes to a struggling classmate.",
           /*peer*/3, /*teacher*/1, /*house*/0,
           "hermione", /*fav*/8, /*trust*/5, {"helpful"},
           "Hermione beams as she copies your diagrams. \"You're a lifesaver.\" Word "
           "spreads that you're generous with your time."},

        CC{"restricted_section", "Sneak into the Restricted Section",
           "Slip past the librarian after dark to consult a forbidden tome.",
           /*peer*/-5, /*teacher*/-10, /*house*/-3,
           "snape", /*fav*/-6, /*trust*/-4, {"rulebreaker"},
           "A lamp flickers. Madame Pince's shadow falls across the aisle. You're "
           "escorted out under Snape's cold gaze — five points lost."},

        CC{"stand_up_for_friend", "Stand up for a friend",
           "Confront a bully who cornered a younger student in the corridor.",
           /*peer*/5, /*teacher*/0, /*house*/2,
           "neville", /*fav*/10, /*trust*/8, {"loyal"},
           "Neville stammers his thanks, eyes bright. The portraits murmur "
           "approval — courage travels fast through these halls."},

        CC{"cheat_potions_exam", "Cheat on the Potions exam",
           "Smuggle a cheat sheet into Slughorn's midterm.",
           /*peer*/2, /*teacher*/-15, /*house*/-4,
           "hermione", /*fav*/-10, /*trust*/-12, {"disapproved"},
           "Hermione's lips thin into a disappointed line. Slughorn notices "
           "the parchment. Your house points evaporate."},

        CC{"help_hagrid", "Help Hagrid with the creatures",
           "Spend a free afternoon mucking out hippogriff paddocks.",
           /*peer*/2, /*teacher*/1, /*house*/0,
           "hagrid", /*fav*/12, /*trust*/10, {"kindhearted"},
           "Hagrid presses a rock cake into your hands, beaming. \"Yer a "
           "good one.\" Buckbeak bows as you leave."},

        CC{"duel_malfoy", "Duel Draco Malfoy",
           "Answer Malfoy's taunts with raised wand in the courtyard.",
           /*peer*/4, /*teacher*/-2, /*house*/3,
           "malfoy", /*fav*/-15, /*trust*/-8, {"rival"},
           "Sparks fly; Malfoy retreats with a snarl. Gryffindors cheer, "
           "but a professor's watchful eye lingers."},

        CC{"study_late_library", "Study late in the library",
           "Burn the midnight oil over Charms theory until the candles gutter.",
           /*peer*/1, /*teacher*/3, /*house*/1,
           "hermione", /*fav*/3, /*trust*/3, {"diligent"},
           "Flitwick passes your table and nods, pleased. Hermione slides "
           "a bookmark your way — quiet solidarity among the studious."},
    };
}

void NarrativeService::seedRelationshipsFor(std::uint64_t characterId)
{
    do_model::ReputationDO rep;
    rep.characterId = characterId;
    rep.relationships = {
        {"hermione",   "Hermione Granger",     "classmate", 10, 5,  {"acquaintance"}, ""},
        {"malfoy",     "Draco Malfoy",          "rival",     -20, -10, {"rival"}, ""},
        {"snape",      "Professor Snape",       "teacher",  -5, 0,   {"strict"}, ""},
        {"mcgonagall", "Professor McGonagall",  "teacher",  5,  5,   {"fair"}, ""},
        {"hagrid",     "Rubeus Hagrid",         "staff",    15, 10,  {"friend"}, ""},
        {"neville",    "Neville Longbottom",     "housemate", 8, 6,  {"housemate"}, ""},
    };
    reputationStore_[characterId] = std::move(rep);
}

const std::vector<do_model::ChoiceConsequenceDO>& NarrativeService::availableChoices() const noexcept
{
    return catalog_;
}

vo::ReputationVO NarrativeService::queryReputation(const dto::ReputationQueryDTO& request)
{
    vo::ReputationVO out;
    auto it = reputationStore_.find(request.characterId);
    if (it == reputationStore_.end()) {
        seedRelationshipsFor(request.characterId);
        it = reputationStore_.find(request.characterId);
    }
    const auto& rep = it->second;

    out.success = true;
    out.message = "Reputation snapshot for character " + std::to_string(request.characterId);
    out.peerReputation = rep.peerReputation;
    out.teacherReputation = rep.teacherReputation;
    out.houseReputation = rep.houseReputation;
    out.peerStanding = standingFor(rep.peerReputation);
    out.teacherStanding = standingFor(rep.teacherReputation);
    out.houseStanding = standingFor(rep.houseReputation);

    out.relationships.reserve(rep.relationships.size());
    for (const auto& r : rep.relationships) {
        vo::RelationshipVO rv;
        rv.npcId = r.npcId;
        rv.displayName = r.displayName;
        rv.role = r.role;
        rv.favorability = r.favorability;
        rv.trust = r.trust;
        rv.standing = relationshipStandingFor(r);
        rv.tags = r.tags;
        out.relationships.push_back(std::move(rv));
    }
    return out;
}

vo::NarrativeResultVO NarrativeService::applyChoice(const dto::ChoiceRequestDTO& request)
{
    vo::NarrativeResultVO out;

    // 查后果目录。
    auto cit = std::find_if(catalog_.begin(), catalog_.end(),
        [&](const do_model::ChoiceConsequenceDO& c) { return c.choiceId == request.choiceId; });
    if (cit == catalog_.end()) {
        out.success = false;
        out.message = "Unknown choice: " + request.choiceId;
        return out;
    }

    // 取出/种入风评。
    auto it = reputationStore_.find(request.characterId);
    if (it == reputationStore_.end()) {
        seedRelationshipsFor(request.characterId);
        it = reputationStore_.find(request.characterId);
    }
    auto& rep = it->second;

    // 应用聚合变动。
    rep.peerReputation = clamp16(rep.peerReputation + cit->peerDelta);
    rep.teacherReputation = clamp16(rep.teacherReputation + cit->teacherDelta);
    rep.houseReputation = clamp16(rep.houseReputation + cit->houseDelta);

    // 应用目标关系变动。
    if (!cit->targetNpcId.empty()) {
        auto rit = std::find_if(rep.relationships.begin(), rep.relationships.end(),
            [&](const do_model::RelationshipDO& r) { return r.npcId == cit->targetNpcId; });
        if (rit != rep.relationships.end()) {
            rit->favorability = clamp16(rit->favorability + cit->favorabilityDelta);
            rit->trust = clamp16(rit->trust + cit->trustDelta);
            for (const auto& tag : cit->addTags) {
                if (std::find(rit->tags.begin(), rit->tags.end(), tag) == rit->tags.end()) {
                    rit->tags.push_back(tag);
                }
            }
            rit->note = cit->narrativeText;
        }
    }

    // 组装结果。
    out.success = true;
    out.message = cit->narrativeText;

    // 后果摘要：列出本次变动的方向。
    std::string summary;
    if (cit->peerDelta)    summary += "peer " + signedDelta(cit->peerDelta) + ", ";
    if (cit->teacherDelta) summary += "teacher " + signedDelta(cit->teacherDelta) + ", ";
    if (cit->houseDelta)   summary += "house " + signedDelta(cit->houseDelta) + ", ";
    if (!cit->targetNpcId.empty() && (cit->favorabilityDelta || cit->trustDelta)) {
        summary += cit->targetNpcId + " favorability " + signedDelta(cit->favorabilityDelta)
                   + ", trust " + signedDelta(cit->trustDelta) + ", ";
    }
    if (!summary.empty()) {
        summary.pop_back();   // trailing space
        summary.pop_back();   // trailing comma
    }
    out.consequenceSummary = summary;

    out.reputation = queryReputation({request.characterId});
    return out;
}

} // namespace arcane::application::service
