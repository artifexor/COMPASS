#ifndef VIEWPOINTSTABLEMODEL_H
#define VIEWPOINTSTABLEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

class ViewManager;
class ViewPoint;

class ViewPointsTableModel : public QAbstractItemModel
{
public:
    ViewPointsTableModel(ViewManager& view_manager);
    //virtual ~ViewPointsTableModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void update();
    unsigned int getIdOf (const QModelIndex& index);

    void setStatus (const QModelIndex &row_index, const std::string& value);

    int commentColumn () { return table_columns_.indexOf("comment"); }
    int statusColumn () { return table_columns_.indexOf("status"); }

private:
    ViewManager& view_manager_;

    QStringList table_columns_{"id", "name", "type", "status", "comment"};

    QIcon open_icon_;
    QIcon closed_icon_;
    QIcon todo_icon_;
    QIcon unknown_icon_;

    std::map<unsigned int, ViewPoint>& view_points_;

    void updateTableColumns();
};

#endif // VIEWPOINTSTABLEMODEL_H
