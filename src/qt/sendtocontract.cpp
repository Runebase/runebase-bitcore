#include "sendtocontract.h"
#include "ui_sendtocontract.h"
#include "platformstyle.h"
#include "walletmodel.h"
#include "guiconstants.h"
#include "rpcconsole.h"
#include "execrpccommand.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "validation.h"
#include "utilmoneystr.h"

namespace SendToContract_NS
{
// Contract data names
static const QString PRC_COMMAND = "sendtocontract";
static const QString PARAM_ADDRESS = "address";
static const QString PARAM_DATAHEX = "datahex";
static const QString PARAM_AMOUNT = "amount";
static const QString PARAM_GASLIMIT = "gaslimit";
static const QString PARAM_GASPRICE = "gasprice";
static const QString PARAM_SENDER = "sender";

static const CAmount SINGLE_STEP = 0.00000001*COIN;
}
using namespace SendToContract_NS;

SendToContract::SendToContract(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendToContract)
{
    // Setup ui components
    Q_UNUSED(platformStyle);
    ui->setupUi(this);
    ui->groupBoxOptional->setStyleSheet(STYLE_GROUPBOX);
    on_updateGasValues();

    ui->labelContractAddress->setToolTip(tr("The contract address that will receive the funds and data."));
    ui->labelDataHex->setToolTip(tr("The data to send."));
    ui->labelAmount->setToolTip(tr("The amount in QTUM to send. Default = 0."));
    ui->labelSenderAddress->setToolTip(tr("The quantum address that will be used as sender."));
   
    // Set defaults
    ui->lineEditGasPrice->setValue(DEFAULT_GAS_PRICE);
    ui->lineEditGasPrice->setSingleStep(SINGLE_STEP);
    ui->lineEditGasLimit->setValue(DEFAULT_GAS_LIMIT_OP_SEND);

    // Create new PRC command line interface
    QStringList lstMandatory;
    lstMandatory.append(PARAM_ADDRESS);
    lstMandatory.append(PARAM_DATAHEX);
    QStringList lstOptional;
    lstOptional.append(PARAM_AMOUNT);
    lstOptional.append(PARAM_GASLIMIT);
    lstOptional.append(PARAM_GASPRICE);
    lstOptional.append(PARAM_SENDER);
    QMap<QString, QString> lstTranslations;
    lstTranslations[PARAM_ADDRESS] = ui->labelContractAddress->text();
    lstTranslations[PARAM_DATAHEX] = ui->labelDataHex->text();
    lstTranslations[PARAM_AMOUNT] = ui->labelAmount->text();
    lstTranslations[PARAM_GASLIMIT] = ui->labelGasLimit->text();
    lstTranslations[PARAM_GASPRICE] = ui->labelGasPrice->text();
    lstTranslations[PARAM_SENDER] = ui->labelSenderAddress->text();
    m_execRPCCommand = new ExecRPCCommand(PRC_COMMAND, lstMandatory, lstOptional, lstTranslations, this);

    // Connect signals with slots
    connect(ui->pushButtonClearAll, SIGNAL(clicked()), SLOT(on_clearAll_clicked()));
    connect(ui->pushButtonSendToContract, SIGNAL(clicked()), SLOT(on_sendToContract_clicked()));
}

SendToContract::~SendToContract()
{
    delete ui;
}

void SendToContract::setModel(WalletModel *_model)
{
    model = _model;
}

void SendToContract::on_clearAll_clicked()
{
    ui->lineEditContractAddress->clear();
    ui->lineEditDataHex->clear();
    ui->lineEditAmount->clear();
    ui->lineEditGasLimit->setValue(DEFAULT_GAS_LIMIT_OP_SEND);
    ui->lineEditGasPrice->setValue(DEFAULT_GAS_PRICE);
    ui->lineEditSenderAddress->clear();
}

void SendToContract::on_sendToContract_clicked()
{
    // Initialize variables
    QMap<QString, QString> lstParams;
    QVariant result;
    QString errorMessage;
    QString resultJson;
    int unit = model->getOptionsModel()->getDisplayUnit();

    // Append params to the list
    ExecRPCCommand::appendParam(lstParams, PARAM_ADDRESS, ui->lineEditContractAddress->text());
    ExecRPCCommand::appendParam(lstParams, PARAM_DATAHEX, ui->lineEditDataHex->text());
    ExecRPCCommand::appendParam(lstParams, PARAM_AMOUNT, BitcoinUnits::format(unit, ui->lineEditAmount->value()));
    ExecRPCCommand::appendParam(lstParams, PARAM_GASLIMIT, QString::number(ui->lineEditGasLimit->value()));
    ExecRPCCommand::appendParam(lstParams, PARAM_GASPRICE, BitcoinUnits::format(unit, ui->lineEditGasPrice->value()));
    ExecRPCCommand::appendParam(lstParams, PARAM_SENDER, ui->lineEditSenderAddress->text());

    // Execute RPC command line
    if(m_execRPCCommand->exec(lstParams, result, resultJson, errorMessage))
    {
        QString message = tr("Send to the contract is performed successfully.\n\n") + resultJson;
        QMessageBox::information(this, tr("Send to contract"), message);
    }
    else
    {
        QMessageBox::warning(this, tr("Send to contract"), errorMessage);
    }
}

void SendToContract::on_updateGasValues()
{
    QtumDGP qtumDGP(globalState.get(), fGettingValuesDGP);
    uint64_t blockGasLimit = qtumDGP.getBlockGasLimit(chainActive.Height()+1);
    uint64_t minGasPrice = CAmount(qtumDGP.getMinGasPrice(chainActive.Height()+1));

    ui->labelGasLimit->setToolTip(tr("Gas limit: Default = %1, Max = %2.").arg(DEFAULT_GAS_LIMIT_OP_SEND).arg(blockGasLimit));
    ui->labelGasPrice->setToolTip(tr("Gas price: QTUM price per gas unit. Default = %1, Min = %2.").arg(QString::fromStdString(FormatMoney(DEFAULT_GAS_PRICE))).arg(QString::fromStdString(FormatMoney(minGasPrice))));
    ui->lineEditGasLimit->setMaximum(blockGasLimit);
}
