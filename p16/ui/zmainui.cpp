#include "zmainui.h"
#include <QDebug>
ZMainUI::ZMainUI(QWidget *parent):QWidget(parent)
{
    this->m_UILft=NULL;
    this->m_UIRht=NULL;
    this->m_hLayout=NULL;
    this->m_vLayout=NULL;
}

ZMainUI::~ZMainUI()
{
    this->ZDoClean();
}
ZImgDispUI* ZMainUI::ZGetDispUI(qint32 index)
{
    switch(index)
    {
    case 0:
        return this->m_UILft;
        break;
    case 1:
        return this->m_UIRht;
        break;
    default:
        break;
    }
    return NULL;
}
qint32 ZMainUI::ZDoInit()
{
    try{
        this->m_llTop=new QLabel;
        this->m_UILft=new ZImgDispUI("MAIN");
        this->m_UIRht=new ZImgDispUI("AUX");
        this->m_hLayout=new QHBoxLayout;
        this->m_vLayout=new QVBoxLayout;
    }catch(...)
    {
        qDebug()<<"<error>:new failed,low memory.";
        return -1;
    }

    if(this->m_UILft->ZDoInit()<0)
    {
        return -1;
    }
    if(this->m_UIRht->ZDoInit()<0)
    {
        return -1;
    }
    this->m_hLayout->addWidget(this->m_UILft);
    this->m_hLayout->addWidget(this->m_UIRht);
    this->m_vLayout->addLayout(this->m_hLayout);
    this->m_vLayout->addWidget(this->m_llTop);
    this->setLayout(this->m_vLayout);

    return 0;
}
qint32 ZMainUI::ZDoClean()
{
    if(this->m_llTop)
    {
        delete this->m_llTop;
    }
    if(this->m_UILft)
    {
        delete this->m_UILft;
    }
    if(this->m_UIRht)
    {
        delete this->m_UIRht;
    }
    if(this->m_hLayout)
    {
        delete this->m_hLayout;
    }
    if(this->m_vLayout)
    {
        delete this->m_vLayout;
    }
    return 0;
}
