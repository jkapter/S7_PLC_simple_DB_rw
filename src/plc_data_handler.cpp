#include "plc_data_handler.h"

S7DataType S7Data::from_string(QString str) {
    if(str == "BOOL") return S7DataType::BOOL;
    if(str == "BYTE") return S7DataType::BYTE;
    if(str == "DINT") return S7DataType::DINT;
    if(str == "INT") return S7DataType::INT;
    if(str == "REAL") return S7DataType::REAL;
    if(str == "S5TIME") return S7DataType::S5TIME;
    if(str == "WORD") return S7DataType::WORD;
    if(str == "DWORD") return S7DataType::DWORD;
    if(str == "TIME") return S7DataType::TIME;
    return S7DataType::INVALID;
}

QString S7Data::to_string(S7DataType type) {
    switch(type) {
    case S7DataType::BOOL:
        return "BOOL";
    case S7DataType::BYTE:
        return "BYTE";
    case S7DataType::DINT:
        return "DINT";
    case S7DataType::INT:
        return "INT";
    case S7DataType::REAL:
        return "REAL";
    case S7DataType::S5TIME:
        return "S5TIME";
    case S7DataType::WORD:
        return "WORD";
    case S7DataType::DWORD:
        return "DWORD";
    case S7DataType::TIME:
        return "TIME";
    default: return "INVALID";
    }
}

std::optional<S7DataItem> S7Data::parse_S7_data_line(const QString& str) {
    S7DataItem ret_struct;
    qsizetype beg = 0;
    qsizetype end = str.indexOf(".", beg);
    if(end - 1 <= beg || end - beg > 6) {
        return std::nullopt;
    }
    bool res;
    int val;
    val = str.mid(2, end - 2).toInt(&res);
    if(!res) {
        return std::nullopt;
    } else {
        ret_struct.db_nomber = val;
    }

    beg = end + 4;
    end = str.indexOf('\t', beg);
    if(str.at(beg - 1) == 'X') {
        QString temp = str.mid(beg, end - beg);
        qsizetype e2 = temp.indexOf('.', 0);
        val = temp.mid(0, e2).toInt(&res);
        if(res) {
            val = val << 8;
            int t = temp.mid(e2+1, temp.length() - e2).toInt(&res);
            if(res) {
                val += t;
            }
        }
    } else {
        val = str.mid(beg, end - beg).toInt(&res);
    }

    if(!res) {
        return std::nullopt;
    } else {
        ret_struct.db_offset = val;
    }

    beg = end + 1;
    end = str.indexOf('\t', beg);
    if(S7Data::from_string(str.mid(beg, end - beg)) != S7DataType::INVALID) {
        ret_struct.type = S7Data::from_string(str.mid(beg, end - beg));
    } else {
        return std::nullopt;
    }

    beg = end + 1;
    end = str.indexOf('\t', beg);
    val = str.mid(beg, end - beg).toInt(&res);
    if(!res) {
        return std::nullopt;
    } else {
        ret_struct.min_val = val;
    }

    beg = end + 1;
    end = str.indexOf('\t', beg);
    val = str.mid(beg, end - beg).toInt(&res);
    if(!res) {
        return std::nullopt;
    } else {
        ret_struct.max_val = val;
    }

    beg = end + 1;
    end = str.indexOf('\t', beg);
    val = str.mid(beg, end - beg).toInt(&res);
    if(!res) {
        return std::nullopt;
    } else {
        ret_struct.gain = val;
    }

    beg = end + 1;
    if(str.mid(beg, str.length() - beg).length() > 0) {
        end = str.indexOf("\t=");
        if(end != -1) {
            ret_struct.name = str.mid(beg, end - beg);
            ret_struct.val_to_write = str.mid(end + 2, str.length() - end - 1);
        } else {
            ret_struct.name = str.mid(beg, str.length() - beg);
            ret_struct.val_to_write = "";
        }
    } else {
        return std::nullopt;
    }

    return ret_struct;
}

QString S7Data::print_S7_data(S7DataItem& item) {
    QString ret_str;
    ret_str = QString("DB%1.DB").arg(item.db_nomber);
    switch(item.type) {
    case S7DataType::REAL:
        ret_str += QString("D%1\t").arg(item.db_offset);
        break;
    case S7DataType::DINT:
        ret_str += QString("D%1\t").arg(item.db_offset);
        break;
    case S7DataType::INT:
        ret_str += QString("W%1\t").arg(item.db_offset);
        break;
    case S7DataType::WORD:
        ret_str += QString("W%1\t").arg(item.db_offset);
        break;
    case S7DataType::BYTE:
        ret_str += QString("B%1\t").arg(item.db_offset);
        break;
    case S7DataType::BOOL:
        ret_str += QString("X%1.%2\t").arg(item.db_offset >> 8).arg(item.db_offset & 0xf);
        break;
    default:
        return "";
    }
    ret_str += to_string(item.type);
    ret_str += QString("\t%1\t%2\t%3\t%4").arg(item.min_val).arg(item.max_val).arg(item.gain).arg(item.name);
    return ret_str;
}

std::shared_ptr<PLCHandlerInterface> MakePLCDataHandler::MakeHandler(S7DataItem& data_item, TS7Client& plc) {
    switch(data_item.type) {
    case S7DataType::REAL :
        return std::shared_ptr<PLCHandler_R>(new PLCHandler_R(data_item, plc));
    case S7DataType::INT:
        return std::shared_ptr<PLCHandler_I>(new PLCHandler_I(data_item, plc));
    case S7DataType::DINT:
        return std::shared_ptr<PLCHandler_DI>(new PLCHandler_DI(data_item, plc));
    case S7DataType::BOOL:
        return std::shared_ptr<PLCHandler_BL>(new PLCHandler_BL(data_item, plc));
    case S7DataType::BYTE:
        return std::shared_ptr<PLCHandler_B>(new PLCHandler_B(data_item, plc));
    case S7DataType::WORD:
        return std::shared_ptr<PLCHandler_W>(new PLCHandler_W(data_item, plc));
    case S7DataType::S5TIME:
        return std::shared_ptr<PLCHandler_S5T>(new PLCHandler_S5T(data_item, plc));
    case S7DataType::DWORD:
        return std::shared_ptr<PLCHandler_DW>(new PLCHandler_DW(data_item, plc));
    case S7DataType::TIME:
        return std::shared_ptr<PLCHandler_T>(new PLCHandler_T(data_item, plc));
    }
    return nullptr;
}

PLCHandlerInterface::PLCHandlerInterface(S7DataItem& data_item, TS7Client& plc)
    : plc_(plc)
    , data_item_(data_item)
    , item_input_delegate_(nullptr)
{
}

PLCHandlerInterface::~PLCHandlerInterface() {
    delete item_input_delegate_;
}

QItemDelegate* PLCHandlerInterface::GetInputValidator() {
    return item_input_delegate_;
}

QTableWidgetItem* PLCHandlerInterface::GetQTableItemOutput() {
    QTableWidgetItem* output_item = new QTableWidgetItem();
    output_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter | (Qt::Alignment)Qt::TextWordWrap);
    output_item->setFlags(Qt::NoItemFlags);
    output_item->setFont(QFont("Segoe", 8, QFont::Bold, false));
    if(fail_to_read_) {
        output_item->setBackground(QBrush(Qt::gray));
    }
    output_item->setText(PrintValue());
    return output_item;
}

QTableWidgetItem* PLCHandlerInterface::GetQTableItemInput() {
    QTableWidgetItem* input_item = new QTableWidgetItem();
    input_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter |  (Qt::Alignment)Qt::TextWordWrap);
    input_item->setFont(QFont("Segoe", 8, QFont::Bold, false));
    if(fail_to_read_) {
        input_item->setBackground(QBrush(Qt::gray));
        input_item->setFlags(Qt::NoItemFlags);
    }
    if(SetValueToWrite(data_item_.val_to_write)) {
        input_item->setText(PrintValueToWrite());
    }
    return input_item;
}

PLCHandler_R::PLCHandler_R(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    if(plc_.Connected()) {
        byte buffer[4];
        if(plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 4, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_real_from_buffer(buffer, 0);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0.0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_R::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return QString::number(plc_value_ * static_cast<double>(data_item_.gain), 'f', 2);
}

QString PLCHandler_R::PrintValueToWrite() const {
    return QString::number(value_to_write_ * static_cast<double>(data_item_.gain), 'f', 2);
}

bool PLCHandler_R::SetValueToWrite(QString string_val) {
    if(string_val.length() > 0 && !fail_to_read_) {
        bool res;
        double val = string_val.toDouble(&res);
        if(res && data_item_.gain != 0) {
            value_to_write_ = val / data_item_.gain;
            data_item_.val_to_write = string_val;
            return true;
        }
    }
    return false;
}

bool PLCHandler_R::WriteToPlc() {
    if(std::abs(value_to_write_ - plc_value_) > 1e-5 && plc_.Connected() && !fail_to_read_) {
        byte buffer[4];
        TS7Client::S7_make_real_to_buffer(buffer, 0, value_to_write_);
        int rw = plc_.DBWrite(data_item_.db_nomber, data_item_.db_offset, 4, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 4, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_real_from_buffer(buffer, 0);
        }
        return std::abs(plc_value_ - value_to_write_) < 1e-5 && rw == 0 && rd == 0;
    }
    return false;
}

PLCHandler_I::PLCHandler_I(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    if(plc_.Connected()) {
        byte buffer[2];
        if(plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 2, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_int_from_buffer(buffer, 0);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_I::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return QString::number(plc_value_ * data_item_.gain);
}

QString PLCHandler_I::PrintValueToWrite() const {
    return QString::number(value_to_write_ * data_item_.gain);
}

bool PLCHandler_I::SetValueToWrite(QString string_val) {
    if(string_val.length() > 0 && !fail_to_read_) {
        bool res;
        int val = string_val.toInt(&res);
        if(res && data_item_.gain != 0) {
            value_to_write_ = val / data_item_.gain;
            data_item_.val_to_write = string_val;
            return true;
        }
    }
    return false;
}

bool PLCHandler_I::WriteToPlc() {
    if(value_to_write_ != plc_value_ && plc_.Connected() && !fail_to_read_) {
        byte buffer[4];
        TS7Client::S7_make_int_to_buffer(buffer, 0, value_to_write_);
        int rw = plc_.DBWrite(data_item_.db_nomber, data_item_.db_offset, 2, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 2, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_int_from_buffer(buffer, 0);
        }
        return plc_value_ == value_to_write_ && rw == 0 && rd == 0;
    }
    return false;
}

PLCHandler_DI::PLCHandler_DI(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    if(plc_.Connected()) {
        byte buffer[4];
        if(plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 4, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_dint_from_buffer(buffer, 0);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_DI::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return QString::number(plc_value_ * data_item_.gain);
}

QString PLCHandler_DI::PrintValueToWrite() const {
    return QString::number(value_to_write_ * data_item_.gain);
}

bool PLCHandler_DI::SetValueToWrite(QString string_val) {
    if(string_val.length() > 0 && !fail_to_read_) {
        bool res;
        int val = string_val.toInt(&res);
        if(res && data_item_.gain != 0) {
            value_to_write_ = val / data_item_.gain;
            data_item_.val_to_write = string_val;
            return true;
        }
    }
    return false;
}

bool PLCHandler_DI::WriteToPlc() {
    if(value_to_write_ != plc_value_ && plc_.Connected() && !fail_to_read_) {
        byte buffer[4];
        TS7Client::S7_make_dint_to_buffer(buffer, 0, value_to_write_);
        int rw = plc_.DBWrite(data_item_.db_nomber, data_item_.db_offset, 4, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 4, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_dint_from_buffer(buffer, 0);
        }
        return plc_value_ == value_to_write_ && rw == 0 && rd == 0;
    }
    return false;
}

PLCHandler_B::PLCHandler_B(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    if(plc_.Connected()) {
        byte buffer[2];
        if(plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 1, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_byte_from_buffer(buffer, 0);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_B::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return QString("B#16#" + QString::number(plc_value_, 16)).toUpper();
}

QString PLCHandler_B::PrintValueToWrite() const {
    return QString("B#16#" + QString::number(value_to_write_, 16)).toUpper();
}

bool PLCHandler_B::SetValueToWrite(QString string_val) {
    if(string_val.length() > 6 && !fail_to_read_) {
        bool res;
        uint8_t val = string_val.mid(5, string_val.length() - 5).toUInt(&res, 16);
        if(res) {
            value_to_write_ = val;
            data_item_.val_to_write = string_val;
            return true;
        }
    }
    return false;
}

bool PLCHandler_B::WriteToPlc() {
    if(value_to_write_ != plc_value_ && plc_.Connected() && !fail_to_read_) {
        byte buffer[4];
        buffer[0] = value_to_write_;
        int rw = plc_.DBWrite(data_item_.db_nomber, data_item_.db_offset, 2, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 2, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_byte_from_buffer(buffer, 0);
        }
        return plc_value_ == value_to_write_ && rw == 0 && rd == 0;
    }
    return false;
}

PLCHandler_W::PLCHandler_W(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    if(plc_.Connected()) {
        byte buffer[2];
        if(plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 2, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_word_from_buffer(buffer, 0);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_W::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return QString("W#16#" + QString::number(plc_value_, 16)).toUpper();
}

QString PLCHandler_W::PrintValueToWrite() const {
    return QString("W#16#" + QString::number(value_to_write_, 16)).toUpper();
}

bool PLCHandler_W::SetValueToWrite(QString string_val) {
    if(string_val.length() > 6 && !fail_to_read_) {
        bool res;
        uint16_t val = string_val.mid(5, string_val.length() - 5).toUInt(&res, 16);
        if(res) {
            value_to_write_ = val;
            data_item_.val_to_write = string_val;
            return true;
        }
    }
    return false;
}

bool PLCHandler_W::WriteToPlc() {
    if(value_to_write_ != plc_value_ && plc_.Connected() && !fail_to_read_) {
        byte buffer[4];
        TS7Client::S7_make_word_to_buffer(buffer, 0, value_to_write_);
        int rw = plc_.DBWrite(data_item_.db_nomber, data_item_.db_offset, 2, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 2, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_word_from_buffer(buffer, 0);
        }
        return plc_value_ == value_to_write_ && rw == 0 && rd == 0;
    }
    return false;
}

PLCHandler_BL::PLCHandler_BL(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    n_bit = data_item_.db_offset & 0xf;
    n_byte = data_item_.db_offset >> 8;

    if(plc_.Connected()) {
        byte buffer[2];
        if(plc_.DBRead(data_item_.db_nomber, n_byte, 1, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_bool_from_buffer(buffer, 0, n_bit);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_BL::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return plc_value_ ? "TRUE" : "FALSE";
}

QString PLCHandler_BL::PrintValueToWrite() const {
    return value_to_write_ ? "TRUE" : "FALSE";
}

bool PLCHandler_BL::SetValueToWrite(QString string_val) {
    if(fail_to_read_) return false;
    if(string_val == "TRUE") {
        value_to_write_ = true;
        data_item_.val_to_write = string_val;
        return true;
    }
    if(string_val == "FALSE") {
        value_to_write_ = false;
        return true;
    }
    return false;
}


bool PLCHandler_BL::WriteToPlc() {
    byte buffer[4];
    if(value_to_write_ != plc_value_ && plc_.Connected() && !fail_to_read_) {
        plc_.DBRead(data_item_.db_nomber, n_byte, 1, buffer);
        byte Mask[] = {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};
        buffer[0] = buffer[0] & Mask[n_bit];
        byte b = value_to_write_ ? 1 : 0;
        b = b << n_bit;
        buffer[0] = buffer[0] | b;
        int rw = plc_.DBWrite(data_item_.db_nomber, n_byte, 1, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, n_byte, 1, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_bool_from_buffer(buffer, 0, n_bit);
        }
        return plc_value_ == value_to_write_ && rw == 0 && rd == 0;
    }
    return false;
}

PLCHandler_S5T::PLCHandler_S5T(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    if(plc_.Connected()) {
        byte buffer[2];
        if(plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 2, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_word_from_buffer(buffer, 0);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_S5T::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return print_from_raw_S5T(plc_value_);
}

QString PLCHandler_S5T::PrintValueToWrite() const {
    return print_from_raw_S5T(value_to_write_);
}

bool PLCHandler_S5T::SetValueToWrite(QString string_val) {
    if(string_val.length() > 5 && get_raw_from_string(string_val).has_value() && !fail_to_read_) {
        value_to_write_ = get_raw_from_string(string_val).value();
        return true;
    }
    return false;
}

bool PLCHandler_S5T::WriteToPlc() {
    if(value_to_write_ != plc_value_ && plc_.Connected() && !fail_to_read_) {
        byte buffer[4];
        TS7Client::S7_make_word_to_buffer(buffer, 0, value_to_write_);
        int rw = plc_.DBWrite(data_item_.db_nomber, data_item_.db_offset, 2, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 2, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_word_from_buffer(buffer, 0);
        }
        return plc_value_ == value_to_write_ && rw == 0 && rd == 0;
    }
    return false;
}

QString PLCHandler_S5T::print_from_raw_S5T(uint16_t val) const {
    QString ret_str = "S5T#";
    int time_base; //time base: 10 seconds
    for(int i = 0; i < 3; ++i) {
        if(((val >> i*4) & 0x000f) > 9) {
            return ret_str + "ERROR";
        }
    }
    if((val & 0x3000) == 0x3000) {
        time_base = 10000;
    } else if((val & 0x3000) == 0x2000) {
        time_base = 1000;
    } else if((val & 0x3000) == 0x1000) {
        time_base = 100;
    } else {
        time_base = 10;
    }

    int time_val = (byte) val & 0x000f;
    time_val += (byte)((val >> 4) & 0x000f) * 10;
    time_val += (byte)((val >> 8) & 0x000f) * 100;
    time_val *= time_base; // время в мс
    if(time_val / 3600000 > 0) {
        ret_str.append(QString("%1h").arg(time_val / (1000*60*60)));
        time_val -= (time_val / 3600000) * 3600000;
    }
    if(time_val / 60000 > 0) {
        ret_str.append(QString("%1m").arg(time_val / (1000*60)));
        time_val -= (time_val / 60000) * 60000;
    }
    if(time_val / 1000 > 0) {
        ret_str.append(QString("%1s").arg(time_val / 1000));
        time_val -= (time_val / 1000) * 1000;
    }
    if(time_val > 0) {
        ret_str.append(QString("%1ms").arg(time_val));
    }
    return ret_str;
}

std::optional<uint16_t> PLCHandler_S5T::get_raw_from_string(QString val) const {

    QRegularExpression regexp("^S5T#?([0-2]h)?([0-5]?\\dm)?([0-5]?\\ds)?(\\d{1,3}ms)?$");
    QRegularExpressionMatch m = regexp.match(val);
    int time_duration = 0;
    uint16_t time_base, time_base_code = 0x3000;
    if(m.hasMatch()) {
        QString str = val;
        str.remove(0, 4);
        bool res;
        qsizetype mid = str.indexOf('h');
        if(mid > 0) {
            int v = str.mid(0, mid).toInt(&res);
            if(res && v < 3) {
                time_duration += v * 3600000;
                str.remove(0, mid + 1);
            } else {
                return std::nullopt;
            }
        }
        mid = str.indexOf('m');
        if(mid > 0 && mid != str.indexOf("ms")) {
            int v = str.mid(0, mid).toInt(&res);
            if(res && v < 60) {
                time_duration += v * 60000;
                str.remove(0, mid + 1);
            } else {
                return std::nullopt;
            }
        }
        mid = str.indexOf('s');
        if(mid > 1 && mid - 1 != str.indexOf("ms")) {
            int v = str.mid(0, mid).toInt(&res);
            if(res && v < 60) {
                time_duration += v * 1000;
                str.remove(0, mid + 1);
            } else {
                return std::nullopt;
            }
        }
        mid = str.indexOf("ms");
        if(mid > 0) {
            int v = str.mid(0, mid).toInt(&res);
            if(res && v < 1000) {
                time_duration += v;
                str.remove(0, mid + 2);
            } else {
                return std::nullopt;
            }
        }

        if(str.length() > 0) {
            return std::nullopt;
        }

        time_base = 10000;
        if(time_duration <= 999000) {
            time_base = 1000;
            time_base_code = 0x2000;
        }
        if(time_duration <= 99900) {
            time_base = 100;
            time_base_code = 0x1000;
        }
        if(time_duration <= 9990) {
            time_base = 10;
            time_base_code = 0x0;
        }

        time_duration /= time_base;
        uint16_t time_duration_code = 0;
        time_duration = time_duration & 0x0fff;
        time_duration_code |= time_duration / 100 > 9 ? 9 : (time_duration / 100);
        time_duration_code = time_duration_code << 4;
        time_duration = time_duration % 100;
        time_duration_code = time_duration_code | (time_duration / 10);
        time_duration_code = time_duration_code << 4;
        time_duration = time_duration % 10;
        time_duration_code = time_duration_code | (time_duration);

        return time_base_code | time_duration_code;
    } else {
        return std::nullopt;
    }
}

PLCHandler_DW::PLCHandler_DW(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    if(plc_.Connected()) {
        byte buffer[4];
        if(plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 4, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_dword_from_buffer(buffer, 0);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_DW::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return QString("DW#16#" + QString::number(plc_value_, 16)).toUpper();
}

QString PLCHandler_DW::PrintValueToWrite() const {
    return QString("DW#16#" + QString::number(value_to_write_, 16)).toUpper();
}

bool PLCHandler_DW::SetValueToWrite(QString string_val) {
    if(string_val.length() > 7 && !fail_to_read_) {
        bool res;
        uint32_t val = string_val.mid(6, string_val.length() - 6).toUInt(&res, 16);
        if(res) {
            value_to_write_ = val;
            data_item_.val_to_write = string_val;
            return true;
        }
    }
    return false;
}

bool PLCHandler_DW::WriteToPlc() {
    if(value_to_write_ != plc_value_ && plc_.Connected() && !fail_to_read_) {
        byte buffer[4];
        TS7Client::S7_make_dword_to_buffer(buffer, 0, value_to_write_);
        int rw = plc_.DBWrite(data_item_.db_nomber, data_item_.db_offset, 4, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 4, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_dword_from_buffer(buffer, 0);
        }
        return plc_value_ == value_to_write_ && rw == 0 && rd == 0;
    }
    return false;
}

PLCHandler_T::PLCHandler_T(S7DataItem& data_item, TS7Client& plc): PLCHandlerInterface(data_item, plc){
    item_input_delegate_ = new TableInputDelegate(data_item_);

    if(plc_.Connected()) {
        byte buffer[4];
        if(plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 4, buffer) == 0) {
            plc_value_ = TS7Client::S7_get_dint_from_buffer(buffer, 0);
            fail_to_read_ = false;
        } else {
            fail_to_read_ = true;
        }
    } else {
        plc_value_ = 0;
    }
    value_to_write_ = plc_value_;
}

QString PLCHandler_T::PrintValue() const {
    if(fail_to_read_) return QString("--");
    return print_from_raw_T(plc_value_);
}

QString PLCHandler_T::PrintValueToWrite() const {
    return print_from_raw_T(value_to_write_);
}

bool PLCHandler_T::SetValueToWrite(QString string_val) {
    if(string_val.length() > 2 && get_raw_from_string(string_val).has_value() && !fail_to_read_) {
        value_to_write_ = get_raw_from_string(string_val).value();
        return true;
    }
    return false;
}

bool PLCHandler_T::WriteToPlc() {
    if(value_to_write_ != plc_value_ && plc_.Connected() && !fail_to_read_) {
        byte buffer[4];
        TS7Client::S7_make_dint_to_buffer(buffer, 0, value_to_write_);
        int rw = plc_.DBWrite(data_item_.db_nomber, data_item_.db_offset, 4, buffer);
        int rd = plc_.DBRead(data_item_.db_nomber, data_item_.db_offset, 4, buffer);
        if(rd == 0) {
            plc_value_ = TS7Client::S7_get_dint_from_buffer(buffer, 0);
        }
        return plc_value_ == value_to_write_ && rw == 0 && rd == 0;
    }
    return false;
}

QString PLCHandler_T::print_from_raw_T(int val) const {
    QString ret_str = "T#";
    if(val < 0) {
        return ret_str + "ERROR";
    }
    int time_val = val;
    if(time_val / (1000*60*60*24) > 0) {
        ret_str.append(QString("%1d").arg(time_val / (1000*60*60*24)));
        time_val -= (time_val / (1000*60*60*24)) * 1000*60*60*24;
    }
    if(time_val / (1000*60*60) > 0) {
        ret_str.append(QString("%1h").arg(time_val / (1000*60*60)));
        time_val -= (time_val / (1000*60*60)) * 1000*60*60;
    }
    if(time_val / (60*1000) > 0) {
        ret_str.append(QString("%1m").arg(time_val / (60*1000)));
        time_val -= (time_val / (60*1000)) * 60*1000;
    }
    if(time_val / 1000 > 0) {
        ret_str.append(QString("%1s").arg(time_val / 1000));
        time_val -= (time_val / 1000) * 1000;
    }
    if(time_val > 0) {
        ret_str.append(QString("%1ms").arg(time_val));
    }
    return ret_str;
}

std::optional<int> PLCHandler_T::get_raw_from_string(QString val) const {
    QRegularExpression regexp("^T#?([0-2]?\\dd)?(\\d{1,2}h)?([0-5]?\\dm)?([0-5]?\\ds)?(\\d{1,3}ms)?$");
    QRegularExpressionMatch m = regexp.match(val);
    int time_duration = 0;
    if(m.hasMatch()) {
        QString str = val;
        str.remove(0, 2);
        bool res;
        qsizetype mid = str.indexOf('d');
        if(mid > 0) {
            int v = str.mid(0, mid).toInt(&res);
            if(res) {
                time_duration += v * 1000*60*60*24;
                str.remove(0, mid + 1);
            } else {
                return std::nullopt;
            }
        }
        mid = str.indexOf('h');
        if(mid > 0) {
            int v = str.mid(0, mid).toInt(&res);
            if(res) {
                time_duration += v * 3600000;
                str.remove(0, mid + 1);
            } else {
                return std::nullopt;
            }
        }
        mid = str.indexOf('m');
        if(mid > 0 && mid != str.indexOf("ms")) {
            int v = str.mid(0, mid).toInt(&res);
            if(res && v < 60) {
                time_duration += v * 60000;
                str.remove(0, mid + 1);
            } else {
                return std::nullopt;
            }
        }
        mid = str.indexOf('s');
        if(mid > 1 && mid-1 != str.indexOf("ms")) {
            int v = str.mid(0, mid).toInt(&res);
            if(res && v < 60) {
                time_duration += v * 1000;
                str.remove(0, mid + 1);
            } else {
                return std::nullopt;
            }
        }
        mid = str.indexOf("ms");
        if(mid > 0) {
            int v = str.mid(0, mid).toInt(&res);
            if(res && v < 1000) {
                time_duration += v;
                str.remove(0, mid + 2);
            } else {
                return std::nullopt;
            }
        }

        if(str.length() > 0) {
            return std::nullopt;
        }
        return time_duration;
    } else {
        return std::nullopt;
    }
}
