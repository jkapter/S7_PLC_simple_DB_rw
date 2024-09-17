#ifndef HINTINPUTDIALOG_H
#define HINTINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QDoubleValidator>
#include <QPlainTextEdit>

class HintInputDialog: public QDialog
{
    Q_OBJECT

public:
    HintInputDialog(QWidget* parent = 0);
    ~HintInputDialog();

signals:
    void applied(QString text);

private slots:
    void accepted();
    void canceled();

private:
    QPlainTextEdit* pte_;

};

#endif // HINTINPUTDIALOG_H
