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

#include "managesectorstaskwidget.h"
#include "managesectorstask.h"
#include "logger.h"
#include "compass.h"
//#include "dbinterface.h"
#include "evaluationmanager.h"
#include "sector.h"
#include "sectorlayer.h"
#include "files.h"
#include "importsectordialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QColorDialog>
#include <QApplication>

using namespace std;
using namespace Utils;

ManageSectorsTaskWidget::ManageSectorsTaskWidget(ManageSectorsTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    main_layout_ = new QVBoxLayout();

    tab_widget_ = new QTabWidget();

    main_layout_->addWidget(tab_widget_);

    addImportTab();
    addManageTab();

    updateSectorTable();

    setLayout(main_layout_);
}

void ManageSectorsTaskWidget::expertModeChangedSlot() {}

void ManageSectorsTaskWidget::addImportTab()
{
    assert(tab_widget_);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_tab_layout = new QVBoxLayout();

    // file stuff
    {
        QVBoxLayout* files_layout = new QVBoxLayout();

        QLabel* files_label = new QLabel("File Selection");
        files_label->setFont(font_bold);
        files_layout->addWidget(files_label);

        file_list_ = new QListWidget();
        file_list_->setWordWrap(true);
        file_list_->setTextElideMode(Qt::ElideNone);
        file_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
        file_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(file_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedFileSlot()));

        updateFileListSlot();
        files_layout->addWidget(file_list_);

        QHBoxLayout* button_layout = new QHBoxLayout();

        add_file_button_ = new QPushButton("Add");
        connect(add_file_button_, &QPushButton::clicked, this, &ManageSectorsTaskWidget::addFileSlot);
        button_layout->addWidget(add_file_button_);

        delete_file_button_ = new QPushButton("Remove");
        connect(delete_file_button_, &QPushButton::clicked, this, &ManageSectorsTaskWidget::deleteFileSlot);
        button_layout->addWidget(delete_file_button_);

        delete_all_files_button_ = new QPushButton("Remove All");
        connect(delete_all_files_button_, &QPushButton::clicked, this, &ManageSectorsTaskWidget::deleteAllFilesSlot);
        button_layout->addWidget(delete_all_files_button_);

        files_layout->addLayout(button_layout);

        main_tab_layout->addLayout(files_layout);

        parse_msg_edit_ = new QTextEdit ();
        parse_msg_edit_->setReadOnly(true);
        main_tab_layout->addWidget(parse_msg_edit_);

        import_button_ = new QPushButton("Import");
        connect (import_button_, &QPushButton::clicked, this, &ManageSectorsTaskWidget::importSlot);
        import_button_->setDisabled(true);
        main_tab_layout->addWidget(import_button_); // is enabled in updateContext
    }

    if (task_.currentFilename().size())
        updateParseMessage();

    QWidget* main_tab_widget = new QWidget();
    main_tab_widget->setContentsMargins(0, 0, 0, 0);
    main_tab_widget->setLayout(main_tab_layout);
    tab_widget_->addTab(main_tab_widget, "Import");
}

void ManageSectorsTaskWidget::addManageTab()
{
    QVBoxLayout* manage_tab_layout = new QVBoxLayout();

    sector_table_ = new QTableWidget();
    sector_table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
    sector_table_->setColumnCount(table_columns_.size());
    sector_table_->setHorizontalHeaderLabels(table_columns_);
    sector_table_->verticalHeader()->setVisible(false);
    sector_table_->sortByColumn(2, Qt::DescendingOrder);
    connect(sector_table_, &QTableWidget::itemChanged, this,
            &ManageSectorsTaskWidget::sectorItemChangedSlot);

    manage_tab_layout->addWidget(sector_table_);


    QFont font_bold;
    font_bold.setBold(true);

    // bottom buttons
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        QPushButton* export_button_ = new QPushButton("Export All");
        connect(export_button_, &QPushButton::clicked, this,
                &ManageSectorsTaskWidget::exportSectorsSlot);
        button_layout->addWidget(export_button_);

        QPushButton* clear_button_ = new QPushButton("Clear All");
        connect(clear_button_, &QPushButton::clicked, this,
                &ManageSectorsTaskWidget::clearSectorsSlot);
        button_layout->addWidget(clear_button_);

        QPushButton* import_button_ = new QPushButton("Import");
        connect(import_button_, &QPushButton::clicked, this,
                &ManageSectorsTaskWidget::importSectorsSlot);
        button_layout->addWidget(import_button_);

        manage_tab_layout->addLayout(button_layout);
    }

    QWidget* manage_tab_widget = new QWidget();
    manage_tab_widget->setContentsMargins(0, 0, 0, 0);
    manage_tab_widget->setLayout(manage_tab_layout);
    tab_widget_->addTab(manage_tab_widget, "Manage");
}

void ManageSectorsTaskWidget::updateSectorTable()
{
    logdbg << "ManageSectorsTaskWidget: updateSectorTable";

    assert(sector_table_);

    sector_table_->blockSignals(true);

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    vector<std::shared_ptr<SectorLayer>>& sector_layers = eval_man.sectorsLayers();

    unsigned int num_layers=0;
    for (auto& sec_lay_it : sector_layers)
        num_layers += sec_lay_it->sectors().size();

    sector_table_->setDisabled(true); // otherwise first element is edited after
    sector_table_->clearContents();
    sector_table_->setRowCount(num_layers);

    int row = 0;
    int col = 0;

    for (auto& sec_lay_it : sector_layers)
    {
        string layer_name = sec_lay_it->name();

        for (auto& sec_it : sec_lay_it->sectors())
        {
            unsigned int sector_id = sec_it->id();
            shared_ptr<Sector> sector = sec_it;

            col = 0;

            {  // Sector ID
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(sector->id()));
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                item->setData(Qt::UserRole, QVariant(sector_id));
                sector_table_->setItem(row, col, item);
            }

            {  // Sector Name
                ++col;
                QTableWidgetItem* item = new QTableWidgetItem(sector->name().c_str());
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                item->setData(Qt::UserRole, QVariant(sector_id));
                sector_table_->setItem(row, col, item);
            }

            {  // Layer Name
                ++col;
                QTableWidgetItem* item = new QTableWidgetItem(layer_name.c_str());
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                item->setData(Qt::UserRole, QVariant(sector_id));
                sector_table_->setItem(row, col, item);
            }

            {  // Exclude
                ++col;
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
                item->setCheckState(sec_it->exclude() ? Qt::Checked : Qt::Unchecked);
                //item1->setCheckState();
                item->setData(Qt::UserRole, QVariant(sector_id));
                sector_table_->setItem(row, col, item);
            }

            {  // Num Points
                ++col;
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(sector->points().size()));
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                item->setData(Qt::UserRole, QVariant(sector_id));
                sector_table_->setItem(row, col, item);
            }

            {  // Altitude Minimum
                ++col;
                if (sector->hasMinimumAltitude())
                {
                    QTableWidgetItem* item =
                            new QTableWidgetItem(QString::number(sector->minimumAltitude()));
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                    item->setData(Qt::UserRole, QVariant(sector_id));
                    sector_table_->setItem(row, col, item);
                }
                else
                {
                    QTableWidgetItem* item = new QTableWidgetItem();
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                    item->setData(Qt::UserRole, QVariant(sector_id));
                    item->setBackground(Qt::darkGray);
                    sector_table_->setItem(row, col, item);
                }
            }

            {  // Altitude Maximum
                ++col;
                if (sector->hasMaximumAltitude())
                {
                    QTableWidgetItem* item =
                            new QTableWidgetItem(QString::number(sector->maximumAltitude()));
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                    item->setData(Qt::UserRole, QVariant(sector_id));
                    sector_table_->setItem(row, col, item);
                }
                else
                {
                    QTableWidgetItem* item = new QTableWidgetItem();
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                    item->setData(Qt::UserRole, QVariant(sector_id));
                    item->setBackground(Qt::darkGray);
                    sector_table_->setItem(row, col, item);
                }
            }

            {  // Color
                ++col;

                QPushButton* button = new QPushButton();
                QPalette pal = button->palette();
                pal.setColor(QPalette::Button, QColor(sector->colorStr().c_str()));
                //loginf << "UGA " << QColor(sector->colorStr().c_str()).name().toStdString();
                button->setAutoFillBackground(true);
                button->setPalette(pal);
                button->setFlat(true);
                button->update();

                connect (button, &QPushButton::clicked, this, &ManageSectorsTaskWidget::changeSectorColorSlot);

                button->setProperty("sector_id", QVariant(sector->id()));
                sector_table_->setCellWidget(row, col, button);
            }

            {  // Delete
                ++col;

                QPushButton* button = new QPushButton();
                button->setIcon(QIcon(Files::getIconFilepath("delete.png").c_str()));
                button->setFlat(true);

                connect (button, &QPushButton::clicked, this, &ManageSectorsTaskWidget::deleteSectorSlot);

                button->setProperty("sector_id", QVariant(sector->id()));
                sector_table_->setCellWidget(row, col, button);
            }

            ++row;
        }
    }

    sector_table_->resizeColumnsToContents();

    sector_table_->blockSignals(false);

    sector_table_->setDisabled(false);
}

void ManageSectorsTaskWidget::addFileSlot()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Add Sector File(s)");
    // dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFiles);
    // dialog.setNameFilter(trUtf8("Splits (*.000 *.001)"));
    QStringList fileNames;
    if (dialog.exec())
    {
        for (auto& filename : dialog.selectedFiles())
            addFile(filename.toStdString());
    }
}

void ManageSectorsTaskWidget::addFile(const std::string& filename)
{
    if (!task_.hasFile(filename))
        task_.addFile(filename);
}

void ManageSectorsTaskWidget::selectFile(const std::string& filename)
{
    assert(task_.hasFile(filename));
    task_.currentFilename(filename);

    QList<QListWidgetItem*> items = file_list_->findItems(filename.c_str(), Qt::MatchExactly);
    assert (items.size() > 0);

    for (auto item_it : items)
    {
        assert (item_it);
        file_list_->setCurrentItem(item_it);
    }

    updateParseMessage();
}

void ManageSectorsTaskWidget::updateParseMessage ()
{
    loginf << "ViewPointsImportTaskWidget: updateParseMessage";

    assert (parse_msg_edit_);
    parse_msg_edit_->setText(task_.parseMessage().c_str());

    import_button_->setEnabled(task_.canImportFile());
}

void ManageSectorsTaskWidget::deleteFileSlot()
{
    loginf << "ManageSectorsTaskWidget: deleteFileSlot";

    if (!file_list_->currentItem() || !task_.currentFilename().size())
    {
        QMessageBox m_warning(QMessageBox::Warning, "Sector File Deletion Failed",
                              "Please select a file in the list.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    assert(task_.currentFilename().size());
    assert(task_.hasFile(task_.currentFilename()));
    task_.removeCurrentFilename();
}

void ManageSectorsTaskWidget::deleteAllFilesSlot()
{
    loginf << "ManageSectorsTaskWidget: deleteAllFilesSlot";
    task_.removeAllFiles();
}

void ManageSectorsTaskWidget::selectedFileSlot()
{
    loginf << "ManageSectorsTaskWidget: selectedFileSlot";
    assert(file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert(task_.hasFile(filename.toStdString()));
    task_.currentFilename(filename.toStdString());
}

void ManageSectorsTaskWidget::updateFileListSlot()
{
    assert(file_list_);

    file_list_->clear();

    for (auto it : task_.fileList())
    {
        QListWidgetItem* item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == task_.currentFilename())
            file_list_->setCurrentItem(item);
    }
}

void ManageSectorsTaskWidget::importSlot()
{
    loginf << "ManageSectorsTaskWidget: importSlot";


    string filename = task_.currentFilename();

    filename = Files::getFilenameFromPath(filename);

    if (filename.find(".", 0) != string::npos)
        filename = filename.substr(0, filename.find(".", 0));

    ImportSectorDialog dialog(filename.c_str(), this);
    int ret = dialog.exec();

    if (ret == QDialog::Accepted)
    {
        loginf << "ManageSectorsTaskWidget: importSlot: accepted, layer name '" << dialog.layerName()
               << "' exclude " << dialog.exclude();

        assert (task_.canImportFile());
        task_.importFile(dialog.layerName(), dialog.exclude(), dialog.color());

        updateSectorTable();
    }
    else
        loginf << "ManageSectorsTaskWidget: importSlot: cancelled";
}

void ManageSectorsTaskWidget::sectorItemChangedSlot(QTableWidgetItem* item)
{
    loginf << "ManageSectorsTaskWidget: sectorItemChangedSlot";

    assert(item);
    assert(sector_table_);


    bool ok;
    unsigned int sector_id = item->data(Qt::UserRole).toUInt(&ok);
    assert(ok);
    int col = sector_table_->currentColumn();

    assert (col < table_columns_.size());
    std::string col_name = table_columns_.at(col).toStdString();

    std::string text = item->text().toStdString();

    loginf << "ManageSectorsTaskWidget: sectorItemChangedSlot: sector_id " << sector_id
           << " col_name " << col_name << " text '" << text << "'";

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    assert (eval_man.hasSector(sector_id));

    std::shared_ptr<Sector> sector = eval_man.sector(sector_id);

    if (col_name == "Sector Name")
    {
        if (text.size())
        {
            if (eval_man.hasSector(text, sector->layerName()))
            {
                QMessageBox m_warning(QMessageBox::Warning, "Sector Change Failed",
                ("Layer '"+sector->layerName()+"' Sector '"+text+"' already exists.").c_str(), QMessageBox::Ok);
                m_warning.exec();
            }
            else
                sector->name(text);
        }
    }
    else if (col_name == "Layer Name")
    {
        if (text.size())
        {
            if (eval_man.hasSector(sector->name(), text))
            {
                QMessageBox m_warning(QMessageBox::Warning, "Sector Change Failed",
                ("Layer '"+text+"' Sector '"+sector->name()+"' already exists.").c_str(), QMessageBox::Ok);
                m_warning.exec();
            }
            else
                sector->layerName(text);
        }
    }
    else if (col_name == "Exclude")
    {
        sector->exclude(item->checkState() == Qt::Checked);
    }
    else if (col_name == "Altitude Minimum")
    {
        if (!text.size())
            sector->removeMinimumAltitude();
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
                sector->minimumAltitude(value);
        }
    }
    else if (col_name == "Altitude Maximum")
    {
        if (!text.size())
            sector->removeMaximumAltitude();
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
                sector->maximumAltitude(value);
        }
    }
    else
        assert(false);  // impossible

    updateSectorTable();
}

void ManageSectorsTaskWidget::changeSectorColorSlot()
{
    loginf << "ManageSectorsTaskWidget: changeSectorColorSlot";

    QPushButton* button = dynamic_cast<QPushButton*>(sender());
    assert (button);

    QVariant sector_id_var = button->property("sector_id");
    assert (sector_id_var.isValid());

    unsigned int sector_id = sector_id_var.toUInt();

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    assert (eval_man.hasSector(sector_id));

    std::shared_ptr<Sector> sector = eval_man.sector(sector_id);

    QColor current_color = QColor(sector->colorStr().c_str());

    QColor color =
            QColorDialog::getColor(current_color, QApplication::activeWindow(), "Select Sector Color",
                                   QColorDialog::DontUseNativeDialog);

    if (color.isValid())
    {
        loginf << "ManageSectorsTaskWidget: changeSectorColorSlot: color " << color.name().toStdString();
        sector->colorStr(color.name().toStdString());
        updateSectorTable();
    }

}

void ManageSectorsTaskWidget::deleteSectorSlot()
{
    loginf << "ManageSectorsTaskWidget: deleteSectorSlot";

    QPushButton* button = dynamic_cast<QPushButton*>(sender());
    assert (button);

    QVariant sector_id_var = button->property("sector_id");
    assert (sector_id_var.isValid());

    unsigned int sector_id = sector_id_var.toUInt();

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    assert (eval_man.hasSector(sector_id));

    std::shared_ptr<Sector> sector = eval_man.sector(sector_id);

    eval_man.deleteSector(sector);

    updateSectorTable();
}

void ManageSectorsTaskWidget::exportSectorsSlot ()
{
    loginf << "ManageSectorsTaskWidget: exportSectorsSlot";

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    QFileDialog dialog(nullptr);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("JSON Files (*.json)");
    dialog.setDefaultSuffix("json");
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);

    QStringList file_names;
    if (dialog.exec())
        file_names = dialog.selectedFiles();

    QString filename;

    if (file_names.size() == 1)
        filename = file_names.at(0);
    else
        loginf << "ManageDataSourcesTask: exportSectorsSlot: cancelled";

    if (filename.size() > 0)
        eval_man.exportSectors(filename.toStdString());
}

void ManageSectorsTaskWidget::clearSectorsSlot ()
{
    loginf << "ManageSectorsTaskWidget: clearSectorsSlot";

    COMPASS::instance().evaluationManager().deleteAllSectors();

    updateSectorTable();
}

void ManageSectorsTaskWidget::importSectorsSlot ()
{
    loginf << "ManageSectorsTaskWidget: importSectorsSlot";

    QString filename =
        QFileDialog::getOpenFileName(nullptr, "Export Sectors as JSON", "", "*.json");

    if (filename.size() > 0)
    {
        importSectorsJSON(filename.toStdString());
    }
    else
        loginf << "ManageDataSourcesTask: importSectorsSlot: cancelled";
}

void ManageSectorsTaskWidget::importSectorsJSON (const std::string& filename)
{
    loginf << "ManageSectorsTaskWidget: importSectorsJSON: filename '" << filename << "'";

    assert (Files::fileExists(filename));

    COMPASS::instance().evaluationManager().importSectors(filename);

    updateSectorTable();
}
