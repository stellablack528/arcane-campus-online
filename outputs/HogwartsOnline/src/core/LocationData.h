#pragma once

#include <QString>
#include <QList>

// 场景区域分类
enum class LocationCategory {
    Hogwarts,       // 霍格沃茨城堡
    Hogsmeade,      // 霍格莫德村
    DiagonAlley,    // 对角巷
    Knockturn,      // 翻倒巷
    Forbidden,      // 禁林
    Ministry,       // 魔法部
    StMungo,        // 圣芒戈
};

// 单个场景信息
struct LocationInfo {
    QString id;             // 内部 ID (对应数据库 scene_id)
    QString nameEn;         // 英文名
    QString nameCn;         // 中文名
    QString descriptionEn;  // 英文描述
    QString descriptionCn;  // 中文描述
    QString emoji;          // 图标
    LocationCategory category;
    int onlineCount = 0;    // 当前在线人数
};

// 所有场景数据
class LocationPresets {
public:
    static QList<LocationInfo> all();
    static QString categoryName(LocationCategory cat, bool chinese);
};
