USE arcane_campus_online;

INSERT INTO users (user_id, username, password_hash, email, avatar, status)
VALUES
    (1, 'raven_test', '$argon2id$v=19$m=65536,t=3,p=4$raven_seed$hash_placeholder_001', 'raven.test@example.com', 'avatars/raven_test.png', 'active'),
    (2, 'gryffin_test', '$argon2id$v=19$m=65536,t=3,p=4$gryffin_seed$hash_placeholder_002', 'gryffin.test@example.com', 'avatars/gryffin_test.png', 'active'),
    (3, 'slyther_test', '$argon2id$v=19$m=65536,t=3,p=4$slyther_seed$hash_placeholder_003', 'slyther.test@example.com', 'avatars/slyther_test.png', 'active')
ON DUPLICATE KEY UPDATE
    password_hash = VALUES(password_hash),
    email = VALUES(email),
    avatar = VALUES(avatar),
    status = VALUES(status);

INSERT INTO rooms (room_id, room_name, room_type, description, max_players)
VALUES
    (1, 'Great Hall', 'public', 'Long tables, floating candles, breakfast, announcements, and the pulse of daily campus life.', 300),
    (2, 'Library', 'study', 'Quiet shelves, parchment notes, study groups, and careful whispers.', 120),
    (3, 'Astronomy Tower', 'exploration', 'A high tower for stargazing, late conversations, and night events.', 80),
    (4, 'Gryffindor Common Room', 'house_common_room', 'A warm red-and-gold common room for Gryffindor students.', 80),
    (5, 'Slytherin Common Room', 'house_common_room', 'A green-lit stone common room beneath the lake.', 80),
    (6, 'Hufflepuff Common Room', 'house_common_room', 'A cozy common room near the kitchens with plants and round doors.', 80),
    (7, 'Ravenclaw Common Room', 'house_common_room', 'An airy tower room with blue accents, books, and riddles.', 80)
ON DUPLICATE KEY UPDATE
    room_type = VALUES(room_type),
    description = VALUES(description),
    max_players = VALUES(max_players);

INSERT INTO characters
    (character_id, user_id, nickname, house, level, experience, gold, current_room_id, title)
VALUES
    (1, 1, 'Aster Quill', 'Ravenclaw', 5, 1280, 42, 2, 'Library Regular'),
    (2, 2, 'Rowan Brave', 'Gryffindor', 5, 1150, 36, 1, 'Breakfast Champion'),
    (3, 3, 'Selene Vale', 'Slytherin', 5, 1190, 58, 5, 'Potion Club Member'),
    (4, 1, 'Mira Hearth', 'Hufflepuff', 4, 940, 31, 6, 'Greenhouse Helper')
ON DUPLICATE KEY UPDATE
    user_id = VALUES(user_id),
    house = VALUES(house),
    level = VALUES(level),
    experience = VALUES(experience),
    gold = VALUES(gold),
    current_room_id = VALUES(current_room_id),
    title = VALUES(title);

INSERT INTO friendships (user_id, friend_id, relation_status)
VALUES
    (1, 2, 'accepted'),
    (2, 1, 'accepted'),
    (1, 3, 'pending')
ON DUPLICATE KEY UPDATE
    relation_status = VALUES(relation_status);

INSERT INTO npcs (npc_id, npc_name, room_id, description, personality)
VALUES
    (1, 'Professor McGonagall', 1, 'A strict professor watching the morning announcements.', 'Precise, stern, fair, and deeply protective of students.'),
    (2, 'Hermione Granger', 2, 'A brilliant student surrounded by open books.', 'Studious, direct, helpful, and impatient with sloppy preparation.'),
    (3, 'Gareth Weasley', 1, 'A cheerful student looking for someone to share breakfast gossip with.', 'Curious, experimental, warm, and easily distracted by snacks.'),
    (4, 'The Grey Lady', 7, 'A quiet ghost drifting near the tall windows.', 'Reserved, poetic, observant, and fond of riddles.'),
    (5, 'Wand Shop Clerk', 1, 'A visiting clerk arranging wand-care kits on a small table.', 'Polite, attentive, and obsessed with wand maintenance.')
ON DUPLICATE KEY UPDATE
    room_id = VALUES(room_id),
    description = VALUES(description),
    personality = VALUES(personality);

INSERT INTO items (item_id, item_name, item_type, description, stackable)
VALUES
    (1, 'Wand', 'equipment', 'A simple student wand used for class and daily campus interactions.', FALSE),
    (2, 'Chocolate Frog', 'food', 'A lively sweet wrapped in blue foil.', TRUE),
    (3, 'Butterbeer', 'drink', 'A warm drink from Hogsmeade.', TRUE),
    (4, 'Potion', 'consumable', 'A small bottle of shimmering potion.', TRUE),
    (5, 'Quill', 'tool', 'Useful for class notes and letters.', FALSE),
    (6, 'Galleon Pouch', 'currency', 'A small pouch used to carry wizarding currency.', FALSE)
ON DUPLICATE KEY UPDATE
    item_type = VALUES(item_type),
    description = VALUES(description),
    stackable = VALUES(stackable);

INSERT INTO inventories (character_id, item_id, quantity)
VALUES
    (1, 1, 1),
    (1, 2, 3),
    (1, 5, 1),
    (2, 1, 1),
    (2, 3, 2),
    (3, 1, 1),
    (3, 4, 2),
    (4, 1, 1),
    (4, 2, 1)
ON DUPLICATE KEY UPDATE
    quantity = VALUES(quantity);

INSERT INTO quests (quest_id, quest_name, description, reward_gold, reward_exp)
VALUES
    (1, 'Breakfast Roll Call', 'Visit the Great Hall during breakfast time.', 5, 30),
    (2, 'Library Study Notes', 'Spend time studying in the Library.', 8, 50),
    (3, 'Astronomy Observation', 'Go to the Astronomy Tower for a night observation.', 10, 70)
ON DUPLICATE KEY UPDATE
    description = VALUES(description),
    reward_gold = VALUES(reward_gold),
    reward_exp = VALUES(reward_exp);

INSERT INTO character_quests (character_id, quest_id, progress, status)
VALUES
    (1, 1, 100, 'completed'),
    (1, 2, 40, 'in_progress'),
    (2, 1, 60, 'in_progress'),
    (3, 3, 0, 'not_started')
ON DUPLICATE KEY UPDATE
    progress = VALUES(progress),
    status = VALUES(status);

INSERT INTO chat_messages (sender_character_id, room_id, content, send_time)
VALUES
    (2, 1, 'Breakfast just opened in the Great Hall.', '2026-07-10 08:30:00'),
    (1, 2, 'The Library is quiet this morning.', '2026-07-10 08:37:00'),
    (3, 5, 'Potion club notes are on the table.', '2026-07-10 08:42:00');
