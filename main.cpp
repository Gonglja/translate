#include "widget.h"

#include <QApplication>
#include <QTextCodec>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //���´���Ϊ����windows���������Ĳ��ܻ����ȷ��ʽ
    a.setFont(QFont("Microsoft Yahei", 9));
   #if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
   #if _MSC_VER
       QTextCodec *codec = QTextCodec::codecForName("GBK");
   #else
       QTextCodec *codec = QTextCodec::codecForName("UTF-8");
   #endif
       QTextCodec::setCodecForLocale(codec);
       QTextCodec::setCodecForCStrings(codec);
       QTextCodec::setCodecForTr(codec);
   #else
       QTextCodec *codec = QTextCodec::codecForName("UTF-8");
       QTextCodec::setCodecForLocale(codec);
   #endif
    //����
    Widget w;
    w.show();
    return a.exec();
}