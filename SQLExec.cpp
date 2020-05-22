/**
 * @file SQLExec.cpp - implementation of SQLExec class
 * @author Mark McKinnon and Chenjie Fan
 * @see "Seattle University, CPSC5300, Spring 2020"
 */
#include "SQLExec.h"

using namespace std;
using namespace hsql;

// define static data
Tables *SQLExec::tables = nullptr;
Indices *SQLExec::indices = nullptr;

// make query result be printable
ostream &operator<<(ostream &out, const QueryResult &qres) {
    if (qres.column_names != nullptr) {
        for (auto const &column_name: *qres.column_names)
            out << column_name << " ";
        out << endl << "+";
        for (unsigned int i = 0; i < qres.column_names->size(); i++)
            out << "----------+";
        out << endl;
        for (auto const &row: *qres.rows) {
            for (auto const &column_name: *qres.column_names) {
                Value value = row->at(column_name);
                switch (value.data_type) {
                    case ColumnAttribute::INT:
                        out << value.n;
                        break;
                    case ColumnAttribute::TEXT:
                        out << "\"" << value.s << "\"";
                        break;
                    case ColumnAttribute::BOOLEAN:
                        out << (value.n == 0 ? "false" : "true");
                        break;
                    default:
                        out << "???";
                }
                out << " ";
            }
            out << endl;
        }
    }
    out << qres.message;
    return out;
}

QueryResult::~QueryResult() {
    // FIXME
    // Destructor that clears data in column names, column attributes, and rows
    // Check if each field has something in it, if it does then delete it
    if (column_names != nullptr) {
        delete column_names;
    }
    if (column_attributes != nullptr) {
        delete column_attributes;
    }
    if (rows != nullptr) {
        for (auto row: *rows) {
            delete row;
        }
        delete rows;
    }

}

/**
 * Execute SQL statement
 * @param statement SQL statement to execute
 * @return          query execution result
 */

QueryResult *SQLExec::execute(const SQLStatement *statement) {
    // FIXME: initialize _tables table, if not yet present
    if (SQLExec::tables == nullptr)
        SQLExec::tables = new Tables();

    if (SQLExec::indices == nullptr)
        SQLExec::indices = new Indices();
    try {
        switch (statement->type()) {
            case kStmtCreate:
                return create((const CreateStatement *) statement);
            case kStmtDrop:
                return drop((const DropStatement *) statement);
            case kStmtShow:
                return show((const ShowStatement *) statement);
            default:
                return new QueryResult("not implemented");
        }
    } catch (DbRelationError &e) {
        throw SQLExecError(string("DbRelationError: ") + e.what());
    }
}

void
SQLExec::column_definition(const ColumnDefinition *col, Identifier &column_name, ColumnAttribute &column_attribute) {
    //throw SQLExecError("not implemented");  // FIXME
    column_name = col->name;
    // Switch statement to set the data type to whatever is passed into the method
    switch (col->type) {
        // Case if the passed in column definition is an int
        case ColumnDefinition::INT:
            column_attribute.set_data_type(ColumnAttribute::INT);
            break;
        // Case if the passed in column definition is text
        case ColumnDefinition::TEXT:
            column_attribute.set_data_type(ColumnAttribute::TEXT);
            break;
        // Case if the passed in column definition is a double
        case ColumnDefinition::DOUBLE:
        default:
      // Throws an error if the data type is not one of these three types
            throw SQLExecError("unrecognized data type");
  }
}

/**
 * create table or index with given statement
 * @param statement given statement 
 * @return          query execution result
 */

QueryResult *SQLExec::create(const CreateStatement *statement) {
    switch (statement->type) {
        case CreateStatement::kTable:
            return create_table(statement);
        case CreateStatement::kIndex:
            return create_index(statement);
        default:
            return new QueryResult("Only CREATE TABLE and CREATE INDEX are implemented");
    }
}


/**
 * Create a table with a given statement
 * @param statement given statement
 * @return          query execution result
 */
QueryResult *SQLExec::create_table(const CreateStatement *statement) {
    //return new QueryResult("create table not implemented");  // FIXME
    // Check if the statement is a create table statement or not
    if (statement->type != CreateStatement::kTable) {
        return new QueryResult("CREATE TABLE is only statement implemented.");
    }
    // Initialize identifier objects and various other objects for column names and attributes
    Identifier table_name = statement->tableName;
    ColumnNames column_names;
    ColumnAttributes column_attributes;
    Identifier column_name;
    ColumnAttribute column_attribute;
    // Create column definition with specified column names and attributes as well as
    // adding column names and attributes to respective objects
    for (ColumnDefinition *col : *statement->columns) {
        column_definition(col, column_name, column_attribute);
        column_names.push_back(column_name);
        column_attributes.push_back(column_attribute);
    }

    // Add tables and columns to schema
    ValueDict row;
    row["table_name"] = table_name;
    // Insert row into tables
    Handle t_handle = SQLExec::tables->insert(&row);
    // Try to create a DbRelation and input all of the columns and their data types
    try {
        Handles c_handles;
        DbRelation& columns = SQLExec::tables->get_table(Columns::TABLE_NAME);
        // Try to put column names and types into rows and also try to create DbRelation
        // for the table.
        try {
            // Puts all column names and their data types into rows
            for (uint i = 0; i < column_names.size(); i++) {
                row["column_name"] = column_names[i];
                row["data_type"] = Value(column_attributes[i].get_data_type() == ColumnAttribute::INT
                                 ? "INT" : "TEXT");
                // Insert into the columns
                c_handles.push_back(columns.insert(&row));
            }
            // Create the DbRelation for the table
            DbRelation& table = SQLExec::tables->get_table(table_name);
            // Check if statement exists or not, and if it does then create the table if the
            // table at that name does not exist, if the statement doesn't exist, then just
            // create the table
            if (statement->ifNotExists) {
                table.create_if_not_exists();
            } else {
                table.create();
            }
            // If an exception gets raised, delete the columns and throw an exception
        } catch (exception& e) {
            // See if you can remove from columns
            try {
                for (auto const &handle: c_handles) {
                   columns.del(handle);

                }
                
            } catch (...) {}
                throw;
        }
        // If an exception gets caught, try to delete the tables at the specified handle
        // and throw an exception
    } catch (exception& e) {
        try {
            // See if you can remove from tables
            SQLExec::tables->del(t_handle);
        } catch (...) {}
        throw;
    }
    // Return a query result saying that the table was created.
    return new QueryResult("created " + table_name);

}

/**
 * Create an index for a table with a given statement
 * @param statement given statement 
 * @return          query execution result
 */

QueryResult *SQLExec::create_index(const CreateStatement *statement) {
    //return new QueryResult("create index not implemented");  // FIXME

    // Initialize identifier objects 
    Identifier table_name = statement->tableName;
    Identifier index_name = statement->indexName;
    char *index_type = statement->indexType;

    DbRelation& table = SQLExec::tables->get_table(table_name);
    const ColumnNames& columns = table.get_column_names();
    for (auto const& col: *statement->indexColumns) {
        if (find(columns.begin(), columns.end(), col) == columns.end())
            throw SQLExecError("column " + string(col) + " doesn't exist");
    }
    //make row for new indices
    ValueDict row;
    row["table_name"] = Value(table_name);
    row["index_name"] = Value(index_name);
    
    row["index_type"] = Value(index_type);
    row["is_unique"] = Value(string(index_type) == "BTREE" ? true : false);

    // hold handles of inserting row for possible rollback when catch exception
    Handles index_handles;
    int seq_in_index = 1;
    try{
        //insert new index(row) to _indices
        for(auto const& col : *statement->indexColumns){
            
            
			row["column_name"] = Value(col);
            row["seq_in_index"] = Value(seq_in_index++);
			index_handles.push_back(SQLExec::indices->insert(&row));
        }
        // create index
        DbIndex& index = SQLExec::indices->get_index(table_name, index_name);
		index.create();

    }catch(exception& e){
        try{
            for (auto const &handle: index_handles) {
                   SQLExec::indices->del(handle);
            }
        }catch(...){}
        throw;
    }

    return new QueryResult("created index " + index_name);
}


/**
 * drop table or index with given statement
 * @param statement given statement 
 * @return          query execution result
 */
QueryResult *SQLExec::drop(const DropStatement *statement) {
	switch (statement->type) {
	case DropStatement::kTable:
		return drop_table(statement);
	case DropStatement::kIndex:
		return drop_index(statement);
	default:
		return new QueryResult("Only DROP TABLE and DROP INDEX are implemented");
	}
}



/**
 * Drop a table with a given statement
 * @param statement given statement 
 * @return          query execution result
 */
QueryResult *SQLExec::drop_table(const DropStatement *statement) {
    //return new QueryResult("not implemented"); // FIXME
    // Check if the statement is a drop table statement, and throw an exception if it isn't
    if (statement->type != DropStatement::kTable) {
        throw SQLExecError("unrecognized DROP type");
    }
    // Create identifier for the table name and throw an exception if the user is trying
    // to drop a schema table
    Identifier table_name = statement->name;
    if (table_name == Tables::TABLE_NAME || table_name == Columns::TABLE_NAME) {
        throw SQLExecError("cannot drop a schema table");
    }
  
    ValueDict where;
    where["table_name"] = Value(table_name);
    // Create a DbRelation for the table that the user wants to drop
    DbRelation& table = SQLExec::tables->get_table(table_name);

    // remove indices
    for (auto const &index_name: SQLExec::indices->get_index_names(table_name)) {
        DbIndex &index = SQLExec::indices->get_index(table_name, index_name);
        index.drop();  // drop the index
    }
    Handles *handles = SQLExec::indices->select(&where);
    for (auto const &handle: *handles)
        SQLExec::indices->del(handle);  // remove all rows from _indices
    delete handles;

    // First remove data from the columns schema before dropping the table
    DbRelation& columns = SQLExec::tables->get_table(Columns::TABLE_NAME);
    handles = columns.select(&where);
    for (auto const& handle: *handles) {
        columns.del(handle);
    }
    delete handles;
    // Drop the table
    table.drop();
    
    // Remove that dropped table from the table schema
    SQLExec::tables->del(*SQLExec::tables->select(&where)->begin());
    // Return a query result saying that the specified table has been dropped
    return new QueryResult(string("Dropped ") + table_name);

}

/**
 * Drop an index with a given statement
 * @param statement given statement 
 * @return          query execution result
 */
QueryResult *SQLExec::drop_index(const DropStatement *statement) {

	//Double check if statement is DROP Index
	if (statement->type != DropStatement::kIndex)
		return new QueryResult("Only handle DROP INDEX");


	// get the table name and index name
	Identifier table_name = statement->name;
	Identifier index_name = statement->indexName;
	// get the index from the database 
	DbIndex& index = SQLExec::indices->get_index(table_name, index_name);
	ValueDict where;
	 
	where["table_name"] = Value(table_name);
	where["index_name"] = Value(index_name);

	Handles* index_handles = SQLExec::indices->select(&where);
	index.drop();
	// iterate the index handles and remove 
	for (auto const &handle: *index_handles) {
		SQLExec::indices->del(handle);
	}


	//Handle memory because select method returns the "new" pointer
	delete index_handles;

	return new QueryResult("dropped index " + index_name);
}



/**
 * show table, columns or index with given statement
 * @param statement given statement
 * @return          query execution result
 */
QueryResult *SQLExec::show(const ShowStatement *statement) {
    switch (statement->type) {
        case ShowStatement::kTables:
            return show_tables();
        case ShowStatement::kColumns:
            return show_columns(statement);
        case ShowStatement::kIndex:
            return show_index(statement);
        default:
            throw SQLExecError("unrecognized SHOW type");
    }
}


/**
* Show all of the indices from a given table
* @param SQL statement with a SHOW INDEX statement within it
* @return QueryResult that shows all the indices at the specified
* table and indication that the query complete
*/
QueryResult *SQLExec::show_index(const ShowStatement *statement) {
    //    return new QueryResult("not implemented"); // FIXME
    // Create DbRelation object that will work with the indices table

    //DbRelation &indices = SQLExec::tables->get_table(Indices::TABLE_NAME);
    Identifier table_name = statement->tableName;
    // Create Column_names object to store all of the indices information
    ColumnNames *column_names = new ColumnNames;
    column_names->push_back("table_name");
    column_names->push_back("index_name");
    column_names->push_back("column_name");
    column_names->push_back("seq_in_index");   
    column_names->push_back("index_type");
    column_names->push_back("is_unique");
    // Create ColumnAttributes object to store all of the column attributes
    // For the indices columns
    ColumnAttributes *column_attributes = new ColumnAttributes;
    column_attributes->push_back(ColumnAttribute(ColumnAttribute::TEXT));
    column_attributes->push_back(ColumnAttribute(ColumnAttribute::INT));
    column_attributes->push_back(ColumnAttribute(ColumnAttribute::BOOLEAN));
    // Get the table that the user wants to show the indices from
    ValueDict where;
    where["table_name"] = Value(statement->tableName);
    //Handles *handles = indices.select(&where);
    Handles *handles = SQLExec::indices->select(&where);
    u_long n = handles->size();
    ValueDicts *rows = new ValueDicts;
    // Show all of the indices from that table
    for (auto const &handle: *handles) {
        //ValueDict *row = indices.project(handle, column_names);
        ValueDict *row = SQLExec::indices->project(handle, column_names);
        rows->push_back(row);
    }
    // Delete the handle when you are done with it
    delete handles;
    // Return the query result that shows the indices and indicates that it's successful
    return new QueryResult(column_names, column_attributes, rows, "successfully returned " +
                            to_string(n) + " indices");
}

/**
* show tables
* @param statement     given statement 
* @return              query execution result
*/
QueryResult *SQLExec::show_tables() {
    //return new QueryResult("not implemented"); // FIXME
    //Get columns for a specific table
    ColumnNames *column_names = new ColumnNames;
    column_names->push_back("table_name");
    
    ColumnAttributes *column_attributes = new ColumnAttributes;
    column_attributes->push_back(ColumnAttribute(ColumnAttribute::TEXT));
    
    Handles *handles = SQLExec::tables->select();
    //Number of returned rows in result, minus 3 means minus "_tables", "_columns" and "_indices"
    u_long row_number = handles->size() - 3;
    ValueDicts *rows = new ValueDicts;
    
    for(auto const &handle : *handles){
        ValueDict *row = SQLExec::tables->project(handle, column_names);
        Identifier table_name = row->at("table_name").s;
        if(table_name != Tables::TABLE_NAME && table_name != Columns::TABLE_NAME && table_name != Indices::TABLE_NAME){
            rows->push_back(row);
        }
    }
    
    delete handles;
    return new QueryResult(column_names, column_attributes, rows, 
            "successfully returned " + to_string(row_number) + " rows");   

}


/**
* show columns
* @param statement     given statement 
* @return              query execution result
*/
QueryResult *SQLExec::show_columns(const ShowStatement *statement) {
    //return new QueryResult("not implemented"); // FIXME
    DbRelation& columns = SQLExec::tables->get_table(Columns::TABLE_NAME);

    //Create Column_names object to store all of the columns information
    
    ColumnNames * column_names = new ColumnNames;
    column_names->push_back("table_name");
    column_names->push_back("column_name");
    column_names->push_back("data_type");
    
    // Create ColumnAttributes object to store all of the column attributes
    ColumnAttributes *column_attributes = new ColumnAttributes;
    column_attributes->push_back(ColumnAttribute(ColumnAttribute::TEXT));

    // Get columns from the table that the user wants to show 
    ValueDict where;
    where["table_name"] = Value(statement->tableName);
    Handles* handles = columns.select(&where);
	u_long row_number = handles->size();// Number of returned rows in result

	ValueDicts* rows = new ValueDicts;

	// get all entries of column names of the table
	
	for (unsigned int i = 0; i < handles->size(); i++) {
		// get each column name and teh data type from the table.
		  
		ValueDict* row = columns.project(handles->at(i), column_names);
		// add each row to the rows vector
		rows->push_back(row);
	}

	//Handle memory 
	//declared in heap
	delete handles;

	//return the QR of all the information about the columns. 
	return new QueryResult(column_names, column_attributes, rows,
		" successfully returned " + to_string(row_number) + " rows");

}

