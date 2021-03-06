#ifndef ZIMGDISPUI_H
#define ZIMGDISPUI_H

#include <QWidget>
class ZImgDispUI : public QWidget
{
    Q_OBJECT
public:
    explicit ZImgDispUI(QString title,bool bMainImg=false,QWidget *parent = nullptr);
    ~ZImgDispUI();
public:
    qint32 ZDoInit();
    qint32 ZDoClean();
protected:
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;
public slots:
    void ZSlotFlushImg(const QImage &img);
private:
    QImage m_img;
    QString m_title;
    qint64 m_counter;
private:
    bool m_bMainImg;
};

#endif // ZIMGDISPUI_H
