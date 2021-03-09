#include "skippymainwindow.h"
#include "ui_skippymainwindow.h"

SkippyMainWindow::SkippyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SkippyMainWindow)
{
    auto pageView = new PageView;
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setFocusPolicy(Qt::ClickFocus);

    //Setup Appirance
    this->setStyleSheet("background-color: rgb(36, 42, 53); color: rgb(139, 148, 190);");
    this->setWindowOpacity(0.98);
    ui->outputTextBrowser->setFontPointSize(10);
    ui->outputTextBrowser->setStyleSheet("background-color: rgb(8, 10, 13); color: rgb(178, 255, 176);");

    //Setup Treeview Appirance

    setStarted(false);
    CheckForSkippyDir();

    //Setup the filesystem for Http UI
    m_dirModel = new QDirModel(this);
    m_dirModel->setReadOnly(false);
    m_dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    m_dirModel->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);
    ui->treeView->setModel(m_dirModel);

    //Setup automatic drive selection
    QModelIndex index = m_dirModel->index(m_startPath);
    ui->treeView->expand(index);
    ui->treeView->scrollTo(index);
    ui->treeView->setCurrentIndex(index);
    ui->treeView->resizeColumnToContents(0);

    //Set up IP Address Combo Box
    QStringList lst = m_server.getAddresses();
    ui->addressComboBox->insertItems(0,lst);
    ui->addressComboBox->insertItem(0,"Any");
    ui->addressComboBox->setCurrentIndex(0);
}

SkippyMainWindow::~SkippyMainWindow()
{
    delete ui;
}

void SkippyMainWindow::skippyOutput(QString value)
{
    //QTextCursor cursor = ui->outputTextBrowser->textCursor();
    //ui->outputTextBrowser->selectAll();
    //ui->outputTextBrowser->setFontPointSize(10);
    //ui->outputTextBrowser->setTextCursor( cursor );
    ui->outputTextBrowser->append(value);
}

void SkippyMainWindow::closeEvent(QCloseEvent *event)
{
    int ret = QMessageBox::warning(this, tr("Confirm close"),
                                   tr("Are you sure you want to shutdown Skippy?\n"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::No)
    {
        event->ignore();
        return;
    }
    event->accept();
    deleteLater();
}

void SkippyMainWindow::on_makeButton_clicked()
{
    //make dir
    QModelIndex index = ui->treeView->currentIndex();
    //Defensive programming
    if(!index.isValid()) return;

    QString name = QInputDialog::getText(this,"Name","Enter a name");

    if(name.isEmpty()) return;

    m_dirModel->mkdir(index,name);
}

void SkippyMainWindow::on_deleteButton_clicked()
{
    QModelIndex index = ui->treeView->currentIndex();

    if(!index.isValid()) return;

    if(m_dirModel->fileInfo(index).isDir())
    {
        //dir
        EmptyDirCheck();
        m_dirModel->rmdir(index);
    }
    else
    {
        //file
        m_dirModel->remove(index);
    }
}

void SkippyMainWindow::on_setButton_clicked()
{
   CheckDirPremissions();
   QModelIndex index = ui->treeView->currentIndex();
   m_path = m_dirModel->filePath(index);
   ui->setLabel->setText(m_dirModel->filePath(index));
}

void SkippyMainWindow::on_listenButton_clicked()
{
    QHostAddress adr = QHostAddress::Any;
    CheckAddressAndPort();
    if(m_server.listen(adr,ui->portSpinBox->value()))
    {
        skippyOutput("Server started");
        m_server.setRate(ui->rateSpinBox->value());
        PathSetup();
    }
    else
    {
        qCritical() << m_server.errorString();
        setStarted(false);
    }
    setStarted(true);
}


void SkippyMainWindow::on_stopButton_clicked()
{
    setStarted(false);
    skippyOutput("Server stopped.");
    m_server.close();
}

void SkippyMainWindow::setStarted(bool started)
{
    if(started)
    {
        ui->listenButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->pageButton->setEnabled(true);
    }
    else
    {
        ui->listenButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        ui->pageButton->setEnabled(false);
    }
}

void SkippyMainWindow::on_pageButton_clicked()
{
    pageView.show();
}

void SkippyMainWindow::CheckAddressAndPort()
{
    QString pageAddress;
    QString port = QString::number(ui->portSpinBox->value());
    QHostAddress adr = QHostAddress::Any;
    if(ui->addressComboBox->currentIndex() > 0)
    {
        adr.setAddress(ui->addressComboBox->currentText());
        pageAddress = ui->addressComboBox->currentText();
    }
    else
    {
        pageAddress = "localhost";
    }
    skippyOutput("IP set to:" + ui->addressComboBox->currentText());
    skippyOutput("Port set to: " + port);
    pageView.loadPage("http://"+ pageAddress + ":" + port);
    skippyOutput("Page address set to: http://"+ pageAddress + ":" + port);
}

void SkippyMainWindow::CheckForSkippyDir()
{
    //Check if the default Skippy directory exists and setup
    QDir SkippyDir(QDir::homePath()+"/Skippy");
    if(!QDir(SkippyDir).exists())
    {
        skippyOutput("Default Skippy directory not found.");
        skippyOutput("Creating Skippy directory");
        QDir().mkdir(QDir::homePath()+"/Skippy");
        skippyOutput("Creating Skippy/www directory");
        QDir().mkdir(QDir::homePath()+"/Skippy/www");
        skippyOutput("Creating Skippy/scanner directory");
        QDir().mkdir(QDir::homePath()+"/Skippy/scanner");
    }
    m_startPath = QDir::homePath()+"/Skippy/www";
    m_scannerPath = QDir::homePath()+"/Skippy/scanner";
}

void SkippyMainWindow::EmptyDirCheck()
{
    QModelIndex index = ui->treeView->currentIndex();
    auto dirPath = m_dirModel->filePath(index);
    if(QDir(dirPath ).entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() != 0)
    {
        QMessageBox::information(this,"Directory is not empty","Not Empty.");
    }
}

void SkippyMainWindow::CheckDirPremissions()
{
    QModelIndex index = ui->treeView->currentIndex();
    auto dirPath = m_dirModel->filePath(index);
    auto dirToCheck = QFile::permissions(dirPath);
    if(dirToCheck & QFile::WriteOther || dirToCheck & QFile::WriteGroup || dirToCheck & QFile::ExeOther || dirToCheck & QFile::ExeGroup)
    {
        int replyW = QMessageBox::warning(this, tr("Warning!"),
                                         tr("Warning this directory has writing privileges.\n Do you want to proceed?"),
                                         QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
        if(replyW == QMessageBox::Yes)
        {
            int replyC = QMessageBox::question(this,
                                               "Change privileges?",
                                               "Do you want to revoke this directories writing privileges?\n Yes is recommended",
                                                QMessageBox::Yes | QMessageBox::No);
            if(replyC == QMessageBox::Yes)
            {
                QFile(dirPath).setPermissions(QFileDevice::ReadOther | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
                SecurePath();

            }
            else
            {;
                return;
            }
        }
        else if(replyW == QMessageBox::No)
        {
            UnsecurePath();
            return;
        }
    }
    else
    {
        SecurePath();
    }
}

void SkippyMainWindow::SecurePath()
{
    ui->outputTextBrowser->setStyleSheet("background-color: rgb(8, 10, 13); color: rgb(178, 255, 176);");
    skippyOutput("Site's path set to an secure directory.");
}

void SkippyMainWindow::UnsecurePath()
{
    ui->outputTextBrowser->setStyleSheet("background-color: rgb(8, 10, 13); color: rgb(252, 126, 129);");
    skippyOutput("Warning: Site's path set to an unsecure directory.");
}

void SkippyMainWindow::FilePremissionCheck()
{
    QModelIndex index = ui->treeView->currentIndex();
    auto dirPath = m_dirModel->filePath(index);
    QDirIterator it(dirPath, QStringList() << "*.html", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        //TODO Check
        qDebug() << this << "html file found";
    }
}

void SkippyMainWindow::PathSetup()
{
    if(m_path != "")
    {
        if(m_path != m_startPath)
        {
            skippyOutput("Sites's path set to: " + m_path);
        }
    }
    else
    {
        skippyOutput("Sites's path set to default path: " + m_startPath);
        ui->setLabel->setText(m_path);
    }
    m_server.setRoot(m_path);
    ui->setLabel->setText(m_path);
}


void SkippyMainWindow::on_scanButton_clicked()
{
    //scanner = new Scanner;
    scanner.pyCreator(m_scannerPath);
}
