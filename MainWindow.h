#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void on_fileBtn_clicked();
    void downloadProgress(qint64, qint64);
    void uploadProgress(qint64, qint64);
    void error(QNetworkReply::NetworkError);
    void finished();


private:
    const char * CONFIG_DIR = "/.config/dummy-client";
    const char * CONFIG_PATH = "/.config/dummy-client/dummy.conf";

    Ui::MainWindow *ui;
    QNetworkReply * reply, *tmpreply;
    QNetworkAccessManager * mgr;
    QString fileName;
    QFile * downloadedFile;

};
#endif // MAINWINDOW_H
