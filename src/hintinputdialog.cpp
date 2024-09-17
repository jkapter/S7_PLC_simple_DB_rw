#include "hintinputdialog.h"

HintInputDialog::HintInputDialog(QWidget* parent): QDialog(parent)
{

    QBoxLayout* layout = new QVBoxLayout;
    pte_ = new QPlainTextEdit(this);
    layout->addWidget(pte_);

    QPushButton* okBtn = new QPushButton("OK");
    connect(okBtn, SIGNAL(clicked()),SLOT(accepted()));
    layout->addWidget(okBtn);

    QPushButton* cancelBtn = new QPushButton("Cancel");
    connect(cancelBtn, SIGNAL(clicked()), SLOT(canceled()));
    layout->addWidget(cancelBtn);

    setLayout(layout);
}

HintInputDialog::~HintInputDialog() {
    delete pte_;
}

void HintInputDialog::accepted() {
    emit applied(pte_->toPlainText());
    this->close();
}

void HintInputDialog::canceled() {
    emit applied("");
    this->close();
}
