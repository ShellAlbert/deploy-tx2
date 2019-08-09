#ifndef ZMAINUI_H
#define ZMAINUI_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <ui/zimgdispui.h>
class ZMainUI : public QWidget
{
    Q_OBJECT

public:
    ZMainUI(QWidget *parent = 0);
    ~ZMainUI();
public:
    qint32 ZDoInit();
    qint32 ZDoClean();

    ZImgDispUI* ZGetDispUI(qint32 index);
private:
    QLabel *m_llTop;//top label.
    ZImgDispUI *m_UILft;//the left main camera.
    ZImgDispUI *m_UIRht;//the right aux camera.
    QHBoxLayout *m_hLayout;
    QVBoxLayout *m_vLayout;
};

#endif // ZMAINUI_H
