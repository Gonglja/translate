#include "apiwidget.h"
#include "ui_apiwidget.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDebug>


APIWidget::APIWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::APIWidget)
    , m_url("http://api.fanyi.baidu.com/api/trans/vip/translate")
{
    ui->setupUi(this);
    setWindowTitle("API设置");
    int status=0;
    if(readConfig(status)){
        if(status == 1){
            ui->textEdit_appid->setPlainText(m_appid);
            ui->textEdit_key->setPlainText(m_key);
        }else if(status == 0){
            //默认配置不输出
            ;
        }else{
            //读取文件状态错误
            ;
        }
    }
}

APIWidget::~APIWidget()
{
    delete ui;
}

void APIWidget::on_pushButton_clicked()
{
    m_appid = ui->textEdit_appid->toPlainText().toUtf8();
    m_key = ui->textEdit_key->toPlainText().toUtf8() ;
    // 配置文件读取标志位
    m_configReadFlag = true;
    QJsonObject jsonObj;
    jsonObj.insert("appid",m_appid);
    jsonObj.insert("key",m_key);
    QJsonDocument jsonDoc(jsonObj);
    //将json对象转换成字符串
    QByteArray data=jsonDoc.toJson();
    QFile file("config.json");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
    this->hide();
}

bool APIWidget::readConfig(int &status){
    //首先判断是否存在配置文件
    QFileInfo fileInfo("config.json");
    if(fileInfo.isFile()){
        qDebug() << "文件存在，读取文件中配置";
        QFile loadFile("config.json");

        if(!loadFile.open(QIODevice::ReadOnly)){
             qDebug() << "could't open projects json";
             status = -1;
             return false;
         }

         QByteArray allData = loadFile.readAll();
         loadFile.close();

         QJsonParseError json_error;
         QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));

         if(json_error.error != QJsonParseError::NoError)
         {
             qDebug() << "json error!";
             status = -1;
             return false;
         }

         QJsonObject rootObj = jsonDoc.object();
         m_appid = rootObj.value("appid").toString();
         m_key = rootObj.value("key").toString();
         status = 1;
         return true;
    }else{
        qDebug() << "文件不存在，读取默认配置";
        m_appid = " ";//默认配置
        m_key = " ";
        status = 0;
        return true;
    }
}
QString APIWidget::getAppid(){
    return m_appid;
}
QString APIWidget::getKey(){
    return m_key;
}
QString APIWidget::getUrl(){
    return m_url;
}
