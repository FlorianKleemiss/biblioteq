DROP EXTENSION IF EXISTS unaccent;
DROP SEQUENCE IF EXISTS book_sequence;
DROP TABLE IF EXISTS admin;
DROP TABLE IF EXISTS book_binding_types;
DROP TABLE IF EXISTS book_copy_info;
DROP TABLE IF EXISTS book_files;
DROP TABLE IF EXISTS book;
DROP TABLE IF EXISTS grey_literature_files;
DROP TABLE IF EXISTS grey_literature_types;
DROP TABLE IF EXISTS grey_literature;
DROP TABLE IF EXISTS item_borrower;
DROP TABLE IF EXISTS item_request;
DROP TABLE IF EXISTS journal_copy_info;
DROP TABLE IF EXISTS journal_files;
DROP TABLE IF EXISTS journal;
DROP TABLE IF EXISTS languages;
DROP TABLE IF EXISTS locations;
DROP TABLE IF EXISTS magazine_copy_info;
DROP TABLE IF EXISTS magazine_files;
DROP TABLE IF EXISTS magazine;
DROP TABLE IF EXISTS member_history;
DROP TABLE IF EXISTS member_history_dnt;
DROP TABLE IF EXISTS member;
DROP TABLE IF EXISTS minimum_days;
DROP TABLE IF EXISTS monetary_units;
DROP TABLE IF EXISTS photograph;
DROP TABLE IF EXISTS photograph_collection;
DROP FUNCTION IF EXISTS delete_book();
DROP FUNCTION IF EXISTS delete_journal();
DROP FUNCTION IF EXISTS delete_magazine();
DROP FUNCTION IF EXISTS delete_request();
DROP DATABASE IF EXISTS xbook_db;
DROP ROLE IF EXISTS biblioteq_administrator;
DROP ROLE IF EXISTS biblioteq_circulation;
DROP ROLE IF EXISTS biblioteq_circulation_librarian;
DROP ROLE IF EXISTS biblioteq_circulation_librarian_membership;
DROP ROLE IF EXISTS biblioteq_circulation_membership;
DROP ROLE IF EXISTS biblioteq_guest;
DROP ROLE IF EXISTS biblioteq_librarian;
DROP ROLE IF EXISTS biblioteq_librarian_membership;
DROP ROLE IF EXISTS biblioteq_membership;
DROP ROLE IF EXISTS biblioteq_patron;
DROP USER IF EXISTS xbook_admin;
DROP USER IF EXISTS xbook_guest;
