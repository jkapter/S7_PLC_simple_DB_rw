#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , byte_buffer_(new byte[480])
{
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(ui->la_plc_status, 1);
    ui->statusbar->addPermanentWidget(ui->la_status_bar_message, 4);
    ui->la_status_bar_message->setText("");


    QGridLayout* grl = new QGridLayout(ui->centralwidget);
    tree_ = new QTreeWidget(this);
    data_table_ = new QTableWidget(this);


    data_table_->verticalHeader()->setVisible(false);
    data_table_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    write_plc_btn_ = new QPushButton(this);
    write_plc_btn_->setMinimumHeight(45);
    update_values_btn_ = new QPushButton(this);
    update_values_btn_->setMinimumHeight(45);
    QLabel* logo_lab = new QLabel(this);
    QPixmap logo_pix (":/img/logo_162x43.png");
    logo_lab->setPixmap(logo_pix);

    grl->addWidget(tree_, 0, 0, 2, 1);
    grl->addWidget(write_plc_btn_, 0, 1, 1, 1);
    grl->addWidget(update_values_btn_, 0, 2, 1, 1);
    grl->addWidget(data_table_, 1, 1, 1, 3);
    grl->addWidget(logo_lab, 0, 3);
    grl->setColumnStretch(0, 10);
    grl->setColumnStretch(1, 10);
    grl->setColumnStretch(2, 10);

    data_table_->setColumnCount(3);
    data_table_->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | (Qt::Alignment)Qt::TextWordWrap);
    data_table_->horizontalHeader()->setFixedHeight(40);

    tree_->setColumnCount(1);
    tree_->setHeaderHidden(true);

    tree_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tree_->header()->setStretchLastSection(false);
    tree_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(tree_, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(tree_item_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(write_plc_btn_, SIGNAL(pressed()), this, SLOT(write_to_plc_btn_clicked()));
    connect(update_values_btn_, SIGNAL(pressed()), this, SLOT(on_update_values_triggered()));
    connect(data_table_, SIGNAL(cellChanged(int,int)), this, SLOT(table_cell_changed(int,int)));

    read_settings();
    connect_plc();

    set_texts_ui();

    if(!read_file_sysparams("sysparams.txt")) {
        ui->la_status_bar_message->setText(tr("Файл \"sysparams. txt\" не найден!"));
    } else {
        bool fs = true;
        for(auto& it: headers_) {
            QTreeWidgetItem* tree_item = new QTreeWidgetItem(QStringList(it));
            item_to_header_tree_[tree_item] = &it;
            tree_->addTopLevelItem(tree_item);
            if(fs) {
                tree_->setCurrentItem(tree_item, 0, QItemSelectionModel::Select);
                fs = false;
            }
        }
    }

    QTimer::singleShot(1, this, SLOT(window_show_up()));
}

MainWindow::~MainWindow()
{
    QJsonObject temp_obj;
    temp_obj.insert("plc_ip", s7_plc_address_);
    temp_obj.insert("window_h", this->height());
    temp_obj.insert("window_w", this->width());
    temp_obj.insert("window_left", this->geometry().left());
    temp_obj.insert("window_top", this->geometry().top());
    temp_obj.insert("ui_language", ui->menu->title() == "File" ? "en" : "ru");

    QJsonDocument output_doc(temp_obj);

    QFile output_file("settings.json");
    if(output_file.open(QIODeviceBase::WriteOnly)) {
        output_file.write(output_doc.toJson());
        output_file.close();
    }

    delete ui;
    delete byte_buffer_;
}

void MainWindow::set_texts_ui() {
    write_plc_btn_->setText(tr("Записать значения в ПЛК"));
    update_values_btn_->setText(tr("Обновить значения"));
    data_table_->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Наименование параметра")));
    data_table_->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Значение в ПЛК")));
    data_table_->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Для записи")));
    ui->la_status_bar_message->setText(tr("Язык приложения изменен."));
    if(ui->la_plc_status->palette().color(QPalette::Window) == Qt::darkGreen) {
        ui->la_plc_status->setText(tr("ПЛК ОК!"));
    } else {
        ui->la_plc_status->setText(tr("ПЛК недоступен!"));
    }
}

void MainWindow::tree_item_changed(QTreeWidgetItem* cur, QTreeWidgetItem* prev) {
    fill_table(cur);
}

bool MainWindow::read_file_sysparams(QString file_name) {
    headers_.clear();
    header_to_data_.clear();
    item_to_header_tree_.clear();
    data_item_to_plc_handler_.clear();

    QFile file(file_name);

    if(!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    int i = 0;
    QTextStream in(&file);
    std::vector<std::optional<S7DataItem>>* data_vec = nullptr;
    while(!in.atEnd()) {
        QString str = in.readLine();
        if(str.length() == 0) {
            continue;
        }
        if(str.front() == '[') {
            qsizetype rght = str.indexOf("]", 1);
            if(rght > 2) {
                headers_.push_back(std::move(str.mid(1, rght - 1)));
                data_vec = &header_to_data_[&headers_.back()];
            } else {
                data_vec = nullptr;
                return false;
            }
        }
        if(str.front() == 'D') {
            if(data_vec) {
                data_vec->push_back(std::move(S7Data::parse_S7_data_line(str)));
                ++i;
            }
        }
    }
    for(auto& [header_ptr, vec_opt_items]: header_to_data_) {
        for(auto& item: vec_opt_items) {
            if(item.has_value()) {
                data_item_to_plc_handler_[&item] = MakePLCDataHandler::MakeHandler(item.value(), S7_Client_);
            } else {
                data_item_to_plc_handler_[&item] = nullptr;
            }
        }
    }
    ui->la_status_bar_message->setText(QString(tr("Файл \"%1\" прочитан. Получено %2 разделов, %3 параметров").arg(file_name).arg(headers_.size()).arg(i)));
    return true;
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    int w1, w2, w3;
    if(data_table_->horizontalHeader()->geometry().width() > 320) {
        w2 = 120;
        w3 = 120;
        w1 = data_table_->horizontalHeader()->geometry().width() - w3 - w2 - 1;
    } else {
        w1 = data_table_->horizontalHeader()->geometry().width()*3 / 5;
        w2 = data_table_->horizontalHeader()->geometry().width() / 5;
        w3 = data_table_->horizontalHeader()->geometry().width() - w1 - w2;
    }
    data_table_->setColumnWidth(0, w1);
    data_table_->setColumnWidth(1, w2);
    data_table_->setColumnWidth(2, w3);
}

void MainWindow::fill_table(QTreeWidgetItem* selected_item) {
    if(item_to_header_tree_.count(selected_item) == 0 || header_to_data_.count(item_to_header_tree_.at(selected_item)) == 0) {
        return;
    }

    connect_plc();

    data_table_->clearContents();
    std::vector<std::optional<S7DataItem>>* vec = &header_to_data_.at(item_to_header_tree_.at(selected_item));
    data_table_->setRowCount(vec->size());
    data_table_->setColumnCount(3);
    for(int i = 0; i < vec->size(); ++i) {
        QTableWidgetItem* name_item = new QTableWidgetItem();
        name_item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter | (Qt::Alignment)Qt::TextWordWrap);
        name_item->setFont(QFont("Segoe", 8, QFont::Bold, false));
        name_item->setBackground(QBrush(Qt::white));
        name_item->setForeground(QBrush(Qt::black));
        name_item->setFlags(Qt::NoItemFlags);
        if(vec->at(i).has_value()) {
            name_item->setText(QString("%1 (%2..%3)").arg(vec->at(i).value().name).arg(vec->at(i).value().min_val).arg(vec->at(i).value().max_val));
            data_table_->setItem(i, 1, data_item_to_plc_handler_.at(&vec->at(i))->GetQTableItemOutput());
            data_table_->setItem(i, 2, data_item_to_plc_handler_.at(&vec->at(i))->GetQTableItemInput());
            data_table_->setItemDelegateForRow(i, data_item_to_plc_handler_.at(&vec->at(i))->GetInputValidator());
        } else {
            name_item->setText(tr("Неверный формат строки параметра!"));
            name_item->setFont(QFont("Segoe", 8, QFont::Bold, false));
            name_item->setBackground(QBrush(Qt::red));
            name_item->setForeground(QBrush(Qt::white));
            data_table_->setItem(i, 1, new QTableWidgetItem());
            data_table_->item(i, 1)->setFlags(Qt::NoItemFlags);
            data_table_->setItem(i, 2, new QTableWidgetItem());
            data_table_->item(i, 2)->setFlags(Qt::NoItemFlags);
            empty_item_delegates_.push_back(std::unique_ptr<TableInputDelegateEmpty>(new TableInputDelegateEmpty()));
            data_table_->setItemDelegateForRow(i, empty_item_delegates_.back().get());
        }
        data_table_->setItem(i, 0, name_item);
        ui->la_status_bar_message->setText(QString(tr("Прочитано %1 строк.").arg(i + 1)));
    }
    S7_Client_.Disconnect();
}

void MainWindow::read_settings() {

    QFile input_file("settings.json");
    if(!input_file.open(QIODeviceBase::ReadOnly)) {
        ui->la_status_bar_message->setText(tr("Не найден файл настроек!"));
        return;
    }

    QJsonParseError json_error;
    QJsonDocument input_doc = QJsonDocument::fromJson(input_file.read(2000), &json_error);

    if(json_error.error != QJsonParseError::NoError) {
        ui->la_status_bar_message->setText(tr("Файл настроек поврежден!"));
        return;
    }

    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegularExpression ipRegex ("^" + ipRange
                               + "\\." + ipRange
                               + "\\." + ipRange
                               + "\\." + ipRange + "$");
    if(ipRegex.match(input_doc.object().value("plc_ip").toString()).hasMatch()) {
        s7_plc_address_ = input_doc.object().value("plc_ip").toString();
    } else {
        ui->la_status_bar_message->setText(tr("Файл настроек поврежден!"));
        return;
    }

    if(input_doc.object().value("window_h").isDouble() && input_doc.object().value("window_w").isDouble()
        && input_doc.object().value("window_top").isDouble() && input_doc.object().value("window_left").isDouble()
        && input_doc.object().value("window_h").toDouble() > 150 && input_doc.object().value("window_w").toDouble() > 200) {
        this->setGeometry(QRect(input_doc.object().value("window_left").toInt(), input_doc.object().value("window_top").toInt(),
                                input_doc.object().value("window_w").toInt(), input_doc.object().value("window_h").toInt()));
    }

    if((input_doc.object().value("ui_language").isString() && input_doc.object().value("ui_language").toString() == "en") ||
       !QLocale::languageToString(QLocale::system().language()).contains("russian", Qt::CaseInsensitive))
    {
        if(app_translator_.load(":/S7_DB_reader_ru_en")) {
            QCoreApplication::installTranslator(&app_translator_);
            ui->retranslateUi(this);
        }
    }
}

void MainWindow::connect_plc() {
    ui->la_status_bar_message->setText(tr("Чтение данных из ПЛК."));
    for(int i = 0; i < 2 && !S7_Client_.Connected(); ++i) {
        QByteArray ba = s7_plc_address_.toLocal8Bit();
        S7_Client_.ConnectTo(ba.data(), 0, 1);
        QThread::msleep(5);
    }

    QPalette palette = ui->la_plc_status->palette();
    palette.setColor(ui->la_plc_status->foregroundRole(), Qt::white);
    ui->la_plc_status->setAutoFillBackground(true);

    if(S7_Client_.Connected()) {
        ui->la_plc_status->setText(tr("ПЛК ОК!"));
        palette.setColor(ui->la_plc_status->backgroundRole(), Qt::darkGreen);
        ui->la_status_bar_message->setText(tr("ПЛК подключен!"));
    } else {
        ui->la_plc_status->setText(tr("ПЛК нет подключения!"));
        palette.setColor(ui->la_plc_status->backgroundRole(), Qt::darkRed);
        ui->la_status_bar_message->setText(tr("ПЛК недоступен."));
    }
    ui->la_plc_status->setPalette(palette);
}

void MainWindow::write_to_plc_btn_clicked() {

    connect_plc();

    if(!S7_Client_.Connected()) {
        return;
    }

    if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, tr("Запись значений в ПЛК"), tr("Записать новые значения в ПЛК?"), QMessageBox::Yes|QMessageBox::No).exec())
    {
        int i = 0;
        if(tree_->currentItem() && item_to_header_tree_.count(tree_->currentItem()) != 0) {
            for(auto& it: header_to_data_.at(item_to_header_tree_.at(tree_->currentItem()))) {
                if(it.has_value()) {
                    i += data_item_to_plc_handler_.at(&it)->WriteToPlc();
                }
            }
        }
        fill_table(tree_->currentItem());
        ui->la_status_bar_message->setText(QString(tr("Записано в ПЛК %1 значений.")).arg(i));
    }
    S7_Client_.Disconnect();
}

void MainWindow::window_show_up() {
    this->resizeEvent(nullptr);

}


void MainWindow::on_write_params_to_file_triggered()
{
    HintInputDialog* input_dialog_ = new HintInputDialog(this);
    input_dialog_->setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
    input_dialog_->setWindowModality(Qt::WindowModality::WindowModal);
    input_dialog_->setWindowTitle(tr("Записать комментарий для файла параметров"));
    input_dialog_->setGeometry(this->pos().rx() + 200, this->pos().ry() + 100, 400, 200);
    QObject::connect(input_dialog_, SIGNAL(applied(QString)), this, SLOT(get_hint_for_file_params(QString)));
    input_dialog_->show();
}

void MainWindow::table_cell_changed(int r, int c) {
    if(c == 2 && tree_->currentItem() && header_to_data_.at(item_to_header_tree_.at(tree_->currentItem())).size() > r
        && header_to_data_.at(item_to_header_tree_.at(tree_->currentItem())).at(r).has_value()) {
        data_item_to_plc_handler_.at(&header_to_data_.at(item_to_header_tree_.at(tree_->currentItem())).at(r))->SetValueToWrite(data_table_->item(r, c)->text());
    }
}

void MainWindow::get_hint_for_file_params(QString hint) {
    QFile file_to_save_params(QFileDialog::getSaveFileName(this, tr("Сохранить как..."), QDir::currentPath(), "TXT Files (*.txt)"));
    if(!file_to_save_params.open(QIODeviceBase::WriteOnly)) {
        ui->la_status_bar_message->setText(QString(tr("Не удалось открыть файл \"%1\" для записи!").arg(file_to_save_params.fileName())));
        return;
    }

    on_update_values_triggered();

    QTextStream log_file_stream(&file_to_save_params);
    QRegularExpression reg("\n|\r\n|\r");
    for(const auto& it: hint.split(reg)) {
        log_file_stream << "\\\\" << it << "\r\n";
    }

    for(auto& it: headers_) {
        log_file_stream << "\r\n[" << it << "]\r\n";
        for(auto& it_data: header_to_data_.at(&it)) {
            if(it_data.has_value()) {
                log_file_stream << S7Data::print_S7_data(it_data.value()) <<  "\t=" << data_item_to_plc_handler_.at(&it_data)->PrintValue() << "\r\n";
            }
        }
    }

    log_file_stream.flush();
    file_to_save_params.close();
}

void MainWindow::on_read_params_from_file_triggered()
{
    QFile input_file_params(QFileDialog::getOpenFileName(this, tr("Открыть файл..."), QDir::currentPath(), "TXT Files (*.txt)"));
    if(!input_file_params.open(QIODeviceBase::ReadOnly)) {
        ui->la_status_bar_message->setText(QString(tr("Не удалось открыть файл \"%1\" для чтения!")).arg(input_file_params.fileName()));
        return;
    }

    if(!read_file_sysparams(input_file_params.fileName())) {
        ui->la_status_bar_message->setText(QString(tr("Файл \"%1\" не найден!")).arg(input_file_params.fileName()));
    } else {
        bool fs = true;
        tree_->clear();
        for(auto& it: headers_) {
            QTreeWidgetItem* tree_item = new QTreeWidgetItem(QStringList(it));
            item_to_header_tree_[tree_item] = &it;
            tree_->addTopLevelItem(tree_item);
            if(fs) {
                tree_->setCurrentItem(tree_item, 0, QItemSelectionModel::Select);
                fs = false;
            }
        }
    }
}


void MainWindow::on_write_all_params_to_plc_triggered()
{
    connect_plc();

    if(!S7_Client_.Connected()) {
        return;
    }

    if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, tr("Запись значений в ПЛК"), tr("Записать все значения в ПЛК?"), QMessageBox::Yes|QMessageBox::No).exec())
    {
        int i = 0;
        for(auto& [item_ptr, plc_ptr]: data_item_to_plc_handler_) {
            if(plc_ptr) {
                i += plc_ptr->WriteToPlc();
            }
        }
        fill_table(tree_->currentItem());
        ui->la_status_bar_message->setText(QString(tr("Записано в ПЛК %1 значений.")).arg(i));
    }
}


void MainWindow::on_wrire_this_page_to_plc_triggered()
{
    connect_plc();
    write_to_plc_btn_clicked();
    S7_Client_.Disconnect();
}


void MainWindow::on_write_this_value_to_plc_triggered()
{
    connect_plc();

    if(!S7_Client_.Connected()) {
        return;
    }

    if (!QMessageBox::Yes == QMessageBox(QMessageBox::Information, tr("Запись значений в ПЛК"), tr("Записать выбранные значения в ПЛК?"), QMessageBox::Yes|QMessageBox::No).exec()) {
        return;
    }

    int j = 0;
    if(data_table_->selectedRanges().size() > 0) {
        for(const auto& it: data_table_->selectedRanges()) {
            for(int i = it.topRow(); i < it.bottomRow() + 1; ++i) {
                if(tree_->currentItem() && header_to_data_.at(item_to_header_tree_.at(tree_->currentItem())).at(i).has_value()) {
                    j += data_item_to_plc_handler_.at(&header_to_data_.at(item_to_header_tree_.at(tree_->currentItem())).at(i))->WriteToPlc();
                }
            }
        }
        fill_table(tree_->currentItem());
        ui->la_status_bar_message->setText(QString(tr("Записано в ПЛК %1 значений.")).arg(j));

    } else {
        ui->la_status_bar_message->setText(QString(tr("Строки не выбраны.")));
    }
    S7_Client_.Disconnect();

}

void MainWindow::on_make_pattern_sysparams_triggered()
{
    QFile file;
    if(ui->menu->title() == "File") {
        file.setFileName(":/img/sysparams_en.txt");
    } else {
        file.setFileName(":/img/sysparams_ru.txt");
    }
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);

        QString filename = "sysparams";
        QFile file_to_write(filename + ".txt");
        while(file_to_write.exists()) {
            filename = "_" + filename;
            file_to_write.setFileName(filename + ".txt");
        }

        if(file_to_write.open(QIODevice::WriteOnly)) {
            QTextStream out(&file_to_write);
            while(!in.atEnd()) {
                out << in.readLine() << "\r\n";
            }
            out.flush();
            file_to_write.close();
            ui->la_status_bar_message->setText(QString(tr("Шаблон записан в файл \"%1\".")).arg(filename + ".txt"));
        } else {
            ui->la_status_bar_message->setText(QString(tr("Не удалось создать файл \"%1\".")).arg(filename + ".txt"));
        }
        file.close();
    } else {
        ui->la_status_bar_message->setText(QString(tr("Не удалось создать шаблон.")));
    }
}


void MainWindow::on_update_values_triggered()
{
    connect_plc();

    if(S7_Client_.Connected()) {
        data_item_to_plc_handler_.clear();
        data_table_->clearContents();
        for(auto& [header_ptr, vec_opt_items]: header_to_data_) {
            for(auto& item: vec_opt_items) {
                if(item.has_value()) {
                    data_item_to_plc_handler_[&item] = MakePLCDataHandler::MakeHandler(item.value(), S7_Client_);
                } else {
                    data_item_to_plc_handler_[&item] = nullptr;
                }
            }
        }
        ui->la_status_bar_message->setText(QString(tr("Значения обновлены.")));
        if(tree_->currentItem()) {
            fill_table(tree_->currentItem());
        }
        S7_Client_.Disconnect();
    } else {
        ui->la_status_bar_message->setText(QString(tr("ПЛК недоступен.")));
    }
    S7_Client_.Disconnect();
}


void MainWindow::on_set_language_en_triggered()
{
    QCoreApplication::removeTranslator(&app_translator_);
    if(app_translator_.load(":/S7_DB_reader_ru_en")) {
        QCoreApplication::installTranslator(&app_translator_);
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event) {
        switch(event->type()) {
        // When the translator is loaded this event is send.
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            set_texts_ui();
            break;
        // Whem the system language changes this event is send.
        case QEvent::LocaleChange:
            //retranslate the ui.
            break;
        default:
            break;
        }
    }
    QMainWindow::changeEvent(event);
}


void MainWindow::on_set_language_ru_triggered()
{
    QCoreApplication::removeTranslator(&app_translator_);
}

