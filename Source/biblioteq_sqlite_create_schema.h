/*
** The sequence table is used for generating unique integers. Please see
** biblioteq_misc_functions::getSqliteUniqueId().
*/

const char *sqlite_create_schema_text = "\
CREATE TABLE book \
( \
    accession_number        TEXT, \
    alternate_id_1          TEXT, \
    author                  TEXT NOT NULL, \
    back_cover	            BYTEA, \
    binding_type            VARCHAR(32) NOT NULL, \
    book_read               INTEGER DEFAULT 0, \
    callnumber	            VARCHAR(64), \
    category	            TEXT NOT NULL, \
    condition               TEXT, \
    description	            TEXT NOT NULL, \
    deweynumber	            VARCHAR(64), \
    edition	                VARCHAR(8) NOT NULL, \
    front_cover	            BYTEA, \
    id		                VARCHAR(32) UNIQUE, \
    isbn13	                VARCHAR(32) UNIQUE, \
    keyword                 TEXT, \
    lccontrolnumber	        VARCHAR(64), \
    marc_tags               TEXT, \
    multivolume_set_isbn    VARCHAR(32) UNIQUE, \
    myoid	                BIGINT NOT NULL, \
    originality             TEXT, \
    pdate	                VARCHAR(32) NOT NULL, \
    place	                TEXT NOT NULL, \
    publisher	            TEXT NOT NULL, \
    quantity	            INTEGER NOT NULL DEFAULT 1, \
    title	                TEXT NOT NULL, \
    type	                VARCHAR(16) NOT NULL DEFAULT 'Book', \
    url                     TEXT \
);									\
 \
CREATE TABLE book_copy_info	 \
( \
    copy_number	    INTEGER NOT NULL DEFAULT 1, \
    copyid	        VARCHAR(64) NOT NULL, \
    myoid	        BIGINT NOT NULL, \
    condition       TEXT, \
    item_oid	    BIGINT NOT NULL, \
    originality     TEXT, \
    status          TEXT, \
    PRIMARY KEY(item_oid, copyid), \
    FOREIGN KEY(item_oid) REFERENCES book(myoid) ON DELETE CASCADE	\
);									\
 \
CREATE TABLE book_files	\
( \
    description	TEXT,\
    file	    BYTEA NOT NULL,\
    file_digest	TEXT NOT NULL,\
    file_name   TEXT NOT NULL,\
    item_oid	BIGINT NOT NULL,\
    myoid	    BIGINT NOT NULL,\
    FOREIGN KEY(item_oid) REFERENCES book(myoid) ON DELETE CASCADE,	\
    PRIMARY KEY(file_digest, item_oid)\
);\
 \
CREATE TABLE book_sequence \
(\
    value INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT	\
);\
 \
CREATE TABLE photograph_collection \
( \
    about	            TEXT DEFAULT '', \
    accession_number    TEXT DEFAULT '', \
    id		            TEXT PRIMARY KEY NOT NULL, \
    image	            BYTEA, \
    image_scaled        BYTEA, \
    myoid	            BIGINT NOT NULL, \
    notes	            TEXT DEFAULT '', \
    title	            TEXT NOT NULL, \
    creation_date       TEXT DEFAULT '', \
    circulation_height  TEXT DEFAULT '', \
    total_number        TEXT DEFAULT '', \
    by_artist           TEXT DEFAULT '', \
    publisher           TEXT DEFAULT '', \
    keywords            TEXT DEFAULT '', \
    type	            VARCHAR(32) NOT NULL DEFAULT 'Photograph Collection' \
); \
 \
CREATE TABLE photograph \
( \
    accession_number        TEXT, \
    callnumber		        VARCHAR(64), \
    collection_oid	        BIGINT NOT NULL, \
    copyright		        TEXT, \
    creators		        TEXT NOT NULL DEFAULT '', \
    format		            TEXT, \
    id                      TEXT NOT NULL, \
    image		            BYTEA, \
    image_scaled	        BYTEA, \
    image_original          BYTEA, \
    medium		            TEXT, \
    myoid		            BIGINT NOT NULL, \
    notes		            TEXT, \
    other_number	        TEXT, \
    pdate		            VARCHAR(32), \
    quantity		        INTEGER DEFAULT 1, \
    reproduction_number     TEXT, \
    subjects		        TEXT, \
    title		            TEXT NOT NULL DEFAULT '', \
    inventor_old            TEXT DEFAULT '', \
    inventor_new            TEXT DEFAULT '', \
    based_on_artist         TEXT DEFAULT '', \
    printer                 TEXT DEFAULT '', \
    title_original_picture  TEXT DEFAULT '', \
    inventory_number        TEXT DEFAULT '', \
    delivery_number         TEXT DEFAULT '', \
    page_number             TEXT DEFAULT '', \
    material                TEXT DEFAULT '', \
    signed                  TEXT DEFAULT '', \
    catalogue               TEXT DEFAULT '', \
    place_of_storage        TEXT DEFAULT '', \
    executing_artist        TEXT DEFAULT '', \
    technique               TEXT DEFAULT '', \
    creation_date           VARCHAR(32), \
    keywords                TEXT DEFAULT '', \
    creation_date_original  VARCHAR(32), \
    title_old               TEXT DEFAULT '', \
    PRIMARY KEY(id, collection_oid), \
    FOREIGN KEY(collection_oid) REFERENCES \
                                photograph_collection(myoid) ON \
				DELETE CASCADE \
); \
 \
CREATE TRIGGER book_purge_trigger AFTER DELETE ON book \
FOR EACH row \
BEGIN \
    DELETE FROM book_copy_info WHERE item_oid = old.myoid; \
END; \
 \
CREATE TABLE book_binding_types \
( \
    binding_type     TEXT NOT NULL PRIMARY KEY \
); \
\
CREATE TABLE sequence \
( \
    value  INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT \
);";
