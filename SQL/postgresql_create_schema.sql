CREATE USER xbook_admin ENCRYPTED PASSWORD 'xbook_admin' CREATEROLE;

CREATE SEQUENCE book_sequence START 1;

CREATE TABLE admin
(
	roles		 TEXT NOT NULL,
	username	 VARCHAR(128) NOT NULL PRIMARY KEY
);

CREATE TABLE book
(
	accession_number     TEXT,
	alternate_id_1	     TEXT,
	author		     TEXT NOT NULL,
	back_cover	     BYTEA,
	binding_type	     VARCHAR(32) NOT NULL,
	callnumber	     VARCHAR(64),
	category	     TEXT NOT NULL,
	condition 	     TEXT,
	description	     TEXT NOT NULL,
	deweynumber	     VARCHAR(64),
	edition		     VARCHAR(8) NOT NULL,
	front_cover	     BYTEA,
	id		     VARCHAR(32) UNIQUE,
	isbn13		     VARCHAR(32) UNIQUE,
	keyword		     TEXT,
	language	     VARCHAR(64) NOT NULL DEFAULT 'UNKNOWN',
	lccontrolnumber      VARCHAR(64),
	location	     TEXT NOT NULL,
	marc_tags            TEXT,
	monetary_units	     VARCHAR(64) NOT NULL DEFAULT 'UNKNOWN',
	multivolume_set_isbn VARCHAR(32) UNIQUE,
	myoid		     BIGSERIAL UNIQUE,
	originality 	     TEXT,
	pdate		     VARCHAR(32) NOT NULL,
	place		     TEXT NOT NULL,
	price		     NUMERIC(10, 2) NOT NULL DEFAULT 0.00,
	publisher	     TEXT NOT NULL,
	quantity	     INTEGER NOT NULL DEFAULT 1,
	title		     TEXT NOT NULL,
	type		     VARCHAR(16) NOT NULL DEFAULT 'Book',
	url		     TEXT
);

CREATE TABLE book_copy_info
(
	condition	 TEXT,
	copy_number	 INTEGER NOT NULL DEFAULT 1,
	copyid		 VARCHAR(64) NOT NULL,
	item_oid	 BIGINT NOT NULL,
	myoid		 BIGSERIAL UNIQUE,
	originality	 TEXT,
	status		 TEXT,
	FOREIGN KEY (item_oid) REFERENCES book (myoid) ON DELETE CASCADE,
	PRIMARY KEY (item_oid, copyid)
);

CREATE TABLE book_files
(
	description	TEXT,
	file		BYTEA NOT NULL,
	file_digest	TEXT NOT NULL,
	file_name	TEXT NOT NULL,
	item_oid	BIGINT NOT NULL,
	myoid		BIGSERIAL NOT NULL,
	FOREIGN KEY (item_oid) REFERENCES book (myoid) ON DELETE CASCADE,
	PRIMARY KEY (file_digest, item_oid)
);

CREATE TABLE grey_literature
(
	author		TEXT NOT NULL,
	client		TEXT,
	document_code_a	TEXT NOT NULL,
	document_code_b TEXT NOT NULL,
	document_date	TEXT NOT NULL,
	document_id	TEXT NOT NULL PRIMARY KEY,
	document_status TEXT,
	document_title	TEXT NOT NULL,
	document_type	TEXT NOT NULL,
	front_cover     BYTEA,
	job_number	TEXT NOT NULL,
	location	TEXT,
	myoid		BIGSERIAL UNIQUE,
	notes		TEXT,
	quantity	 INTEGER NOT NULL DEFAULT 1,
	type		VARCHAR(16) NOT NULL DEFAULT 'Grey Literature'
);

CREATE TABLE grey_literature_files
(
	description	TEXT,
	file		BYTEA NOT NULL,
	file_digest	TEXT NOT NULL,
	file_name	TEXT NOT NULL,
	item_oid	BIGINT NOT NULL,
	myoid		BIGSERIAL NOT NULL,
	FOREIGN KEY (item_oid) REFERENCES grey_literature (myoid)
		    	       ON DELETE CASCADE,
	PRIMARY KEY (file_digest, item_oid)
);

CREATE TABLE journal
(
	accession_number TEXT,
	back_cover	 BYTEA,
	callnumber	 VARCHAR(64),
	category	 TEXT NOT NULL,
	description	 TEXT NOT NULL,
	deweynumber	 VARCHAR(64),
	front_cover	 BYTEA,
	id		 VARCHAR(32),
	issueno		 INTEGER NOT NULL DEFAULT 0,
	issuevolume	 INTEGER NOT NULL DEFAULT 0,
	keyword		 TEXT,
	language	 VARCHAR(64) NOT NULL DEFAULT 'UNKNOWN',
	lccontrolnumber	 VARCHAR(64),
	location	 TEXT NOT NULL,
	marc_tags	 TEXT,
	monetary_units	 VARCHAR(64) NOT NULL DEFAULT 'UNKNOWN',
	myoid		 BIGSERIAL UNIQUE,
	pdate		 VARCHAR(32) NOT NULL,
	place		 TEXT NOT NULL,
	price		 NUMERIC(10, 2) NOT NULL DEFAULT 0.00,
	publisher	 TEXT NOT NULL,
	quantity	 INTEGER NOT NULL DEFAULT 1,
	title		 TEXT NOT NULL,
	type		 VARCHAR(16) NOT NULL DEFAULT 'Journal',
	UNIQUE (id, issuevolume, issueno)
);

CREATE TABLE journal_copy_info
(
	copy_number	 INTEGER NOT NULL DEFAULT 1,
	copyid		 VARCHAR(64) NOT NULL,
	item_oid	 BIGINT NOT NULL,
	myoid		 BIGSERIAL UNIQUE,
	status		 TEXT,
	FOREIGN KEY (item_oid) REFERENCES journal (myoid) ON DELETE CASCADE,
	PRIMARY KEY (item_oid, copyid)
);

CREATE TABLE journal_files
(
	description	TEXT,
	file		BYTEA NOT NULL,
	file_digest	TEXT NOT NULL,
	file_name	TEXT NOT NULL,
	item_oid	BIGINT NOT NULL,
	myoid		BIGSERIAL NOT NULL,
	FOREIGN KEY (item_oid) REFERENCES journal (myoid) ON DELETE CASCADE,
	PRIMARY KEY (file_digest, item_oid)
);

CREATE TABLE magazine
(
	accession_number TEXT,
	back_cover	 BYTEA,
	callnumber	 VARCHAR(64),
	category	 TEXT NOT NULL,
	description	 TEXT NOT NULL,
	deweynumber	 VARCHAR(64),
	front_cover	 BYTEA,
	id		 VARCHAR(32),
	issueno		 INTEGER NOT NULL DEFAULT 0,
	issuevolume	 INTEGER NOT NULL DEFAULT 0,
	keyword		 TEXT,
	language	 VARCHAR(64) NOT NULL DEFAULT 'UNKNOWN',
	lccontrolnumber	 VARCHAR(64),
	location	 TEXT NOT NULL,
	marc_tags	 TEXT,
	monetary_units	 VARCHAR(64) NOT NULL DEFAULT 'UNKNOWN',
	myoid		 BIGSERIAL UNIQUE,
	pdate		 VARCHAR(32) NOT NULL,
	place		 TEXT NOT NULL,
	price		 NUMERIC(10, 2) NOT NULL DEFAULT 0.00,
	publisher	 TEXT NOT NULL,
	quantity	 INTEGER NOT NULL DEFAULT 1,
	title		 TEXT NOT NULL,
	type		 VARCHAR(16) NOT NULL DEFAULT 'Magazine',
	UNIQUE (id, issuevolume, issueno)
);

CREATE TABLE magazine_copy_info
(
	copy_number	 INTEGER NOT NULL DEFAULT 1,
	copyid		 VARCHAR(64) NOT NULL,
	item_oid	 BIGINT NOT NULL,
	myoid		 BIGSERIAL UNIQUE,
	status		 TEXT,
	FOREIGN KEY (item_oid) REFERENCES magazine (myoid) ON DELETE CASCADE,
	PRIMARY KEY (item_oid, copyid)
);

CREATE TABLE magazine_files
(
	description	TEXT,
	file		BYTEA NOT NULL,
	file_digest	TEXT NOT NULL,
	file_name	TEXT NOT NULL,
	item_oid	BIGINT NOT NULL,
	myoid		BIGSERIAL NOT NULL,
	FOREIGN KEY (item_oid) REFERENCES magazine (myoid) ON DELETE CASCADE,
	PRIMARY KEY (file_digest, item_oid)
);

CREATE TABLE photograph_collection
(
        id                  TEXT PRIMARY KEY NOT NULL,
        accesion_number     TEXT,
        about		    TEXT,
        location	    TEXT NOT NULL,
        image               BYTEA,
        image_scaled        BYTEA,
        myoid               BIGSERIAL UNIQUE,
        notes               TEXT,
        title               TEXT NOT NULL,
        creation_date       TEXT,
        circulation_height  TEXT,
        total_number        TEXT,
        by_artist           TEXT,
        publisher           TEXT,
        keywords            TEXT,
        type		    VARCHAR(32) NOT NULL DEFAULT 'Photograph Collection'
);

CREATE TABLE photograph
(
	accession_number TEXT,
	callnumber		  VARCHAR(64),
	collection_oid		  BIGINT NOT NULL,
        inventar_old		  TEXT NOT NULL,
        executing_artis		  TEXT NOT NULL,
	format			  TEXT,
	id			  TEXT NOT NULL,
	image			  BYTEA,
	image_scaled		  BYTEA,
        technique		  TEXT NOT NULL,
	myoid			  BIGSERIAL UNIQUE,
        ohter_number		  TEXT,
        notes   		  TEXT,
        creation_date		  VARCHAR(32) NOT NULL,
	quantity		  INTEGER NOT NULL DEFAULT 1,
	reproduction_number  	  TEXT NOT NULL,
        keywords		  TEXT,
	title			  TEXT NOT NULL,
        medium                    TEXT NOT NULL,
        pdate			  VARCHAR(32) NOT NULL,
        creation_date_original    VARCHAR(32) NOT NULL,
        creators                  TEXT NOT NULL,
        copyright                   TEXT NOT NULL,
        title_old                   TEXT NOT NULL,
        inventor_new                TEXT NOT NULL,
        based_on_artist             TEXT NOT NULL,
        printer                     TEXT NOT NULL,
        title_original_picture      TEXT NOT NULL,
        inventory_number            TEXT NOT NULL,
        delivery_number             TEXT NOT NULL,
        page_number                 TEXT NOT NULL,
        material                    TEXT NOT NULL,
        signed                      TEXT NOT NULL,
        subjects                    TEXT NOT NULL,
        catalogue                   TEXT NOT NULL,
        place_of_storage            TEXT NOT NULL,
	FOREIGN KEY (collection_oid) REFERENCES photograph_collection (myoid)
		    		     ON DELETE CASCADE,
	PRIMARY KEY (id, collection_oid)
);

CREATE TABLE member
(
	city		 VARCHAR(256) NOT NULL,
	comments	 TEXT,
	dob		 VARCHAR(32) NOT NULL,
	email		 VARCHAR(128),
	expiration_date	 VARCHAR(32) NOT NULL,
	first_name	 VARCHAR(128) NOT NULL,
	general_registration_number	 TEXT,
	last_name	 VARCHAR(128) NOT NULL,
	maximum_reserved_books INTEGER NOT NULL DEFAULT 0,
	memberclass	 TEXT,
	memberid	 VARCHAR(16) NOT NULL PRIMARY KEY,
	membership_fees	 NUMERIC(10, 2) NOT NULL DEFAULT 0.00,
	membersince	 VARCHAR(32) NOT NULL,
	middle_init	 VARCHAR(1),
	overdue_fees	 NUMERIC(10, 2) NOT NULL DEFAULT 0.00,
	sex		 VARCHAR(32) NOT NULL DEFAULT 'Private',
	state_abbr	 VARCHAR(16) NOT NULL DEFAULT 'N/A',
	street		 VARCHAR(256) NOT NULL,
	telephone_num	 VARCHAR(32),
	zip		 VARCHAR(16) NOT NULL DEFAULT 'N/A'
);

CREATE TABLE member_history
(
	copyid		 VARCHAR(64) NOT NULL,
	duedate		 VARCHAR(32) NOT NULL,
	item_oid	 BIGINT NOT NULL,
	memberid	 VARCHAR(16) NOT NULL,
	myoid		 BIGSERIAL PRIMARY KEY,
	reserved_by	 VARCHAR(128) NOT NULL,
	reserved_date	 VARCHAR(32) NOT NULL,
	returned_date	 VARCHAR(32) NOT NULL,
	type		 VARCHAR(16) NOT NULL,
	FOREIGN KEY (memberid) REFERENCES member (memberid) ON DELETE CASCADE
);

CREATE TABLE member_history_dnt
(
	dnt		INTEGER NOT NULL DEFAULT 1,
	memberid	VARCHAR(16) NOT NULL PRIMARY KEY,
	FOREIGN KEY (memberid) REFERENCES member (memberid) ON DELETE CASCADE
);

CREATE TABLE item_borrower
(
	copy_number	 INTEGER NOT NULL DEFAULT 1,
	copyid		 VARCHAR(64) NOT NULL,
	duedate		 VARCHAR(32) NOT NULL,
	item_oid	 BIGINT NOT NULL,
	memberid	 VARCHAR(16) NOT NULL,
	myoid		 BIGSERIAL PRIMARY KEY,
	reserved_by	 VARCHAR(128) NOT NULL,
	reserved_date	 VARCHAR(32) NOT NULL,
	type		 VARCHAR(16) NOT NULL,
	FOREIGN KEY (memberid) REFERENCES member ON DELETE RESTRICT
);

CREATE TABLE item_request
(
	item_oid	 BIGINT NOT NULL,
	memberid	 VARCHAR(16) NOT NULL,
	myoid		 BIGSERIAL NOT NULL,
	requestdate	 VARCHAR(32) NOT NULL,
	type		 VARCHAR(16) NOT NULL,
	PRIMARY KEY (item_oid, memberid, type),
	FOREIGN KEY (memberid) REFERENCES member (memberid) ON DELETE CASCADE
);

CREATE OR REPLACE FUNCTION delete_book() RETURNS trigger AS '
BEGIN
	DELETE FROM item_borrower WHERE item_oid = old.myoid;
	DELETE FROM member_history WHERE item_oid = old.myoid AND
	type = ''Book'';
	RETURN NULL;
END;
' LANGUAGE plpgsql;
CREATE TRIGGER book_trigger AFTER DELETE ON book
FOR EACH row EXECUTE PROCEDURE delete_book();

CREATE OR REPLACE FUNCTION delete_grey_literature() RETURNS trigger AS '
BEGIN
	DELETE FROM item_borrower WHERE item_oid = old.myoid;
	DELETE FROM member_history WHERE item_oid = old.myoid AND
	type = ''Grey Literature'';
	RETURN NULL;
END;
' LANGUAGE plpgsql;
CREATE TRIGGER grey_literature_trigger AFTER DELETE ON grey_literature
FOR EACH row EXECUTE PROCEDURE delete_grey_literature();

CREATE OR REPLACE FUNCTION delete_journal() RETURNS trigger AS '
BEGIN
	DELETE FROM item_borrower WHERE item_oid = old.myoid;
	DELETE FROM member_history WHERE item_oid = old.myoid AND
	type = ''Journal'';
	RETURN NULL;
END;
' LANGUAGE 'plpgsql';
CREATE TRIGGER journal_trigger AFTER DELETE ON journal
FOR EACH row EXECUTE PROCEDURE delete_journal();

CREATE OR REPLACE FUNCTION delete_magazine() RETURNS trigger AS '
BEGIN
	DELETE FROM item_borrower WHERE item_oid = old.myoid;
	DELETE FROM member_history WHERE item_oid = old.myoid AND
	type = ''Magazine'';
	RETURN NULL;
END;
' LANGUAGE 'plpgsql';
CREATE TRIGGER magazine_trigger AFTER DELETE ON magazine
FOR EACH row EXECUTE PROCEDURE delete_magazine();

CREATE TABLE book_binding_types
(
	binding_type	 TEXT NOT NULL PRIMARY KEY
);

CREATE TABLE grey_literature_types
(
	document_type	 TEXT NOT NULL PRIMARY KEY
);

CREATE TABLE languages
(
	language	 TEXT NOT NULL PRIMARY KEY
);

CREATE TABLE locations
(
	location	 TEXT NOT NULL,
	type		 VARCHAR(32) NOT NULL,
	PRIMARY KEY (location, type)
);

CREATE TABLE monetary_units
(
	monetary_unit	 TEXT NOT NULL PRIMARY KEY
);

CREATE TABLE minimum_days
(
	days		 INTEGER NOT NULL,
	type		 VARCHAR(16) NOT NULL PRIMARY KEY
);

CREATE ROLE biblioteq_administrator INHERIT SUPERUSER;
CREATE ROLE biblioteq_circulation INHERIT;
CREATE ROLE biblioteq_circulation_librarian INHERIT;
CREATE ROLE biblioteq_circulation_librarian_membership INHERIT;
CREATE ROLE biblioteq_circulation_membership INHERIT;
CREATE ROLE biblioteq_guest NOINHERIT;
CREATE ROLE biblioteq_librarian INHERIT;
CREATE ROLE biblioteq_librarian_membership INHERIT;
CREATE ROLE biblioteq_membership CREATEROLE INHERIT;
CREATE ROLE biblioteq_patron NOINHERIT;
GRANT DELETE ON member_history_dnt TO biblioteq_membership;
GRANT DELETE, INSERT, SELECT ON item_request TO biblioteq_patron;
GRANT DELETE, INSERT, SELECT, UPDATE ON admin TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON book TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON book TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON book_binding_types TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON book_binding_types TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON book_copy_info TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON book_copy_info TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON book_files TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON book_files TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON grey_literature TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON grey_literature TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON grey_literature_files TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON grey_literature_files TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON grey_literature_types TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON grey_literature_types TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON item_borrower TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON item_borrower TO biblioteq_circulation;
GRANT DELETE, INSERT, SELECT, UPDATE ON journal TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON journal TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON journal_copy_info TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON journal_copy_info TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON journal_files TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON journal_files TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON languages TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON languages TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON locations TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON locations TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON magazine TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON magazine TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON magazine_copy_info TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON magazine_copy_info TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON magazine_files TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON magazine_files TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON member TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON member TO biblioteq_membership;
GRANT DELETE, INSERT, SELECT, UPDATE ON member_history TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON minimum_days TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON minimum_days TO biblioteq_circulation;
GRANT DELETE, INSERT, SELECT, UPDATE ON minimum_days TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON monetary_units TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON monetary_units TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON photograph TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON photograph TO biblioteq_librarian;
GRANT DELETE, INSERT, SELECT, UPDATE ON photograph_collection TO biblioteq_administrator;
GRANT DELETE, INSERT, SELECT, UPDATE ON photograph_collection TO biblioteq_librarian;
GRANT DELETE, SELECT ON item_request TO biblioteq_administrator;
GRANT DELETE, SELECT ON item_request TO biblioteq_circulation;
GRANT DELETE, SELECT ON member_history TO biblioteq_librarian;
GRANT DELETE, SELECT ON member_history_dnt TO biblioteq_administrator;
GRANT INSERT, SELECT, UPDATE ON member_history TO biblioteq_circulation;
GRANT INSERT, SELECT, UPDATE ON member_history_dnt TO biblioteq_patron;
GRANT SELECT (item_oid, type) ON item_borrower TO biblioteq_guest;
GRANT SELECT ON admin TO biblioteq_circulation;
GRANT SELECT ON admin TO biblioteq_librarian;
GRANT SELECT ON admin TO biblioteq_membership;
GRANT SELECT ON book TO biblioteq_circulation;
GRANT SELECT ON book TO biblioteq_guest;
GRANT SELECT ON book TO biblioteq_membership;
GRANT SELECT ON book TO biblioteq_patron;
GRANT SELECT ON book_binding_types TO biblioteq_circulation;
GRANT SELECT ON book_binding_types TO biblioteq_guest;
GRANT SELECT ON book_binding_types TO biblioteq_membership;
GRANT SELECT ON book_binding_types TO biblioteq_patron;
GRANT SELECT ON book_copy_info TO biblioteq_circulation;
GRANT SELECT ON book_copy_info TO biblioteq_guest;
GRANT SELECT ON book_copy_info TO biblioteq_membership;
GRANT SELECT ON book_copy_info TO biblioteq_patron;
GRANT SELECT ON book_copy_info_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON book_copy_info_myoid_seq TO biblioteq_guest;
GRANT SELECT ON book_copy_info_myoid_seq TO biblioteq_membership;
GRANT SELECT ON book_copy_info_myoid_seq TO biblioteq_patron;
GRANT SELECT ON book_files TO biblioteq_circulation;
GRANT SELECT ON book_files TO biblioteq_guest;
GRANT SELECT ON book_files TO biblioteq_membership;
GRANT SELECT ON book_files TO biblioteq_patron;
GRANT SELECT ON book_files_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON book_files_myoid_seq TO biblioteq_guest;
GRANT SELECT ON book_files_myoid_seq TO biblioteq_membership;
GRANT SELECT ON book_files_myoid_seq TO biblioteq_patron;
GRANT SELECT ON book_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON book_myoid_seq TO biblioteq_guest;
GRANT SELECT ON book_myoid_seq TO biblioteq_membership;
GRANT SELECT ON book_myoid_seq TO biblioteq_patron;
GRANT SELECT ON grey_literature TO biblioteq_circulation;
GRANT SELECT ON grey_literature TO biblioteq_guest;
GRANT SELECT ON grey_literature TO biblioteq_membership;
GRANT SELECT ON grey_literature TO biblioteq_patron;
GRANT SELECT ON grey_literature_files TO biblioteq_circulation;
GRANT SELECT ON grey_literature_files TO biblioteq_guest;
GRANT SELECT ON grey_literature_files TO biblioteq_membership;
GRANT SELECT ON grey_literature_files TO biblioteq_patron;
GRANT SELECT ON grey_literature_files_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON grey_literature_files_myoid_seq TO biblioteq_guest;
GRANT SELECT ON grey_literature_files_myoid_seq TO biblioteq_membership;
GRANT SELECT ON grey_literature_files_myoid_seq TO biblioteq_patron;
GRANT SELECT ON grey_literature_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON grey_literature_myoid_seq TO biblioteq_guest;
GRANT SELECT ON grey_literature_myoid_seq TO biblioteq_membership;
GRANT SELECT ON grey_literature_myoid_seq TO biblioteq_patron;
GRANT SELECT ON grey_literature_types TO biblioteq_circulation;
GRANT SELECT ON grey_literature_types TO biblioteq_guest;
GRANT SELECT ON grey_literature_types TO biblioteq_membership;
GRANT SELECT ON grey_literature_types TO biblioteq_patron;
GRANT SELECT ON item_borrower TO biblioteq_administrator;
GRANT SELECT ON item_borrower TO biblioteq_circulation;
GRANT SELECT ON item_borrower TO biblioteq_librarian;
GRANT SELECT ON item_borrower TO biblioteq_membership;
GRANT SELECT ON item_borrower TO biblioteq_patron;
GRANT SELECT ON item_request TO biblioteq_librarian;
GRANT SELECT ON journal TO biblioteq_circulation;
GRANT SELECT ON journal TO biblioteq_guest;
GRANT SELECT ON journal TO biblioteq_membership;
GRANT SELECT ON journal TO biblioteq_patron;
GRANT SELECT ON journal_copy_info TO biblioteq_circulation;
GRANT SELECT ON journal_copy_info TO biblioteq_guest;
GRANT SELECT ON journal_copy_info TO biblioteq_membership;
GRANT SELECT ON journal_copy_info TO biblioteq_patron;
GRANT SELECT ON journal_copy_info_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON journal_copy_info_myoid_seq TO biblioteq_guest;
GRANT SELECT ON journal_copy_info_myoid_seq TO biblioteq_membership;
GRANT SELECT ON journal_copy_info_myoid_seq TO biblioteq_patron;
GRANT SELECT ON journal_files TO biblioteq_circulation;
GRANT SELECT ON journal_files TO biblioteq_guest;
GRANT SELECT ON journal_files TO biblioteq_membership;
GRANT SELECT ON journal_files TO biblioteq_patron;
GRANT SELECT ON journal_files_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON journal_files_myoid_seq TO biblioteq_guest;
GRANT SELECT ON journal_files_myoid_seq TO biblioteq_membership;
GRANT SELECT ON journal_files_myoid_seq TO biblioteq_patron;
GRANT SELECT ON journal_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON journal_myoid_seq TO biblioteq_guest;
GRANT SELECT ON journal_myoid_seq TO biblioteq_membership;
GRANT SELECT ON journal_myoid_seq TO biblioteq_patron;
GRANT SELECT ON languages TO biblioteq_circulation;
GRANT SELECT ON languages TO biblioteq_guest;
GRANT SELECT ON languages TO biblioteq_membership;
GRANT SELECT ON languages TO biblioteq_patron;
GRANT SELECT ON locations TO biblioteq_circulation;
GRANT SELECT ON locations TO biblioteq_guest;
GRANT SELECT ON locations TO biblioteq_membership;
GRANT SELECT ON locations TO biblioteq_patron;
GRANT SELECT ON magazine TO biblioteq_circulation;
GRANT SELECT ON magazine TO biblioteq_guest;
GRANT SELECT ON magazine TO biblioteq_membership;
GRANT SELECT ON magazine TO biblioteq_patron;
GRANT SELECT ON magazine_copy_info TO biblioteq_circulation;
GRANT SELECT ON magazine_copy_info TO biblioteq_guest;
GRANT SELECT ON magazine_copy_info TO biblioteq_membership;
GRANT SELECT ON magazine_copy_info TO biblioteq_patron;
GRANT SELECT ON magazine_copy_info_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON magazine_copy_info_myoid_seq TO biblioteq_guest;
GRANT SELECT ON magazine_copy_info_myoid_seq TO biblioteq_membership;
GRANT SELECT ON magazine_copy_info_myoid_seq TO biblioteq_patron;
GRANT SELECT ON magazine_files TO biblioteq_circulation;
GRANT SELECT ON magazine_files TO biblioteq_guest;
GRANT SELECT ON magazine_files TO biblioteq_membership;
GRANT SELECT ON magazine_files TO biblioteq_patron;
GRANT SELECT ON magazine_files_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON magazine_files_myoid_seq TO biblioteq_guest;
GRANT SELECT ON magazine_files_myoid_seq TO biblioteq_membership;
GRANT SELECT ON magazine_files_myoid_seq TO biblioteq_patron;
GRANT SELECT ON magazine_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON magazine_myoid_seq TO biblioteq_guest;
GRANT SELECT ON magazine_myoid_seq TO biblioteq_membership;
GRANT SELECT ON magazine_myoid_seq TO biblioteq_patron;
GRANT SELECT ON member TO biblioteq_circulation;
GRANT SELECT ON member_history TO biblioteq_patron;
GRANT SELECT ON member_history_dnt TO biblioteq_circulation;
GRANT SELECT ON member_history_myoid_seq TO biblioteq_patron;
GRANT SELECT ON minimum_days TO biblioteq_membership;
GRANT SELECT ON monetary_units TO biblioteq_circulation;
GRANT SELECT ON monetary_units TO biblioteq_guest;
GRANT SELECT ON monetary_units TO biblioteq_membership;
GRANT SELECT ON monetary_units TO biblioteq_patron;
GRANT SELECT ON photograph TO biblioteq_circulation;
GRANT SELECT ON photograph TO biblioteq_guest;
GRANT SELECT ON photograph TO biblioteq_membership;
GRANT SELECT ON photograph TO biblioteq_patron;
GRANT SELECT ON photograph_collection TO biblioteq_circulation;
GRANT SELECT ON photograph_collection TO biblioteq_guest;
GRANT SELECT ON photograph_collection TO biblioteq_membership;
GRANT SELECT ON photograph_collection TO biblioteq_patron;
GRANT SELECT ON photograph_collection_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON photograph_collection_myoid_seq TO biblioteq_guest;
GRANT SELECT ON photograph_collection_myoid_seq TO biblioteq_membership;
GRANT SELECT ON photograph_collection_myoid_seq TO biblioteq_patron;
GRANT SELECT ON photograph_myoid_seq TO biblioteq_circulation;
GRANT SELECT ON photograph_myoid_seq TO biblioteq_guest;
GRANT SELECT ON photograph_myoid_seq TO biblioteq_membership;
GRANT SELECT ON photograph_myoid_seq TO biblioteq_patron;
GRANT SELECT, UPDATE, USAGE ON book_copy_info_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON book_copy_info_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON book_files_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON book_files_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON book_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON book_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON book_sequence TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON book_sequence TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON grey_literature_files_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON grey_literature_files_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON grey_literature_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON grey_literature_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON item_borrower_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON item_borrower_myoid_seq TO biblioteq_circulation;
GRANT SELECT, UPDATE, USAGE ON item_request_myoid_seq TO biblioteq_circulation;
GRANT SELECT, UPDATE, USAGE ON item_request_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON item_request_myoid_seq TO biblioteq_patron;
GRANT SELECT, UPDATE, USAGE ON journal_copy_info_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON journal_copy_info_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON journal_files_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON journal_files_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON journal_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON journal_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON magazine_copy_info_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON magazine_copy_info_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON magazine_files_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON magazine_files_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON magazine_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON magazine_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON member_history_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON member_history_myoid_seq TO biblioteq_circulation;
GRANT SELECT, UPDATE, USAGE ON photograph_collection_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON photograph_collection_myoid_seq TO biblioteq_librarian;
GRANT SELECT, UPDATE, USAGE ON photograph_myoid_seq TO biblioteq_administrator;
GRANT SELECT, UPDATE, USAGE ON photograph_myoid_seq TO biblioteq_librarian;
GRANT SELECT, USAGE ON item_request_myoid_seq TO biblioteq_administrator;
GRANT biblioteq_circulation TO biblioteq_administrator WITH ADMIN OPTION;
GRANT biblioteq_circulation TO biblioteq_circulation_librarian WITH ADMIN OPTION;
GRANT biblioteq_circulation TO biblioteq_circulation_librarian_membership WITH ADMIN OPTION;
GRANT biblioteq_circulation TO biblioteq_circulation_membership WITH ADMIN OPTION;
GRANT biblioteq_librarian TO biblioteq_administrator WITH ADMIN OPTION;
GRANT biblioteq_librarian TO biblioteq_circulation_librarian WITH ADMIN OPTION;
GRANT biblioteq_librarian TO biblioteq_circulation_librarian_membership WITH ADMIN OPTION;
GRANT biblioteq_librarian TO biblioteq_librarian_membership WITH ADMIN OPTION;
GRANT biblioteq_membership TO biblioteq_administrator WITH ADMIN OPTION;
GRANT biblioteq_membership TO biblioteq_circulation_librarian_membership WITH ADMIN OPTION;
GRANT biblioteq_membership TO biblioteq_circulation_membership WITH ADMIN OPTION;
GRANT biblioteq_membership TO biblioteq_librarian_membership WITH ADMIN OPTION;
GRANT biblioteq_patron TO biblioteq_administrator WITH ADMIN OPTION;
REVOKE ALL ON admin FROM biblioteq_guest;
REVOKE ALL ON admin FROM biblioteq_patron;

CREATE EXTENSION IF NOT EXISTS unaccent;
CREATE USER xbook_guest ENCRYPTED PASSWORD 'xbook_guest' IN ROLE biblioteq_guest;
GRANT biblioteq_administrator TO xbook_admin WITH ADMIN OPTION;
INSERT INTO admin (username, roles) VALUES ('xbook_admin', 'administrator');

/* PostgreSQL 9.5 or newer is required. */

ALTER TABLE item_borrower ENABLE ROW LEVEL SECURITY;
ALTER TABLE item_request ENABLE ROW LEVEL SECURITY;
ALTER TABLE member_history ENABLE ROW LEVEL SECURITY;
ALTER TABLE member_history_dnt ENABLE ROW LEVEL SECURITY;
CREATE POLICY item_borrower_biblioteq_patron_policy ON item_borrower TO biblioteq_patron USING (memberid = session_user);
CREATE POLICY item_borrower_policy ON item_borrower TO biblioteq_administrator, biblioteq_circulation USING (true);
CREATE POLICY item_request_biblioteq_patron_policy ON item_request TO biblioteq_patron USING (memberid = session_user);
CREATE POLICY item_request_policy ON item_request TO biblioteq_administrator, biblioteq_circulation, biblioteq_librarian USING (true);
CREATE POLICY member_history_biblioteq_patron_policy ON member_history TO biblioteq_patron USING (memberid = session_user);
CREATE POLICY member_history_policy ON member_history TO biblioteq_administrator, biblioteq_circulation, biblioteq_librarian, biblioteq_membership USING (true);
CREATE POLICY member_history_dnt_biblioteq_patron_policy ON member_history_dnt TO biblioteq_patron USING (memberid = session_user);
CREATE POLICY member_history_dnt_policy ON member_history_dnt TO biblioteq_administrator, biblioteq_circulation, biblioteq_membership USING (true);
