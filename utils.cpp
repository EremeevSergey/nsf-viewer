#include <QHBoxLayout>
#include "utils.h"

void setWidgetTo   (QWidget* what,QWidget* where)
{
    QHBoxLayout* lay = new QHBoxLayout(where);
    lay->setMargin(0);
    lay->addWidget(what);
    where->setLayout(lay);
}

