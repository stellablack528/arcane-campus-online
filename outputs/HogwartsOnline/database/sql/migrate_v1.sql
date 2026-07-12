USE arcane_campus_online;

CREATE TABLE IF NOT EXISTS schema_migrations (
    version VARCHAR(32) NOT NULL,
    applied_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (version)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT IGNORE INTO schema_migrations (version)
VALUES ('v1_database_infrastructure');
