-- migrate_v4_generic_story_engine.sql
-- Adds versioned multi-chapter state, structured NPC memory, and an append-only
-- event log. The procedure guard keeps repeated imports safe.

USE arcane_campus_online;

DROP PROCEDURE IF EXISTS apply_v4_generic_story_engine;

DELIMITER $$
CREATE PROCEDURE apply_v4_generic_story_engine()
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM schema_migrations WHERE version = 'v4_generic_story_engine'
    ) THEN
        ALTER TABLE character_story_states
            ADD COLUMN content_version INT UNSIGNED NOT NULL DEFAULT 1 AFTER chapter_id,
            ADD COLUMN status VARCHAR(24) NOT NULL DEFAULT 'active' AFTER stage,
            ADD COLUMN state_json JSON NULL AFTER flags,
            ADD COLUMN revision BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER state_json,
            ADD COLUMN started_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP AFTER revision,
            ADD COLUMN completed_at TIMESTAMP NULL DEFAULT NULL AFTER started_at,
            DROP PRIMARY KEY,
            ADD PRIMARY KEY (character_id, chapter_id),
            ADD KEY idx_story_state_active (character_id, status, updated_at);

        UPDATE character_story_states
        SET state_json = JSON_OBJECT(
            'legacy_flags', flags,
            'flags', JSON_ARRAY(),
            'visited_scenes', JSON_ARRAY(scene_id),
            'variables', JSON_OBJECT()
        )
        WHERE state_json IS NULL;

        ALTER TABLE npc_memories
            ADD COLUMN memory_json JSON NULL AFTER memory_flags,
            ADD COLUMN encounter_count INT UNSIGNED NOT NULL DEFAULT 0 AFTER affinity,
            ADD COLUMN revision BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER memory_json;

        UPDATE npc_memories
        SET memory_json = JSON_OBJECT(
            'legacy_flags', memory_flags,
            'facts', JSON_ARRAY(),
            'last_scene_id', '',
            'times_met', encounter_count
        )
        WHERE memory_json IS NULL;

        CREATE TABLE story_event_log (
            event_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
            character_id BIGINT UNSIGNED NOT NULL,
            chapter_id VARCHAR(64) NOT NULL,
            scene_id VARCHAR(64) NOT NULL,
            action_id VARCHAR(64) NOT NULL,
            result_json JSON NOT NULL,
            created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (event_id),
            KEY idx_story_events_character_time (character_id, created_at),
            KEY idx_story_events_chapter (chapter_id, scene_id),
            CONSTRAINT fk_story_event_character
                FOREIGN KEY (character_id) REFERENCES characters(character_id)
                ON DELETE CASCADE ON UPDATE CASCADE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

        INSERT INTO schema_migrations (version) VALUES ('v4_generic_story_engine');
    END IF;
END$$
DELIMITER ;

CALL apply_v4_generic_story_engine();
DROP PROCEDURE apply_v4_generic_story_engine;
