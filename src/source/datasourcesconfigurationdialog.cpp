#include "datasourcesconfigurationdialog.h"
#include "datasourcetablemodel.h"
#include "datasourceeditwidget.h"
#include "datasourcemanager.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QHeaderView>
#include <QFileDialog>

using namespace std;

DataSourcesConfigurationDialog::DataSourcesConfigurationDialog(DataSourceManager& ds_man)
    : QDialog(), ds_man_(ds_man)
{
    setWindowTitle("Configure Data Sources");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(1000, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QHBoxLayout* top_layout = new QHBoxLayout();

    table_model_ = new DataSourceTableModel(ds_man_, *this);

    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(table_model_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(2, Qt::AscendingOrder);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //table_view_->setIconSize(QSize(24, 24));
    table_view_->setWordWrap(true);
    table_view_->reset();

    connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &DataSourcesConfigurationDialog::currentRowChanged);

    // HACKY adjust row size but works
    connect(table_view_->horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
            table_view_, SLOT(resizeRowsToContents()));

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
    top_layout->addWidget(table_view_);

    edit_widget_ = new DataSourceEditWidget (ds_man_, *this);
    top_layout->addWidget(edit_widget_);

    main_layout->addLayout(top_layout);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    main_layout->addWidget(line);

    QHBoxLayout* button_layout = new QHBoxLayout();

    QPushButton* import_button = new QPushButton ("Import");
    import_button->setToolTip("Imports data sources as JSON file. The data sources defined in the JSON file"
                              " will override existing ones (with same SAC/SIC) in the configuration and database");
    connect(import_button, &QPushButton::clicked, this, &DataSourcesConfigurationDialog::importClickedSlot);
    button_layout->addWidget(import_button);

    button_layout->addSpacing(32);

    QPushButton* delete_all_button = new QPushButton("Delete All");
    delete_all_button->setToolTip("Deletes all data sources existing in the configuration,"
                                  " but not those (also) defined in the database");
    connect(delete_all_button, &QPushButton::clicked, this, &DataSourcesConfigurationDialog::deleteAllClickedSlot);
    button_layout->addWidget(delete_all_button);

    button_layout->addSpacing(32);

    QPushButton* export_button = new QPushButton ("Export");
    export_button->setToolTip("Exports all data sources as JSON file");
    connect(export_button, &QPushButton::clicked, this, &DataSourcesConfigurationDialog::exportClickedSlot);
    button_layout->addWidget(export_button);

    button_layout->addStretch();

    QPushButton* done_button = new QPushButton("Done");
    connect(done_button, &QPushButton::clicked, this, &DataSourcesConfigurationDialog::doneClickedSlot);
    button_layout->addWidget(done_button);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}

void DataSourcesConfigurationDialog::updateDataSource(unsigned int ds_id)
{
    table_model_->updateDataSource(ds_id);
}

void DataSourcesConfigurationDialog::beginResetModel()
{
    table_model_->beginModelReset();
}

void DataSourcesConfigurationDialog::endResetModel()
{
    table_model_->endModelReset();
}

void DataSourcesConfigurationDialog::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    assert (edit_widget_);

    if (!current.isValid())
    {
        loginf << "DataSourcesConfigurationDialog: currentRowChanged: invalid index";

        edit_widget_->clear();

        return;
    }

    auto const source_index = proxy_model_->mapToSource(current);
    assert (source_index.isValid());

    unsigned int id = table_model_->getIdOf(source_index);

    loginf << "DataSourcesConfigurationDialog: currentRowChanged: current id " << id;

    edit_widget_->showID(id);
}

void DataSourcesConfigurationDialog::importClickedSlot()
{
    loginf << "DataSourcesConfigurationDialog: importClickedSlot";

    string filename = QFileDialog::getOpenFileName(this, "Import Data Sources", "", "*.json").toStdString();

    if (filename.size() > 0)
    {
        table_model_->beginModelReset();

        ds_man_.importDataSources(filename);

        table_model_->endModelReset();
    }
}

void DataSourcesConfigurationDialog::deleteAllClickedSlot()
{

}
void DataSourcesConfigurationDialog::exportClickedSlot()
{

}

void DataSourcesConfigurationDialog::doneClickedSlot()
{
    emit doneSignal();
}
