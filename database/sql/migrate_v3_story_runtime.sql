-- migrate_v3_story_runtime.sql
-- Local story persistence for the first-year Diagon Alley chapter.

USE arcane_campus_online;

CREATE TABLE IF NOT EXISTS character_story_states (
    character_id BIGINT UNSIGNED NOT NULL,
    chapter_id VARCHAR(64) NOT NULL,
    scene_id VARCHAR(64) NOT NULL,
    stage VARCHAR(64) NOT NULL,
    flags TEXT NOT NULL,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (character_id),
    CONSTRAINT fk_story_state_character
        FOREIGN KEY (character_id) REFERENCES characters(character_id)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS character_wands (
    character_id BIGINT UNSIGNED NOT NULL,
    wood_id VARCHAR(64) NOT NULL,
    core_id VARCHAR(64) NOT NULL,
    wood_name VARCHAR(96) NOT NULL,
    core_name VARCHAR(96) NOT NULL,
    length_inches DECIMAL(5,2) NOT NULL,
    flexibility VARCHAR(96) NOT NULL,
    handle_style VARCHAR(96) NOT NULL,
    finish VARCHAR(96) NOT NULL,
    color_tone VARCHAR(96) NOT NULL,
    engraving VARCHAR(64) NOT NULL DEFAULT '',
    claimed BOOLEAN NOT NULL DEFAULT FALSE,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (character_id),
    CONSTRAINT fk_character_wand_character
        FOREIGN KEY (character_id) REFERENCES characters(character_id)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS npc_memories (
    character_id BIGINT UNSIGNED NOT NULL,
    npc_id VARCHAR(64) NOT NULL,
    affinity INT NOT NULL DEFAULT 0,
    memory_flags TEXT NOT NULL,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (character_id, npc_id),
    KEY idx_npc_memories_npc_id (npc_id),
    CONSTRAINT fk_npc_memory_character
        FOREIGN KEY (character_id) REFERENCES characters(character_id)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO rooms (room_id, room_name, room_type, description, max_players, is_restricted)
VALUES (9, 'Diagon Alley', 'shopping', 'A winding wizarding street of shops, voices, owls, and first-year discoveries.', 120, FALSE)
ON DUPLICATE KEY UPDATE description = VALUES(description), max_players = VALUES(max_players);

INSERT INTO items (item_id, item_name, item_type, description, stackable)
VALUES
    (12, 'First-Year Book and Writing Set', 'school_supply', 'The complete first-year reading list and a new quill from Flourish and Blotts.', FALSE),
    (13, 'First-Year Potion Kit', 'school_supply', 'A brass cauldron, glass phials, scales, and a starter bundle of potion ingredients.', FALSE)
ON DUPLICATE KEY UPDATE item_type = VALUES(item_type), description = VALUES(description), stackable = VALUES(stackable);

INSERT INTO schema_migrations (version) VALUES ('v3_story_runtime')
ON DUPLICATE KEY UPDATE applied_at = applied_at;
