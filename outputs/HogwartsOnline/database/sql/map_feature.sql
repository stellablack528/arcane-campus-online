USE arcane_campus_online;

-- ============================================================
-- 🗺️ 活点地图功能 — 新增表
-- ============================================================

CREATE TABLE IF NOT EXISTS player_location (
    character_id   BIGINT UNSIGNED NOT NULL,
    room_name      VARCHAR(96)  NOT NULL DEFAULT 'Great Hall',
    x_pos          INT          NOT NULL DEFAULT 0,
    y_pos          INT          NOT NULL DEFAULT 0,
    is_online      TINYINT(1)   NOT NULL DEFAULT 0,
    last_seen      TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (character_id),
    INDEX idx_player_location_room (room_name),
    INDEX idx_player_location_online (is_online, last_seen)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================
-- 📦 种子数据：玩家当前位置 + 活点地图物品
-- ============================================================

INSERT INTO items (item_id, item_name, item_type, description, stackable)
VALUES (7, 'Marauder''s Map', 'special', 'A magical parchment that shows the real-time location of everyone in the castle. Messrs. Moony, Wormtail, Padfoot, and Prongs are proud to present this masterpiece.', FALSE)
ON DUPLICATE KEY UPDATE item_type = VALUES(item_type), description = VALUES(description), stackable = VALUES(stackable);

INSERT INTO player_location (character_id, room_name, x_pos, y_pos, is_online)
VALUES
    (1, 'Library', 15, 42, 1),
    (2, 'Great Hall', 30, 10, 1),
    (3, 'Slytherin Common Room', 8, 25, 1),
    (4, 'Hufflepuff Common Room', 12, 18, 0)
ON DUPLICATE KEY UPDATE
    room_name = VALUES(room_name), x_pos = VALUES(x_pos),
    y_pos = VALUES(y_pos), is_online = VALUES(is_online);

-- 给几个测试角色发一张活点地图
INSERT INTO inventories (character_id, item_id, quantity)
VALUES (1, 7, 1), (2, 7, 1), (3, 7, 1)
ON DUPLICATE KEY UPDATE quantity = VALUES(quantity);
