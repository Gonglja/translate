#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QPushButton>
#include "apiwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    //传入QString数据 获取MD5返回
    QString getHashData(QString);
    //传入输入字符串，拼接随机数后格式化数据并返回
    QString getSplicingData(QString);
    //使用get方法完成 数据的获取
    //具体流程 发送get消息，服务器响应后返回，得到返回消息（期间一直阻塞直到服务器响应）
    QString getSyncData(const QString &strUrl);

signals:

private slots:
    //在使用按键send后发生的一系列处理
    void on_pushButton_send_clicked();
    //当下拉框comboBox发生改变后，通过arg1传入改变后的字符串
    void on_comboBox_currentTextChanged(const QString &arg1);
    //当候选框发生改变后，通过checked传入变化的数值（true or false）
    void on_radioButton_clicked(bool checked);
    //更新输出框中的结果
    void updateResult();
    //通过下拉框 来选择使用的翻译的方式（初级/中级/高级）
    void on_comboBox_2_activated(int index);

    void on_pushButton_apiSetting_clicked();

private:
    Ui::Widget *ui;
    QString m_sourceType;     // 翻译的源类型
    QString m_transResultType;// 翻译结果的类型
    bool m_thesisMode;        // 是否开启论文模式
    QString m_transResult;    // 翻译结果
    QNetworkAccessManager* m_naManager;
    int m_transLevel;         // 翻译等级 0-初级  1-中级  2-高级
    QPushButton apiSettingButton;// 添加API设置按键
    APIWidget apiWidget;         // 获取API数据类

};
//Json格式 从QString转为QJsonObject类型
QJsonObject QstringToJson(QString jsonString);
//Json格式 从QJsonObject转为QString类型，后面没有用到
QString JsonToQstring(QJsonObject jsonObject);
#endif // WIDGET_H
