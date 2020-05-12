#include "widget.h"
#include "ui_widget.h"

#include <QtDebug>
#include <QTime>
#include <QCryptographicHash>//计算hash值的类
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QList>

#include <cstdlib>
#include <ctime>



//3个List分别 与 初级、中级、高级 三种转换方式所对应
static QStringList LevelParam[3]={{"zh","en","de","zh"},
                                  {"zh","en","de","jp","spa","zh"},
                                  {"zh","en","de","jp","spa","it","pl","bul","zh"}};

// 以下为给初始成员变量赋初值，使之在构造时赋值
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_sourceType("auto")
    , m_transResultType("zh")
    , m_thesisMode(false)
    , m_transLevel(0)
{
    ui->setupUi(this);
    //因为m_naManager 为指针类型，所以需要给其分配空间
    //注：不需要在析构函数中delete指针，因为在new时继承了父类，会随父类析构时一起删掉
    m_naManager = new QNetworkAccessManager(this);
    // 当接收到数据完成信号后，更新结果 ，后面没有用到
//    connect(this,SIGNAL(recvDataFinishedSignal()),this,SLOT(updateResult()),Qt::AutoConnection);
    setWindowTitle("翻译小助手");
}

Widget::~Widget()
{
    delete ui;
}

QString Widget::getHashData(QString input){
    //返回哈希数据，第二个参数是采用何种算法
    QByteArray hashData = QCryptographicHash::hash(input.toLocal8Bit(),QCryptographicHash::Md5);
    //返回字节数组的十六进制编码，编码使用数字0-9和字母a-f
    return QString (hashData.toHex());
}

QString Widget::getSplicingData(QString inputStr){
    // 生成一个随机数
    QString strSalt = QString::number(qrand()% 1000);
    //qDebug() << apiWidget.getAppid();
    //qDebug() << apiWidget.getKey();
    //qDebug() << strSalt;
    //http://api.fanyi.baidu.com/api/trans/vip/translate?q=apple&from=en&to=zh&appid=2015063000000001&salt=1435660288&sign=f89f9594663708c1605f3d736d01d2d4
    // 返回拼接后的数据
    return QString("%1?q=%2&from=%3&to=%4&appid=%5&salt=%6&sign=%7")
            .arg(apiWidget.getUrl())
            .arg(inputStr)
            .arg(m_sourceType)
            .arg(m_transResultType)
            .arg(apiWidget.getAppid())
            .arg(strSalt)
            .arg(getHashData(apiWidget.getAppid()+inputStr+strSalt+apiWidget.getKey()));
}

QString Widget::getSyncData(const QString &strUrl)
{
    assert(!strUrl.isEmpty());
    //传入一个地址并转换为QUrl格式
    const QUrl url = QUrl::fromUserInput(strUrl);
    assert(url.isValid());

    QNetworkRequest request(url);
    QNetworkReply* reply = m_naManager->get(request); //m_naManager是QNetworkAccessManager对象

    //如下为 同步获取服务器响应 阻塞函数
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    // 获取http状态码
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
        qDebug() << "status code=" << statusCode.toInt();

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        qDebug() << "reason=" << reason.toString();

    QNetworkReply::NetworkError err = reply->error();
    if(err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    }else{
        // 获取返回内容，并全部读取出存到retStr中
        QByteArray replyData = reply->readAll();
        // qDebug() <<replyData;

        //下面为针对百度api返回的json格式进行解析
        //e.g  {"from":"en","to":"zh","trans_result":[{"src":"apple","dst":"苹果"}]}
        // 第一步 将QString类型转为QJsonObject类型
        QJsonObject obj =  QstringToJson(replyData.toStdString().c_str());
        // 第二步 获取到trans_result对应字段后将其按数组取出（实际上只有一个）所以为.at(0)并返回Json对象
        QJsonObject obj2 = obj.value("trans_result").toArray().at(0).toObject();
        //针对错误API进行的处理: 判断返回数据是否异常并将异常输出到输出框
        if(obj2.isEmpty()){
            m_transResult=replyData;
        }else{
        // 第三步 将获取到的Json对象 取出dst 对应字段经转换后存入 m_transResult
            m_transResult = obj2.value("dst").toString();
        }
    }

    reply->deleteLater();
    reply = nullptr;
    return m_transResult;
}

void Widget::on_pushButton_send_clicked()
{
    //从文本框获取输入数据
    QString inputStr=ui->textEdit_input->toPlainText().toUtf8();
    //BUG： 解决输入字符跨行后 只翻译第一行的问题
    inputStr.replace(QString("\n"),QString(""));
    qDebug() << inputStr ;
    //判断为翻译模式还是论文模式
    // 翻译模式 为直接翻译成指定的语言，翻译次数一次
    // 论文模式 按照指定的翻译模式翻译，翻译次数为多次
    if(!m_thesisMode){
        qDebug()<<"当前为翻译模式";
        m_transResult = getSyncData(getSplicingData(inputStr));
    }else{
        //去重原理
        //初级 中->英->德->中
        //中级 中->英->德->日->西班牙->中
        //高级 中->英->德->日->西班牙->意大利->波兰->保加利亚->中
        // 中  英 德 日 西班牙 意大利 波兰 保加利亚
        // zh en de jp spa   it    pl  bul
        qDebug()<<"当前为论文模式";
        // 获取指定的翻译模式中的参数
        QStringList currLevelParam = LevelParam[m_transLevel];
        // 遍历指定的翻译模式中的参数并将数据按照指定参数翻译
        // -1的原因是 最后一次肯定是其他语言翻译成中文，后续i + 1 为最后一个参数，所以最后一次翻译为i - 1次才能保证数据访问不会越界
        for(int i = 0;i < currLevelParam.size() -1; i++){
            // 给源语言类型 赋值
            m_sourceType = currLevelParam.at(i);
            // 给目的语言类型 赋值
            m_transResultType = currLevelParam.at(i+1);

            // 判断是否为首次转换，首次转换时数据从文本框输入，后续翻译时，数据为上一次生成数据结果
            if(i == 0){
                m_transResult = getSyncData(getSplicingData(inputStr));
            }else{
                m_transResult = getSyncData(getSplicingData(m_transResult));
            }
            qDebug() << m_sourceType <<"->"<<m_transResultType<<" : "<<m_transResult;
        }
    }
    // 更新结果显示
    updateResult();
}
void Widget::updateResult(){
    // 更新指定数据到文本输出框
    ui->textEdit_output->setPlainText(m_transResult);
    qDebug() << m_transResult;
}

void Widget::on_comboBox_currentTextChanged(const QString &arg1)
{
    m_transResultType = arg1;
}

void Widget::on_radioButton_clicked(bool checked)
{
    m_thesisMode = checked;
}

void Widget::on_comboBox_2_activated(int index)
{
    m_transLevel = index;
}

QJsonObject QstringToJson(QString jsonString)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data());
    if(jsonDocument.isNull())
    {
        qDebug()<< "===> please check the string "<< jsonString.toLocal8Bit().data();
    }
    QJsonObject jsonObject = jsonDocument.object();
    return jsonObject;
}

QString JsonToQstring(QJsonObject jsonObject)
{
    return QString(QJsonDocument(jsonObject).toJson());
}

void Widget::on_pushButton_apiSetting_clicked()
{
    qDebug()<<"切换到API设置页面";
    apiWidget.show();
}
