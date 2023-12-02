/*
** The sequence table is used for generating unique integers. Please see
** biblioteq_misc_functions::getSqliteUniqueId().
*/

const char *sqlite_create_schema_text = "\
CREATE TABLE book							\
(									\
    accession_number TEXT,						\
    alternate_id_1 TEXT,						\
    author       TEXT NOT NULL,						\
    back_cover	 BYTEA,							\
    binding_type VARCHAR(32) NOT NULL,					\
    book_read    INTEGER DEFAULT 0,					\
    callnumber	 VARCHAR(64),						\
    category	 TEXT NOT NULL,						\
    condition    TEXT,							\
    description	 TEXT NOT NULL,						\
    deweynumber	 VARCHAR(64),						\
    edition	 VARCHAR(8) NOT NULL,					\
    front_cover	 BYTEA,							\
    id		 VARCHAR(32) UNIQUE,					\
    isbn13	 VARCHAR(32) UNIQUE,					\
    keyword      TEXT,							\
    language	 VARCHAR(64) NOT NULL DEFAULT 'UNKNOWN',		\
    lccontrolnumber	 VARCHAR(64),					\
    location	 TEXT NOT NULL,						\
    marc_tags    TEXT,							\
    monetary_units	 VARCHAR(64) NOT NULL DEFAULT 'UNKNOWN',	\
    multivolume_set_isbn VARCHAR(32) UNIQUE,				\
    myoid	 BIGINT NOT NULL,					\
    originality  TEXT,							\
    pdate	 VARCHAR(32) NOT NULL,					\
    place	 TEXT NOT NULL,						\
    price	 NUMERIC(10, 2) NOT NULL DEFAULT 0.00,			\
    publisher	 TEXT NOT NULL,						\
    quantity	 INTEGER NOT NULL DEFAULT 1,				\
    title	 TEXT NOT NULL,						\
    type	 VARCHAR(16) NOT NULL DEFAULT 'Book',			\
    url          TEXT							\
);									\
									\
CREATE TABLE book_copy_info						\
(									\
    copy_number	 INTEGER NOT NULL DEFAULT 1,				\
    copyid	 VARCHAR(64) NOT NULL,					\
    myoid	 BIGINT NOT NULL,					\
    condition    TEXT,							\
    item_oid	 BIGINT NOT NULL,					\
    originality  TEXT,							\
    status       TEXT,							\
    PRIMARY KEY(item_oid, copyid),					\
    FOREIGN KEY(item_oid) REFERENCES book(myoid) ON DELETE CASCADE	\
);									\
									\
CREATE TABLE book_files							\
(									\
    description	TEXT,							\
    file	BYTEA NOT NULL,						\
    file_digest	TEXT NOT NULL,						\
    file_name   TEXT NOT NULL,						\
    item_oid	BIGINT NOT NULL,					\
    myoid	BIGINT NOT NULL,					\
    FOREIGN KEY(item_oid) REFERENCES book(myoid) ON DELETE CASCADE,	\
    PRIMARY KEY(file_digest, item_oid)					\
);									\
									\
CREATE TABLE book_sequence						\
(									\
    value            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT		\
);									\
									\
CREATE TABLE photograph_collection					\
(									\
    about	 TEXT,							\
    accession_number TEXT,						\
    id		 TEXT PRIMARY KEY NOT NULL,				\
    image	 BYTEA,							\
    image_scaled BYTEA,							\
    location     TEXT NOT NULL,						\
    myoid	 BIGINT NOT NULL,					\
    notes	 TEXT,							\
    title	 TEXT NOT NULL,						\
    type	 VARCHAR(32) NOT NULL DEFAULT 'Photograph Collection'	\
);									\
									\
CREATE TABLE photograph							\
(									\
    accession_number      TEXT, 					\
    callnumber		  VARCHAR(64),					\
    collection_oid	  BIGINT NOT NULL,				\
    copyright		  TEXT NOT NULL,				\
    creators		  TEXT NOT NULL,				\
    format		  TEXT,						\
    id                    TEXT NOT NULL,				\
    image		  BYTEA,					\
    image_scaled	  BYTEA,					\
    medium		  TEXT NOT NULL,				\
    myoid		  BIGINT NOT NULL,				\
    notes		  TEXT,						\
    other_number	  TEXT,						\
    pdate		  VARCHAR(32) NOT NULL,				\
    quantity		  INTEGER NOT NULL DEFAULT 1,			\
    reproduction_number   TEXT NOT NULL,				\
    subjects		  TEXT,						\
    title		  TEXT NOT NULL,				\
    PRIMARY KEY(id, collection_oid),					\
    FOREIGN KEY(collection_oid) REFERENCES				\
                                photograph_collection(myoid) ON		\
				DELETE CASCADE				\
);									\
									\
CREATE TRIGGER book_purge_trigger AFTER DELETE ON book			\
FOR EACH row								\
BEGIN									\
    DELETE FROM book_copy_info WHERE item_oid = old.myoid;		\
    DELETE FROM item_borrower WHERE item_oid = old.myoid;		\
    DELETE FROM member_history WHERE item_oid = old.myoid AND		\
                type = old.type;					\
END;									\
									\
CREATE TABLE book_binding_types						\
(									\
    binding_type     TEXT NOT NULL PRIMARY KEY				\
);									\
									\
CREATE TABLE languages							\
(									\
    language	 TEXT NOT NULL PRIMARY KEY				\
);      								\
									\
CREATE TABLE locations				                        \
(									\
    location	 TEXT NOT NULL,						\
    type	 VARCHAR(32) NOT NULL,					\
    PRIMARY KEY(location, type)						\
);      								\
									\
CREATE TABLE minimum_days						\
(									\
    days		 INTEGER NOT NULL,                              \
    type		 VARCHAR(16) NOT NULL PRIMARY KEY               \
);                                                                      \
									\
CREATE TABLE monetary_units						\
(									\
    monetary_unit	 TEXT NOT NULL PRIMARY KEY                      \
);                                                                      \
									\
CREATE TABLE sequence							\
(									\
    value            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT		\
);									\
									\
";
