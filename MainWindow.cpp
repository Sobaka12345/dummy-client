#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QDir>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if(!QDir(QDir::homePath() + CONFIG_DIR).exists() ||
       !QFile().exists(QDir::homePath() + CONFIG_PATH))
    {
        QDir dir(QDir::homePath() + "/.config");
        dir.mkdir("dummy-client");

        QFile conf(QDir::homePath() + CONFIG_PATH);
        conf.open(QIODevice::WriteOnly);
        conf.write(QString("http://192.168.0.101/server_script.php").toUtf8() + "\n");
        conf.write("\n");
        conf.write("true\n");
        conf.write("false");
        conf.close();
    }

    QFile conf(QDir::homePath() + CONFIG_PATH);
    conf.open(QIODevice::ReadOnly);
    ui->scriptPath->setText(conf.readLine());
    fileName = conf.readLine().trimmed();
    ui->fileLabel->setText(fileName.trimmed().isEmpty() ? "Choose file for upload" : fileName);
    ui->checkBox->setChecked(conf.readLine().trimmed() == "true" ? true : false);
    ui->checkBox_2->setChecked(conf.readLine() == "true" ? true : false);
    conf.close();

    mgr = new QNetworkAccessManager(this);
}

MainWindow::~MainWindow()
{
    QFile conf(QDir::homePath() + CONFIG_PATH);
    conf.open(QIODevice::WriteOnly);
    conf.write(ui->scriptPath->text().toUtf8().trimmed() + "\n");
    conf.write(fileName.toUtf8().trimmed() + "\n");
    conf.write(ui->checkBox->isChecked() ? "true\n" : "false\n");
    conf.write(ui->checkBox_2->isChecked() ? "true" : "false");
    conf.close();

    delete ui;
    delete mgr;
}

void MainWindow::on_startButton_clicked()
{
    if(fileName.trimmed().isEmpty())
        return;
    ui->startButton->setEnabled(false);

    QNetworkRequest request(QUrl(ui->scriptPath->text().trimmed()));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setPriority(QNetworkRequest::Priority::HighPriority);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QString bound("--------4815162342");
    QByteArray data(QString("--" + bound + "\r\n").toUtf8());
    data.append("Content-Disposition: form-data; name=\"action\"\r\n\r\n");
    data.append("server_script.php\r\n");
    data.append("--" + bound + "\r\n");

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    data.append("Content-Disposition: form-data; name=\"uploaded\"; filename=\"" + file.fileName().section("/",-1,-1) + "\"\r\n");
    data.append("Content-Type: application/octet-stream\r\n\r\n");
    data.append(file.readAll());
    data.append("\r\n");
    data.append("--" + bound + "--\r\n");
    request.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, QString::number(data.length()));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "multipart/form-data; boundary=" + bound);
    file.close();

    downloadedFile = new QFile("tmp");
    downloadedFile->open(QIODevice::WriteOnly);
    reply = mgr->post(request, data);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(error(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
                this, SLOT(downloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)),
                this, SLOT(uploadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()),
                this, SLOT(finished()));
}

void MainWindow::uploadProgress(qint64 part, qint64 total)
{
    if(!total)
        return;
    ui->progressBar->setMaximum(total * 2);
    ui->progressBar->setValue(part);
}

void MainWindow::downloadProgress(qint64 part, qint64 total)
{
    if(!total)
        return;

    ui->progressBar->setValue((float(part) / reply->header(QNetworkRequest::ContentLengthHeader).toInt())
                              * (ui->progressBar->maximum() / 2) + ui->progressBar->maximum() / 2);

    downloadedFile->write(reply->readAll());
}

void MainWindow::error(QNetworkReply::NetworkError error)
{
    qDebug() << error;

    ui->startButton->setEnabled(true);
    ui->progressBar->setValue(0);

    QMessageBox msg;
    msg.setWindowTitle("ERROR");
    msg.setFixedSize(300,150);
    msg.setText(reply->errorString());
    msg.exec();
}

void MainWindow::finished()
{
    QString fname = reply->header(QNetworkRequest::ContentDispositionHeader).toString();
    fname.remove(0, fname.indexOf("filename=", 0, Qt::CaseInsensitive) + 9);
    if(fname.isEmpty())
    {
        if(ui->checkBox_2->isChecked())
            fname = "server_response.html";
        else {
            downloadedFile->close();
            downloadedFile->remove();
            delete downloadedFile;

            ui->startButton->setEnabled(true);

            reply->deleteLater();
            return;
        }
    }

    if(QFile(fname).exists())
    {
        if(ui->checkBox->isChecked())
            QFile(fname).remove();
        else
        {
            int i = 1;
            while(QFile(fname + "(" + QString::number(i) + ")").exists()) i++;
            fname = fname + "(" + QString::number(i) + ")";
        }
    }

    downloadedFile->rename(fname);
    downloadedFile->close();
    delete downloadedFile;

    ui->startButton->setEnabled(true);

    reply->deleteLater();
}

void MainWindow::on_fileBtn_clicked()
{
    QString temp = QFileDialog::getOpenFileName(this, "Choose file", "./");
    fileName = temp.isEmpty() ? fileName : temp;
    ui->fileLabel->setText(fileName.trimmed().isEmpty() ? "Choose file for upload" : fileName);
}
