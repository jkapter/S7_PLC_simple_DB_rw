#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTableWidget>
#include <QFile>
#include <QHeaderView>
#include <QResizeEvent>
#include <QJsonParseError>
#include <QThread>
#include <QJsonObject>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QTranslator>

#include "snap7/snap7.h"
#include "plc_data_handler.h"
#include "hintinputdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void tree_item_changed(QTreeWidgetItem* cur, QTreeWidgetItem* prev);
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;
    void write_to_plc_btn_clicked();
    void window_show_up();
    void table_cell_changed(int r, int c);
    void get_hint_for_file_params(QString hint);

private slots:
    void on_write_params_to_file_triggered();

    void on_read_params_from_file_triggered();

    void on_write_all_params_to_plc_triggered();

    void on_wrire_this_page_to_plc_triggered();

    void on_write_this_value_to_plc_triggered();

    void on_make_pattern_sysparams_triggered();

    void on_update_values_triggered();

    void on_set_language_en_triggered();

    void on_set_language_ru_triggered();

private:
    class TableInputDelegateEmpty;
    Ui::MainWindow *ui;

    QTranslator app_translator_;
    QTableWidget* data_table_;
    QTreeWidget* tree_;
    QPushButton* write_plc_btn_;
    QPushButton* update_values_btn_;
    TS7Client S7_Client_;
    QString s7_plc_address_ = "192.168.88.2";
    byte* byte_buffer_;

    std::list<QString> headers_;
    std::unordered_map<QString*, std::vector<std::optional<S7DataItem>>> header_to_data_;
    std::unordered_map<QTreeWidgetItem*, QString*> item_to_header_tree_;
    std::unordered_map<std::optional<S7DataItem>*, std::shared_ptr<PLCHandlerInterface>> data_item_to_plc_handler_;
    std::list<std::unique_ptr<TableInputDelegateEmpty>> empty_item_delegates_;

    bool read_file_sysparams(QString file_name);
    void fill_table(QTreeWidgetItem* selected_item);
    void read_settings();
    void connect_plc();
    void set_texts_ui();

    class TableInputDelegateEmpty: public QItemDelegate {
    public:
        TableInputDelegateEmpty()
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            QDoubleValidator *validator = new QDoubleValidator(0.0, 0.0, 0, lineEdit);
            validator->setLocale(QLocale::C);
            lineEdit->setValidator(validator);
            return lineEdit;
        }
    };
};
#endif // MAINWINDOW_H
