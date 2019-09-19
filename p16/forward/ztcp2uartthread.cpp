#include "ztcp2uartthread.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <sys/socket.h>
#include <zgblpara.h>
#include <QtSerialPort/QSerialPort>
ZTcp2UartForwardThread::ZTcp2UartForwardThread()
{
    this->m_bCleanup=false;
}
qint32 ZTcp2UartForwardThread::ZStartThread()
{
    this->start();
    return 0;
}
qint32 ZTcp2UartForwardThread::ZStopThread()
{
    return 0;
}
void ZTcp2UartForwardThread::run()
{
    qDebug()<<"Tcp2Uart thread start.";
    /*sudo chmod 777 /dev/ttyTHS2*/
    QSerialPort *uart=new QSerialPort;
    uart->setPortName("/dev/ttyTHS2");
    uart->setBaudRate(QSerialPort::Baud115200);
    uart->setDataBits(QSerialPort::Data8);
    uart->setParity(QSerialPort::NoParity);
    uart->setStopBits(QSerialPort::OneStop);
    uart->setFlowControl(QSerialPort::NoFlowControl);
    if(!uart->open(QIODevice::ReadWrite))
    {
        qDebug()<<"failed to open uart"<<uart->portName();
        delete uart;
        uart=NULL;
        this->m_bCleanup=true;
        //set global request to exit flag to notify other threads to terminate.
        gGblPara.m_bGblRst2Exit=true;
        qDebug()<<"Tcp2Uart thread done.";
        return;
    }

    this->m_bCleanup=false;
    while(!gGblPara.m_bGblRst2Exit)
    {
        QTcpServer *tcpServer=new QTcpServer;
        int on=1;
        int sockFd=tcpServer->socketDescriptor();
        setsockopt(sockFd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
        if(!tcpServer->listen(QHostAddress::Any,TCP_PORT_FORWARD))
        {
            qDebug()<<"Tcp2Uart failed to listen on port:"<<TCP_PORT_FORWARD;
            this->sleep(3);
            delete tcpServer;
            continue;
        }
        qDebug()<<"Tcp2Uart listen on tcp "<<TCP_PORT_FORWARD;
        //wait until get a new connection.
        while(!gGblPara.m_bGblRst2Exit)
        {
            //qDebug()<<"wait for tcp connection";
            if(tcpServer->waitForNewConnection(1000*2))
            {
                break;
            }
        }

        QTcpSocket *tcpSocket=tcpServer->nextPendingConnection();
        if(tcpSocket)
        {
            //close server once one client connected.
            //only allow one client connected at moment.
            tcpServer->close();

            //set connected flag.
            gGblPara.m_bTcp2UartConnected=true;

            while(!gGblPara.m_bGblRst2Exit)
            {
                //only ImgProc On then tx diffXY to uart.
                if(gGblPara.m_bJsonImgProOn)
                {
                    qint16 nDiffX=gGblPara.m_nDiffX;
                    qint16 nDiffY=gGblPara.m_nDiffY;
                    QByteArray baCmd;
                    baCmd.resize(12);
                    baCmd[0]=0x5A;//frame head.
                    baCmd[1]=0xA5;
                    baCmd[2]=0x0C;//frame length,12 bytes.
                    baCmd[3]=0x02;//frame type.
                    baCmd[4]=0x01;//end point.
                    //data section.
                    baCmd[5]=0x23;
                    baCmd[6]=0x04;
                    baCmd[7]=(qint8)((nDiffX>>8)&0xFF);//diff X.
                    baCmd[8]=(qint8)(nDiffX&0xFF);
                    baCmd[9]=(qint8)((nDiffY>>8)&0xFF);//diff Y.
                    baCmd[10]=(qint8)(nDiffY&0xFF);
                    baCmd[11]=gGblPara.ZDoCheckSum((uint8_t*)baCmd.data(),11);//checksum.
                    uart->write(baCmd);
                    if(!uart->waitForBytesWritten(SOCKET_TX_TIMEOUT))
                    {
                        qDebug()<<"Tcp2Uart failed to tx DiffXY "<<uart->errorString();
                        break;
                    }
                }

                //read data from tcp and write it to uart.
                if(tcpSocket->waitForReadyRead(50))//100ms.
                {
                    QByteArray baFromTcp=tcpSocket->readAll();
                    //qDebug()<<"from tcp:"<<baFromTcp;
                    uart->write(baFromTcp);
                    if(!uart->waitForBytesWritten(1000))
                    {
                        qDebug()<<"Tcp2Uart failed to forward tcp to uart "<<uart->errorString();
                        break;
                    }
                    gGblPara.m_nTcp2UartBytes+=baFromTcp.size();
                }
                //read data from uart and write data to tcp.
                if(uart->waitForReadyRead(50))//100ms.
                {
                    QByteArray baFromUart=uart->readAll();
                    //qDebug()<<"from uart:"<<baFromUart;
                    tcpSocket->write(baFromUart);
                    if(!tcpSocket->waitForBytesWritten(1000))
                    {
                        qDebug()<<"Tcp2Uart failed to forward uart to tcp:"<<tcpSocket->errorString();
                        break;
                    }
                    gGblPara.m_nUart2TcpBytes+=baFromUart.size();
                }

                //check tcp socket state.
                if(tcpSocket->state()!=QAbstractSocket::ConnectedState)
                {
                    qWarning()<<"Tcp2Uart socket broken.";
                    break;
                }
            }

            //set disconnect flag.
            gGblPara.m_bTcp2UartConnected=false;
        }
        delete tcpServer;
        tcpServer=NULL;
    }
    //do clean work here.
    uart->close();
    delete uart;
    uart=NULL;
    this->m_bCleanup=true;
    qDebug()<<"Tcp2Uart thread done.";

    //set global request exit flag to notify other threads.
    gGblPara.m_bGblRst2Exit=true;
    return;
}
bool ZTcp2UartForwardThread::ZIsCleanup()
{
    return this->m_bCleanup;
}
