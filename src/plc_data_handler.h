#ifndef PLC_DATA_HANDLER_H
#define PLC_DATA_HANDLER_H

#include <QTableWidgetItem>
#include <QItemDelegate>
#include <QLineEdit>
#include <QComboBox>
#include <QRegularExpressionValidator>

#include "snap7/snap7.h"

enum class S7DataType {
    BOOL,
    BYTE,
    WORD,
    INT,
    S5TIME,
    DINT,
    REAL,
    DWORD,
    TIME,
    INVALID
};

struct S7DataItem {
    QString name;
    size_t db_nomber;
    size_t db_offset;
    S7DataType type;
    int min_val;
    int max_val;
    int gain;
    QString val_to_write;
};

namespace S7Data {
S7DataType from_string(QString str);
std::optional<S7DataItem> parse_S7_data_line(const QString& str);
QString to_string(S7DataType);
QString print_S7_data(S7DataItem& item);
}

class PLCHandlerInterface;

class MakePLCDataHandler {
public:
    MakePLCDataHandler() = delete;
    static std::shared_ptr<PLCHandlerInterface> MakeHandler(S7DataItem& data_item, TS7Client& plc);
};

class PLCHandlerInterface
{
public:
    PLCHandlerInterface(S7DataItem& data_item, TS7Client& plc);
    QTableWidgetItem* GetQTableItemOutput();
    QTableWidgetItem* GetQTableItemInput();
    QItemDelegate* GetInputValidator();
    virtual bool WriteToPlc() = 0;
    virtual bool SetValueToWrite(QString string_val) = 0;
    virtual QString PrintValue() const = 0;
    virtual QString PrintValueToWrite() const = 0;
    ~PLCHandlerInterface();

protected:
    TS7Client& plc_;
    S7DataItem data_item_;
    QItemDelegate* item_input_delegate_;
    bool fail_to_read_ = false;

};

class PLCHandler_R: public PLCHandlerInterface
{
public:
    PLCHandler_R(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    QString PrintValueToWrite() const override;
    bool SetValueToWrite(QString string_val) override;

private:
    double plc_value_;
    double value_to_write_;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            QDoubleValidator *validator = new QDoubleValidator(data_item_.min_val, data_item_.max_val, 2, lineEdit);
            validator->setLocale(QLocale::C);
            lineEdit->setValidator(validator);
            return lineEdit;
        }
    private:
        S7DataItem& data_item_;
    };
};

class PLCHandler_I: public PLCHandlerInterface
{
public:
    PLCHandler_I(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    bool SetValueToWrite(QString string_val) override;
    QString PrintValueToWrite() const override;
private:
    int plc_value_;
    int value_to_write_;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            QIntValidator *validator = new QIntValidator(data_item_.min_val, data_item_.max_val, lineEdit);
            lineEdit->setValidator(validator);
            return lineEdit;
        }
    private:
        S7DataItem& data_item_;
    };
};

class PLCHandler_DI: public PLCHandlerInterface
{
public:
    PLCHandler_DI(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    bool SetValueToWrite(QString string_val) override;
    QString PrintValueToWrite() const override;
private:
    int plc_value_;
    int value_to_write_;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            QIntValidator *validator = new QIntValidator(data_item_.min_val, data_item_.max_val, lineEdit);

            lineEdit->setValidator(validator);
            return lineEdit;
        }
    private:
        S7DataItem& data_item_;
    };
};

class PLCHandler_B: public PLCHandlerInterface
{
public:
    PLCHandler_B(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    bool SetValueToWrite(QString string_val) override;
    QString PrintValueToWrite() const override;
private:
    uint8_t plc_value_;
    uint8_t value_to_write_;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            lineEdit->setInputMask("\\B\\#16\\#Hh");
            return lineEdit;
        }
    private:
        S7DataItem& data_item_;
    };
};

class PLCHandler_W: public PLCHandlerInterface
{
public:
    PLCHandler_W(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    bool SetValueToWrite(QString string_val) override;
    QString PrintValueToWrite() const override;
private:
    uint16_t plc_value_;
    uint16_t value_to_write_;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            lineEdit->setInputMask("W\\#16\\#Hhhh");
            return lineEdit;
        }

    private:
        S7DataItem& data_item_;
    };
};

class PLCHandler_BL: public PLCHandlerInterface
{
public:
    PLCHandler_BL(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    bool SetValueToWrite(QString string_val) override;
    QString PrintValueToWrite() const override;
private:
    bool plc_value_;
    bool value_to_write_;
    int n_byte;
    int n_bit;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QComboBox *comboBox = new QComboBox(parent);
            comboBox->addItem("TRUE");
            comboBox->addItem("FALSE");
            return comboBox;
        }
    private:
        S7DataItem& data_item_;
    };
};

class PLCHandler_S5T: public PLCHandlerInterface
{
public:
    PLCHandler_S5T(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    bool SetValueToWrite(QString string_val) override;
    QString PrintValueToWrite() const override;
private:
    uint16_t plc_value_;
    uint16_t value_to_write_;

    QString print_from_raw_S5T(uint16_t val) const;
    std::optional<uint16_t> get_raw_from_string(QString val) const;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            QRegularExpression regexp("^S5T#?([0-2]h)?([0-5]?\\dm)?([0-5]?\\ds)?(\\d{1,3}ms)?$");
            QRegularExpressionValidator* validator = new QRegularExpressionValidator(regexp, parent);
            //lineEdit->setInputMask("S5T\\#NNnnnnnnnnnnnn");
            lineEdit->setValidator(validator);
            return lineEdit;
        }

    private:
        S7DataItem& data_item_;
    };
};

class PLCHandler_DW: public PLCHandlerInterface
{
public:
    PLCHandler_DW(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    bool SetValueToWrite(QString string_val) override;
    QString PrintValueToWrite() const override;
private:
    uint32_t plc_value_;
    uint32_t value_to_write_;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            lineEdit->setInputMask("DW\\#16\\#Hhhhhhhh");
            return lineEdit;
        }

    private:
        S7DataItem& data_item_;
    };
};

class PLCHandler_T: public PLCHandlerInterface
{
public:
    PLCHandler_T(S7DataItem& data_item, TS7Client& plc);
    bool WriteToPlc() override;
    QString PrintValue() const override;
    bool SetValueToWrite(QString string_val) override;
    QString PrintValueToWrite() const override;
private:
    int plc_value_;
    int value_to_write_;

    QString print_from_raw_T(int val) const;
    std::optional<int> get_raw_from_string(QString val) const;

    class TableInputDelegate: public QItemDelegate {
    public:
        TableInputDelegate(S7DataItem& data_item)
            : data_item_(data_item)
        {}

        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                              const QModelIndex & index) const
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            QRegularExpression regexp("^T#?([0-2]?\\dd)?(\\d{1,2}h)?([0-5]?\\dm)?([0-5]?\\ds)?(\\d{1,3}ms)?$");
            QRegularExpressionValidator* validator = new QRegularExpressionValidator(regexp, parent);
            lineEdit->setValidator(validator);
            return lineEdit;
        }

    private:
        S7DataItem& data_item_;
    };
};
#endif // PLC_DATA_HANDLER_H
