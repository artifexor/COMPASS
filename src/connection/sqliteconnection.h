/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SQLITECONNECTION_H_
#define SQLITECONNECTION_H_

#include <qobject.h>

#include <sqlite3.h>

#include <memory>
#include <string>

#include "configurable.h"

#include "global.h"

class Buffer;
class DBInterface;
class SQLiteConnectionWidget;
class SQLiteConnectionInfoWidget;
class SavedFile;
class PropertyList;
class DBCommand;
class DBCommandList;
class DBResult;
class DBConnectionInfo;
class Buffer;
class DBTableInfo;
class QWidget;

/**
 * @brief Interface for a SQLite3 database connection
 *
 */
class SQLiteConnection : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void connectedSignal();

public:
    SQLiteConnection(const std::string& class_id, const std::string& instance_id,
                     DBInterface* interface);
    virtual ~SQLiteConnection();

    void openFile(const std::string& file_name);

    virtual void disconnect();

    void executeSQL(const std::string& sql);

    void prepareBindStatement(const std::string& statement);
    void beginBindTransaction();
    void stepAndClearBindings();
    void endBindTransaction();
    void finalizeBindStatement();

    void bindVariable(unsigned int index, int value);
    void bindVariable(unsigned int index, double value);
    void bindVariable(unsigned int index, const std::string& value);
    void bindVariableNull(unsigned int index);

    std::shared_ptr<DBResult> execute(const DBCommand& command);
    std::shared_ptr<DBResult> execute(const DBCommandList& command_list);

    void prepareCommand(const std::shared_ptr<DBCommand> command);
    std::shared_ptr<DBResult> stepPreparedCommand(unsigned int max_results = 0);
    void finalizeCommand();
    bool getPreparedCommandDone() { return prepared_command_done_; }

    std::map<std::string, DBTableInfo> getTableInfo();
    virtual std::vector<std::string> getDatabases();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    QWidget* widget();
    // void deleteWidget ();
    QWidget* infoWidget();
    std::string status() const;
    std::string identifier() const;
    std::string shortIdentifier() const;
    std::string type() const { return SQLITE_IDENTIFIER; }

    const std::map<std::string, SavedFile*>& fileList() { return file_list_; }
    bool hasFile(const std::string& filename) { return file_list_.count(filename) > 0; }
    void addFile(const std::string& filename);
    void removeFile(const std::string& filename);
    void removeAllFiles ();

    const std::string& lastFilename() { return last_filename_; }

    bool ready() { return connection_ready_; }

protected:
    DBInterface& interface_;

    bool connection_ready_;

    std::string last_filename_;

    /// Database handle to execute queries
    sqlite3* db_handle_{nullptr};
    /// Statement for binding variables to.
    sqlite3_stmt* statement_{nullptr};

    std::shared_ptr<DBCommand> prepared_command_;
    bool prepared_command_done_;

    std::unique_ptr<SQLiteConnectionWidget> widget_;
    std::unique_ptr<SQLiteConnectionInfoWidget> info_widget_;

    std::map<std::string, SavedFile*> file_list_;

    void execute(const std::string& command);
    void execute(const std::string& command, std::shared_ptr<Buffer> buffer);
    void readRowIntoBuffer(const PropertyList& list, unsigned int num_properties,
                           std::shared_ptr<Buffer> buffer, unsigned int index);

    void prepareStatement(const std::string& sql);
    void finalizeStatement();

    std::vector<std::string> getTableList();
    DBTableInfo getColumnList(const std::string& table);

    virtual void checkSubConfigurables() override {};
};

#endif /* DBINTERFACE_H_ */
