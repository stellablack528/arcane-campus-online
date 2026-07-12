CREATE DATABASE IF NOT EXISTS arcane_campus_online
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_unicode_ci;

USE arcane_campus_online;

CREATE TABLE IF NOT EXISTS users (
    user_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    username VARCHAR(64) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    email VARCHAR(128) NOT NULL,
    avatar VARCHAR(255) NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_login_at TIMESTAMP NULL DEFAULT NULL,
    status ENUM('active', 'disabled', 'banned', 'deleted') NOT NULL DEFAULT 'active',
    PRIMARY KEY (user_id),
    UNIQUE KEY uk_users_username (username),
    UNIQUE KEY uk_users_email (email),
    KEY idx_users_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS rooms (
    room_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    room_name VARCHAR(96) NOT NULL,
    room_type VARCHAR(48) NOT NULL,
    description TEXT NOT NULL,
    max_players INT UNSIGNED NOT NULL DEFAULT 100,
    PRIMARY KEY (room_id),
    UNIQUE KEY uk_rooms_room_name (room_name),
    KEY idx_rooms_room_type (room_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS characters (
    character_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    user_id BIGINT UNSIGNED NOT NULL,
    nickname VARCHAR(64) NOT NULL,
    house ENUM('Gryffindor', 'Slytherin', 'Hufflepuff', 'Ravenclaw') NOT NULL,
    level INT UNSIGNED NOT NULL DEFAULT 1,
    experience BIGINT UNSIGNED NOT NULL DEFAULT 0,
    gold BIGINT UNSIGNED NOT NULL DEFAULT 0,
    current_room_id BIGINT UNSIGNED NULL,
    title VARCHAR(64) NULL,
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (character_id),
    UNIQUE KEY uk_characters_nickname (nickname),
    KEY idx_characters_user_id (user_id),
    KEY idx_characters_current_room_id (current_room_id),
    KEY idx_characters_house (house),
    CONSTRAINT fk_characters_user_id
        FOREIGN KEY (user_id) REFERENCES users(user_id)
        ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT fk_characters_current_room_id
        FOREIGN KEY (current_room_id) REFERENCES rooms(room_id)
        ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS friendships (
    user_id BIGINT UNSIGNED NOT NULL,
    friend_id BIGINT UNSIGNED NOT NULL,
    relation_status ENUM('pending', 'accepted', 'blocked') NOT NULL DEFAULT 'pending',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, friend_id),
    KEY idx_friendships_friend_id (friend_id),
    KEY idx_friendships_relation_status (relation_status),
    CONSTRAINT fk_friendships_user_id
        FOREIGN KEY (user_id) REFERENCES users(user_id)
        ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT fk_friendships_friend_id
        FOREIGN KEY (friend_id) REFERENCES users(user_id)
        ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT chk_friendships_not_self
        CHECK (user_id <> friend_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS chat_messages (
    message_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    sender_character_id BIGINT UNSIGNED NOT NULL,
    room_id BIGINT UNSIGNED NOT NULL,
    content TEXT NOT NULL,
    send_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (message_id),
    KEY idx_chat_messages_sender_character_id (sender_character_id),
    KEY idx_chat_messages_room_id (room_id),
    KEY idx_chat_messages_room_time (room_id, send_time),
    CONSTRAINT fk_chat_messages_sender_character_id
        FOREIGN KEY (sender_character_id) REFERENCES characters(character_id)
        ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT fk_chat_messages_room_id
        FOREIGN KEY (room_id) REFERENCES rooms(room_id)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS npcs (
    npc_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    npc_name VARCHAR(96) NOT NULL,
    room_id BIGINT UNSIGNED NOT NULL,
    description TEXT NOT NULL,
    personality TEXT NOT NULL,
    PRIMARY KEY (npc_id),
    KEY idx_npcs_room_id (room_id),
    KEY idx_npcs_npc_name (npc_name),
    CONSTRAINT fk_npcs_room_id
        FOREIGN KEY (room_id) REFERENCES rooms(room_id)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS items (
    item_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    item_name VARCHAR(96) NOT NULL,
    item_type VARCHAR(48) NOT NULL,
    description TEXT NOT NULL,
    stackable BOOLEAN NOT NULL DEFAULT FALSE,
    PRIMARY KEY (item_id),
    UNIQUE KEY uk_items_item_name (item_name),
    KEY idx_items_item_type (item_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS inventories (
    inventory_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    character_id BIGINT UNSIGNED NOT NULL,
    item_id BIGINT UNSIGNED NOT NULL,
    quantity INT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (inventory_id),
    UNIQUE KEY uk_inventories_character_item (character_id, item_id),
    KEY idx_inventories_character_id (character_id),
    KEY idx_inventories_item_id (item_id),
    CONSTRAINT fk_inventories_character_id
        FOREIGN KEY (character_id) REFERENCES characters(character_id)
        ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT fk_inventories_item_id
        FOREIGN KEY (item_id) REFERENCES items(item_id)
        ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT chk_inventories_quantity_positive
        CHECK (quantity > 0)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS quests (
    quest_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    quest_name VARCHAR(128) NOT NULL,
    description TEXT NOT NULL,
    reward_gold INT UNSIGNED NOT NULL DEFAULT 0,
    reward_exp INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (quest_id),
    UNIQUE KEY uk_quests_quest_name (quest_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS character_quests (
    character_id BIGINT UNSIGNED NOT NULL,
    quest_id BIGINT UNSIGNED NOT NULL,
    progress INT UNSIGNED NOT NULL DEFAULT 0,
    status ENUM('not_started', 'in_progress', 'completed', 'claimed') NOT NULL DEFAULT 'not_started',
    PRIMARY KEY (character_id, quest_id),
    KEY idx_character_quests_quest_id (quest_id),
    KEY idx_character_quests_status (status),
    CONSTRAINT fk_character_quests_character_id
        FOREIGN KEY (character_id) REFERENCES characters(character_id)
        ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT fk_character_quests_quest_id
        FOREIGN KEY (quest_id) REFERENCES quests(quest_id)
        ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
