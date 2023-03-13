CREATE TABLE IF NOT EXISTS "main" (
        `keys`  VARCHAR ( 5 ) NOT NULL,
        `ch`    TEXT NOT NULL,
        `cat`   INTEGER NOT NULL,
        `cnt`   INTEGER
);
CREATE INDEX `keys_index_main` ON `main` (
        `keys`
);
CREATE TABLE IF NOT EXISTS "simple" (
        `keys`  TEXT NOT NULL,
        `ch`    TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS "phrase" (
        `keys`  VARCHAR ( 4 ) NOT NULL,
        `ph`    TEXT NOT NULL
);