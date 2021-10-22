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

#include "dbtablecolumncombobox.h"

#include "compass.h"
#include "dbovariable.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbtablecolumn.h"
#include "metadbtable.h"

DBTableColumnComboBox::DBTableColumnComboBox(const std::string& meta_table, DBOVariable& variable,
                                             QWidget* parent)
    : meta_table_(meta_table), variable_(variable)
{
    logdbg << "DBTableColumnComboBox: DBTableColumnComboBox";

    DBSchema& dbschema = COMPASS::instance().schemaManager().getCurrentSchema();

    assert(dbschema.hasMetaTable(meta_table));
    const MetaDBTable& meta = dbschema.metaTable(meta_table);

    std::string variable_id = variable_.variableIdentifier();

    auto cols = meta.columns();

    addItem("");

    logdbg << "DBTableColumnComboBox: DBTableColumnComboBox: variable_id '" << variable_id << "'";

    int index = -1;
    unsigned int cnt = 1;
    for (auto it = cols.begin(); it != cols.end(); it++)
    {
        if (variable_id == it->second.identifier())
            index = cnt;

        addItem(it->second.identifier().c_str());
        cnt++;
    }

    if (index != -1)
    {
        setCurrentIndex(index);
    }
}

DBTableColumnComboBox::~DBTableColumnComboBox() {}
