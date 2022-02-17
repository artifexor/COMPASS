#include "acadfilterwidget.h"
#include "stringconv.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace std;
using namespace Utils;

ACADFilterWidget::ACADFilterWidget(ACADFilter& filter, const std::string& class_id, const std::string& instance_id)
    : DBFilterWidget(class_id, instance_id, filter), filter_(filter)
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    label_ = new QLabel("ACADs IN");
    layout->addWidget(label_);

    value_edit_ = new QLineEdit();
    connect(value_edit_, &QLineEdit::textEdited, this, &ACADFilterWidget::valueEditedSlot);
    layout->addWidget(value_edit_);

    child_layout_->addLayout(layout);

    update();
}

ACADFilterWidget::~ACADFilterWidget()
{

}

void ACADFilterWidget::update()

{
    DBFilterWidget::update();

    assert (value_edit_);

    value_edit_->setText(filter_.valuesString().c_str());
}

void ACADFilterWidget::valueEditedSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "ACADFilterWidget: valueEditedSlot: '" << value_str << "'";

    filter_.valuesString(value_str);
}


