#include "codeeditorbackend.h"

CodeEditorBackend::CodeEditorBackend()
{
    controller = Controller::getInstance();
    th = new QThread();
    Am= new BeginInterpreter();
    Am->moveToThread(th);
    connect(this, SIGNAL(AntlrStart()),Am, SLOT(begin()));
    th->start(QThread::LowestPriority);
    connect(controller->beckhoff, SIGNAL(CurrentLineChangedB()),this, SLOT(changedRunningLine()));
    //m_text = "func()\r\nend";
    emit textChanged(m_text);
}

QString CodeEditorBackend::text() const
{
    return m_text;
}

QUrl CodeEditorBackend::fileUrl() const
{
    return m_fileUrl;
}

QString CodeEditorBackend::fileName() const
{
    return m_fileName;
}

void CodeEditorBackend::setText(QString text)
{
    if (m_text == text)
        return;

    m_text = text;
    emit textChanged(text);
}

void CodeEditorBackend::setFileUrl(QUrl fileUrl)
{
    if (m_fileUrl == fileUrl)
        return;

    m_fileUrl = fileUrl;
    emit fileUrlChanged(fileUrl);
    setFileName(fileUrl.fileName());
}

void CodeEditorBackend::setFileName(QString fileName)
{
    if (m_fileName == fileName)
        return;

    m_fileName = fileName;
    emit fileNameChanged(fileName);
}

void CodeEditorBackend::play(QString runFromLine)
{
    controller->beckhoff->runFromLineNumber=runFromLine.toInt();
    controller->beckhoff->runAll=true;
    controller->beckhoff->currentLine=0;
    controller->beckhoff->doNextLine=true;
    controller->beckhoff->stopAnltrRun=false;
    Am->load(m_fileUrl.toLocalFile().toUtf8().constData());
    //->load(m_fileUrl.toUtf8().constData());

    //Am->begin();
    Q_EMIT AntlrStart();
}
void CodeEditorBackend::pause()
{
    controller->beckhoff->doNextLine=!controller->beckhoff->doNextLine;
}

void CodeEditorBackend::stop()
{
    controller->beckhoff->doNextLine=true;
    controller->beckhoff->stopAnltrRun=true;
}
void CodeEditorBackend::changedRunningLine()
{
    int currentL= controller->beckhoff->currentLine;
    int index1=0, index2=0;
    for(int i=0; i<currentL; i++){
        index1 = m_text.indexOf("\n", index1+1);
    }
    index2 = m_text.indexOf("\n", index1+1);

    Q_EMIT lineSelect(index1, index2);
}

QString CodeEditorBackend::addCommandToCurrentLine(int cmd, QString targetP1, QString targetP2, QString targetP3, QString frameType, QString frameTargetPoint, QString moveParam, QString theta, QString exp1, QString exp2, QString id)
{
    string str;
    string moveParam_ = moveParam.toUtf8().constData();
    string theta_ = theta.toUtf8().constData();
    string exp1_ = exp1.toUtf8().constData();
    string exp2_ = exp2.toUtf8().constData();
    string id_ = id.toUtf8().constData();
    string returnVal = id_+"_ret";

    switch (cmd) {
    case LanguageCMD::IF:
        str = "IF "+exp1_+" THEN\r\n\r\nENDIF";
        break;
    case LanguageCMD::IFELSE:
        str = "IF "+exp1_+" THEN\r\n\r\nELSE\r\n\r\nENDIF";
        break;
    case LanguageCMD::FOR:
        str = "FOR "+id_+" = "+exp1_+" TO "+exp2_+" \r\n\r\nENDFOR";
        break;
    case LanguageCMD::WHILE:
        str = "WHILE "+exp1_+"\r\n\r\nENDWHILE";
        break;
    case LanguageCMD::SETFRAME:
        str = "SETFRAME "+(string)(frameType.toUtf8().constData())+" "+(string)(frameTargetPoint.toUtf8().constData());
        break;
    case LanguageCMD::INTERRUPT:
        str = "int "+id_+"_handler()\n\nEND";
        str += "int "+returnVal+"\n"+"GLOBAL INTERRUPT DECL "+id_+" [priority] WHEN "+exp1_+" DO "+returnVal+"="+id_+"_handler()";
        break;
    case LanguageCMD::Function:
        str = exp1_+" "+id_+"("+exp2_+")\n\nEND";
        break;
    case LanguageCMD::PTP:
        str = "PTP "+(string)(targetP1.toUtf8().constData())+" "+moveParam_;
        break;
    case LanguageCMD::LIN:
        str = "LIN "+(string)(targetP1.toUtf8().constData())+" "+moveParam_;
        break;
    case LanguageCMD::CIRC:
        str = "CIR "+(string)(targetP1.toUtf8().constData())+" "+(string)(targetP2.toUtf8().constData())+" "+theta_+" "+moveParam_;
        break;
    }
    return QString::fromStdString("\r\n"+str);
}

void CodeEditorBackend::setCursorPos(int pos)
{
    cursorPosition = pos;
}

bool CodeEditorBackend::save()
{
    QFile file(m_fileUrl.toLocalFile());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    file.write(m_text.toUtf8());
    file.close();
    return true;
}

bool CodeEditorBackend::load()
{
    QFile file(m_fileUrl.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QByteArray content = file.readAll();
    setText(QString::fromUtf8(content.constData(), content.length()));

    file.close();
    return true;
}
