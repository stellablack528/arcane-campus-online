CREATE TABLE IF NOT EXISTS users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL DEFAULT '',
    email TEXT NOT NULL UNIQUE,
    avatar TEXT NOT NULL DEFAULT '',
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_login_at TEXT,
    status TEXT NOT NULL DEFAULT 'active'
);

CREATE TABLE IF NOT EXISTS rooms (
    room_id INTEGER PRIMARY KEY,
    room_name TEXT NOT NULL UNIQUE,
    room_type TEXT NOT NULL,
    description TEXT NOT NULL,
    max_players INTEGER NOT NULL DEFAULT 100,
    is_restricted INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS characters (
    character_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    nickname TEXT NOT NULL UNIQUE,
    house TEXT NOT NULL,
    level INTEGER NOT NULL DEFAULT 1,
    experience INTEGER NOT NULL DEFAULT 0,
    gold INTEGER NOT NULL DEFAULT 0,
    current_room_id INTEGER,
    title TEXT NOT NULL DEFAULT '',
    gender TEXT NOT NULL DEFAULT 'other',
    hair_color TEXT NOT NULL DEFAULT '',
    eye_color TEXT NOT NULL DEFAULT '',
    blood_status TEXT NOT NULL DEFAULT 'halfblood',
    create_time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (current_room_id) REFERENCES rooms(room_id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS friendships (
    user_id INTEGER NOT NULL,
    friend_id INTEGER NOT NULL,
    relation_status TEXT NOT NULL DEFAULT 'pending',
    create_time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, friend_id)
);

CREATE TABLE IF NOT EXISTS chat_messages (
    message_id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender_character_id INTEGER NOT NULL,
    room_id INTEGER NOT NULL,
    content TEXT NOT NULL,
    send_time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (sender_character_id) REFERENCES characters(character_id) ON DELETE CASCADE,
    FOREIGN KEY (room_id) REFERENCES rooms(room_id)
);

CREATE TABLE IF NOT EXISTS npcs (
    npc_id INTEGER PRIMARY KEY AUTOINCREMENT,
    npc_name TEXT NOT NULL,
    room_id INTEGER NOT NULL,
    description TEXT NOT NULL,
    personality TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS items (
    item_id INTEGER PRIMARY KEY,
    item_name TEXT NOT NULL UNIQUE,
    item_type TEXT NOT NULL,
    description TEXT NOT NULL,
    stackable INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS inventories (
    inventory_id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id INTEGER NOT NULL,
    item_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL DEFAULT 1,
    UNIQUE (character_id, item_id),
    FOREIGN KEY (character_id) REFERENCES characters(character_id) ON DELETE CASCADE,
    FOREIGN KEY (item_id) REFERENCES items(item_id)
);

CREATE TABLE IF NOT EXISTS quests (
    quest_id INTEGER PRIMARY KEY AUTOINCREMENT,
    quest_name TEXT NOT NULL UNIQUE,
    description TEXT NOT NULL,
    reward_gold INTEGER NOT NULL DEFAULT 0,
    reward_exp INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS character_quests (
    character_id INTEGER NOT NULL,
    quest_id INTEGER NOT NULL,
    progress INTEGER NOT NULL DEFAULT 0,
    status TEXT NOT NULL DEFAULT 'active',
    PRIMARY KEY (character_id, quest_id)
);

CREATE TABLE IF NOT EXISTS player_locations (
    character_id INTEGER PRIMARY KEY,
    room_id INTEGER NOT NULL,
    pos_x REAL NOT NULL DEFAULT 0,
    pos_y REAL NOT NULL DEFAULT 0,
    is_online INTEGER NOT NULL DEFAULT 0,
    last_update TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS character_story_states (
    character_id INTEGER NOT NULL,
    chapter_id TEXT NOT NULL,
    content_version INTEGER NOT NULL DEFAULT 1,
    scene_id TEXT NOT NULL,
    stage TEXT NOT NULL,
    flags TEXT NOT NULL DEFAULT '',
    status TEXT NOT NULL DEFAULT 'active',
    state_json TEXT NOT NULL DEFAULT '{}',
    revision INTEGER NOT NULL DEFAULT 0,
    started_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    completed_at TEXT,
    updated_at INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (character_id, chapter_id),
    FOREIGN KEY (character_id) REFERENCES characters(character_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS character_wands (
    character_id INTEGER PRIMARY KEY,
    wood_id TEXT NOT NULL,
    core_id TEXT NOT NULL,
    wood_name TEXT NOT NULL,
    core_name TEXT NOT NULL,
    length_inches REAL NOT NULL,
    flexibility TEXT NOT NULL,
    handle_style TEXT NOT NULL,
    finish TEXT NOT NULL,
    color_tone TEXT NOT NULL,
    engraving TEXT NOT NULL DEFAULT '',
    claimed INTEGER NOT NULL DEFAULT 0,
    updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS npc_memories (
    character_id INTEGER NOT NULL,
    npc_id TEXT NOT NULL,
    affinity INTEGER NOT NULL DEFAULT 0,
    encounter_count INTEGER NOT NULL DEFAULT 0,
    memory_flags TEXT NOT NULL DEFAULT '',
    memory_json TEXT NOT NULL DEFAULT '{}',
    revision INTEGER NOT NULL DEFAULT 0,
    updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (character_id, npc_id)
);

CREATE TABLE IF NOT EXISTS story_event_log (
    event_id INTEGER PRIMARY KEY AUTOINCREMENT,
    character_id INTEGER NOT NULL,
    chapter_id TEXT NOT NULL,
    scene_id TEXT NOT NULL,
    action_id TEXT NOT NULL,
    result_json TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TRIGGER IF NOT EXISTS story_state_insert AFTER INSERT ON character_story_states
FOR EACH ROW
BEGIN
    UPDATE character_story_states
    SET updated_at = (SELECT COALESCE(MAX(updated_at), 0) + 1 FROM character_story_states)
    WHERE character_id = NEW.character_id AND chapter_id = NEW.chapter_id;
END;

CREATE TRIGGER IF NOT EXISTS story_state_touch
AFTER UPDATE OF content_version, scene_id, stage, flags, status, state_json, revision, completed_at
ON character_story_states FOR EACH ROW
BEGIN
    UPDATE character_story_states
    SET updated_at = (SELECT COALESCE(MAX(updated_at), 0) + 1 FROM character_story_states)
    WHERE character_id = NEW.character_id AND chapter_id = NEW.chapter_id;
END;

INSERT OR IGNORE INTO rooms (room_id, room_name, room_type, description, max_players, is_restricted) VALUES
    (1, 'Great Hall', 'public', 'The heart of campus life.', 300, 0),
    (2, 'Library', 'study', 'Quiet shelves and old books.', 120, 0),
    (3, 'Astronomy Tower', 'exploration', 'A tower for stargazing.', 80, 1),
    (4, 'Gryffindor Common Room', 'house_common_room', 'A warm common room.', 80, 0),
    (5, 'Slytherin Common Room', 'house_common_room', 'A common room beneath the lake.', 80, 0),
    (6, 'Hufflepuff Common Room', 'house_common_room', 'A room near the kitchens.', 80, 0),
    (7, 'Ravenclaw Common Room', 'house_common_room', 'An airy tower.', 80, 0),
    (8, 'Forbidden Forest', 'danger', 'The edge of the grounds.', 30, 1),
    (9, 'Diagon Alley', 'shopping', 'A street of magical shops.', 120, 0),
    (10, 'Courtyard', 'public', 'A fountain and a lamp that stays lit.', 120, 0),
    (11, 'Transfiguration Classroom', 'study', 'A classroom of careful transformations.', 80, 0),
    (12, 'Hogsmeade', 'public', 'A nearby wizarding village.', 120, 0);

INSERT OR IGNORE INTO items (item_id, item_name, item_type, description, stackable) VALUES
    (1, 'Wand', 'equipment', 'A student wand.', 0),
    (2, 'Chocolate Frog', 'food', 'A lively sweet.', 1),
    (3, 'Butterbeer', 'drink', 'A warm drink.', 1),
    (4, 'Potion', 'consumable', 'A small bottle of potion.', 1),
    (5, 'Quill', 'tool', 'A writing quill.', 0),
    (6, 'Galleon Pouch', 'currency', 'A pouch for coins.', 0),
    (7, 'Marauder''s Map', 'tool', 'A magical map.', 0),
    (8, 'Owl', 'pet', 'A feathered companion.', 0),
    (9, 'Cat', 'pet', 'An independent companion.', 0),
    (10, 'Toad', 'pet', 'A traditional companion.', 0),
    (11, 'School Robe', 'equipment', 'Standard school robes.', 0),
    (12, 'First-Year Book and Writing Set', 'school_supply', 'The first-year reading list and quill.', 0),
    (13, 'First-Year Potion Kit', 'school_supply', 'A cauldron, phials, scales and supplies.', 0);

PRAGMA user_version = 1;
