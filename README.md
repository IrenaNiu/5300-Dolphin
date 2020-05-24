# 5300-Dolphin
DB Relation Manager project for CPSC5300/4300 at Seattle U, Spring 2020 

## Sprint Invierno
**Milestone 5:** </br>
Insert, Delete, Simple Queries - rudimentary implementation of certain INSERT, DELETE and simple SELECT statements.</br>

**General Steps:** </br>
1. Git clone or download this repo
2. Compile the code by runing "make"
3. Run "./sql5300 ../data" (Should make directory of "data" outside the repo first)
4. Try example test commands as below
5. Enter "quit" to exit

**Test Steps for M5:** (Create Table, Show Table/Columns and Drop Table)
```sql
SQL> create table goober (x int, y int, z int)
CREATE TABLE goober (x INT, y INT, z INT)
created goober
SQL> show tables
SHOW TABLES
table_name 
+----------+
"goober" 
successfully returned 1 rows
SQL> create table foo (id int, data text)
CREATE TABLE foo (id INT, data TEXT)
created foo
SQL> show tables
SHOW TABLES
table_name 
+----------+
"goober" 
"foo" 
successfully returned 2 rows
SQL> show columns from foo
SHOW COLUMNS FROM foo
table_name column_name data_type 
+----------+----------+----------+
"foo" "id" "INT" 
"foo" "data" "TEXT" 
successfully returned 2 rows
SQL> create index fx on foo (id)
CREATE INDEX fx ON foo USING BTREE (id)
created index fx
SQL> create index fz on foo (data)
CREATE INDEX fz ON foo USING BTREE (data)
created index fz
SQL> show index from foo
SHOW INDEX FROM foo
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
"foo" "fx" "id" 1 "BTREE" true 
"foo" "fz" "data" 1 "BTREE" true 
successfully returned 2 rows
SQL> insert into foo (id, data) values (1,"one")
INSERT INTO foo (id, data) VALUES (1, "one")
successfully inserted 1 row into foo and 2 indices
SQL> select * from foo
SELECT * FROM foo
id data 
+----------+----------+
1 "one" 
successfully returned 1 rows
QL> insert into foo values (2, "Two"); insert into foo values (3, "Three"); insert into foo values (99, "wowzers, Penny!!")
INSERT INTO foo VALUES (2, "Two")
successfully inserted 1 row into foo and 2 indices
INSERT INTO foo VALUES (3, "Three")
successfully inserted 1 row into foo and 2 indices
INSERT INTO foo VALUES (99, "wowzers, Penny!!")
successfully inserted 1 row into foo and 2 indices
SQL> select * from foo
SELECT * FROM foo
id data 
+----------+----------+
1 "one" 
2 "Two" 
3 "Three" 
99 "wowzers, Penny!!" 
successfully returned 4 rows
SQL> select * from foo where id=3
SELECT * FROM foo WHERE id = 3
id data 
+----------+----------+
3 "Three" 
successfully returned 1 rows
SQL> select * from foo where id=1 and data="one"
SELECT * FROM foo WHERE id = 1 AND data = "one"
id data 
+----------+----------+
1 "one" 
successfully returned 1 rows
SQL> select * from foo where id=99 and data="nine"
SELECT * FROM foo WHERE id = 99 AND data = "nine"
id data 
+----------+----------+
successfully returned 0 rows
SQL> select id from foo
SELECT id FROM foo
id 
+----------+
1 
2 
3 
99 
successfully returned 4 rows
SQL> select data from foo where id=1
SELECT data FROM foo WHERE id = 1
data 
+----------+
"one" 
successfully returned 1 rows
SQL> delete from foo where id=1
DELETE FROM foo WHERE id = 1
successfully deleted 1 rows from foo and 2 indices
SQL> select * from foo
SELECT * FROM foo
id data 
+----------+----------+
2 "Two" 
3 "Three" 
99 "wowzers, Penny!!" 
successfully returned 3 rows
SQL> delete from foo
DELETE FROM foo
successfully deleted 3 rows from foo and 2 indices
SQL> select * from foo
SELECT * FROM foo
id data 
+----------+----------+
successfully returned 0 rows
SQL> insert into foo values (2, "Two"); insert into foo values (3, "Three"); insert into foo values (99, "wowzers, Penny!!")
INSERT INTO foo VALUES (2, "Two")
successfully inserted 1 row into foo and 2 indices
INSERT INTO foo VALUES (3, "Three")
successfully inserted 1 row into foo and 2 indices
INSERT INTO foo VALUES (99, "wowzers, Penny!!")
successfully inserted 1 row into foo and 2 indices
SQL> select * from foo
SELECT * FROM foo
id data 
+----------+----------+
2 "Two" 
3 "Three" 
99 "wowzers, Penny!!" 
successfully returned 3 rows
SQL> drop index fz from foo
DROP INDEX fz FROM foo
dropped index fz
SQL> show index from foo
SHOW INDEX FROM foo
table_name index_name column_name seq_in_index index_type is_unique 
+----------+----------+----------+----------+----------+----------+
"foo" "fx" "id" 1 "BTREE" true 
successfully returned 1 rows
SQL> insert into foo (id) VALUES (100)
INSERT INTO foo (id) VALUES (100)
Error: DbRelationError: don't know how to handle NULLs, defaults, etc. yet
SQL> select * from foo
SELECT * FROM foo
id data 
+----------+----------+
2 "Two" 
3 "Three" 
99 "wowzers, Penny!!" 
successfully returned 3 rows
SQL> drop table foo
DROP TABLE foo
dropped foo
SQL> show tables
SHOW TABLES
table_name 
+----------+
"goober" 
successfully returned 1 rows
SQL> quit
```

If there is failed test or just want to start over the test, delete all data as below:
```sh
rm -f ../data/*
``` 

## Current Status
Milestone 5:  </br>
Milestone 6:  </br>


## Previous Sprint & Milestones

## Sprint Otoño
**Milestone 3:**
Schema Storage - rudimentary implementation of CREATE TABLE, DROP TABLE, SHOW TABLE, and SHOW COLUMNS. </br> 

**Milestone 4:**
Indexing Setup - rudimentary implementation of CREATE INDEX, SHOW INDEX, and DROP INDEX. </br>