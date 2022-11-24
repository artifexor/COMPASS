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

#ifndef DBCONTENTREADDBJOB_H_
#define DBCONTENTREADDBJOB_H_

#include "boost/date_time/posix_time/posix_time.hpp"
#include "dbcontent/variable/variableset.h"
#include "job.h"

class Buffer;
class DBContent;
class DBInterface;

class DBContentReadDBJob : public Job
{
    Q_OBJECT

signals:
    void intermediateSignal(std::shared_ptr<Buffer> buffer);

public:
    DBContentReadDBJob(DBInterface& db_interface, DBContent& dbcontent, dbContent::VariableSet read_list,
                       const std::vector<std::string>& extra_from_parts,
                       std::string custom_filter_clause, std::vector<dbContent::Variable*> filtered_variables,
                       bool use_order, dbContent::Variable* order_variable, bool use_order_ascending,
                       const std::string& limit_str);
    virtual ~DBContentReadDBJob();

    virtual void run();

    dbContent::VariableSet& readList() { return read_list_; }

    unsigned int rowCount() const;

protected:
    DBInterface& db_interface_;
    DBContent& dbcontent_;
    dbContent::VariableSet read_list_;
    std::vector<std::string> extra_from_parts_;
    std::string custom_filter_clause_;
    std::vector<dbContent::Variable*> filtered_variables_;
    bool use_order_;
    dbContent::Variable* order_variable_;
    bool use_order_ascending_;
    std::string limit_str_;

    unsigned int row_count_{0};
    std::shared_ptr<Buffer> cached_buffer_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
};

#endif /* DBCONTENTREADDBJOB_H_ */