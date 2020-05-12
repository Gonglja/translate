#ifndef APIWIDGET_H
#define APIWIDGET_H

#include <QWidget>

namespace Ui {
class APIWidget;
}

class APIWidget : public QWidget
{
    Q_OBJECT

public:
    explicit APIWidget(QWidget *parent = nullptr);
    ~APIWidget();
    bool readConfig(int &status);
    QString getAppid();
    QString getKey();
    QString getUrl();

private slots:
    void on_pushButton_clicked();

private:
    Ui::APIWidget *ui;
    QString m_appid;//appid 百度翻译中心注册得到
    QString m_key;  //key   百度翻译中心注册得到
    QString m_url;  //url
    bool m_configReadFlag; // 默认为false 使用默认配置， 当检测到用户设置api后，读取存入到config.json中的配置
};

#endif // APIWIDGET_H
