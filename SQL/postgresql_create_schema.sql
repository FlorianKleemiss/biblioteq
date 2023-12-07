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

CREATE TABLE book_binding_types
(
	binding_type	 TEXT NOT NULL PRIMARY KEY
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
