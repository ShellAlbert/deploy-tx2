#include "zimgdispui.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>
#include <zgblpara.h>
ZImgDispUI::ZImgDispUI(QString title,QWidget *parent) : QWidget(parent)
{
    this->m_title=title;
}
ZImgDispUI::~ZImgDispUI()
{

}
qint32 ZImgDispUI::ZDoInit()
{
    this->m_counter=0;
    return 0;
}
qint32 ZImgDispUI::ZDoClean()
{
    return 0;
}
void ZImgDispUI::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if(this->m_img.isNull())
    {
        return;
    }

    //1.draw image on widget.
    QPainter painter(this);
    painter.drawImage(QRectF(0,0,this->width(),this->height()),this->m_img);

    //2.draw informations on widget.
    QFont font=painter.font();
    font.setPixelSize(20);
    painter.setFont(font);
    painter.setPen(Qt::green);
    QFontMetrics fontMetrics=painter.fontMetrics();
    QRect rectFont(0,0,fontMetrics.width(this->m_title),fontMetrics.height());
    painter.drawText(rectFont,this->m_title);

    QString counter=QString("%1").arg(this->m_counter);
    QRect rectCounter(this->width()-fontMetrics.width(counter),0,fontMetrics.width(counter),fontMetrics.height());
    painter.drawText(rectCounter,counter);
}
void ZImgDispUI::ZSlotFlushImg(const QImage &img)
{
    this->m_img=img;
    this->m_counter++;
    this->update();
}
