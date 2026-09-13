-- migrate_v2.sql — 开局剧情进度字段
-- 在 characters 表新增 prologue_step，记录玩家当前剧情阶段。
-- 枚举值：DIAGON_ALLEY_PET / DIAGON_ALLEY_WAND / EXPRESS / SORTING_HAT / COMPLETED

USE arcane_campus_online;

ALTER TABLE characters
    ADD COLUMN prologue_step VARCHAR(30) NOT NULL DEFAULT 'DIAGON_ALLEY_PET'
    AFTER blood_status;

-- 记录迁移标记。
INSERT INTO schema_migrations (version) VALUES ('v2_prologue_step')
    ON DUPLICATE KEY UPDATE applied_at = applied_at;
