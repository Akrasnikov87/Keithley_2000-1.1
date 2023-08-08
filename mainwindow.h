#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QtSerialPort>
#include "qcustomplot.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
    void delay(const int msleep) {
        QTime dieTime = QTime::currentTime().addMSecs(msleep);
        while (QTime::currentTime() < dieTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents,1);
        }
    }
	void Check() {
		portPushButton->setEnabled(!lineEdit->text().isEmpty());
	}
	void settings() {
		dialog.show();
	}
    void Save();
    void Connect();
	void Start();
	void Stop();
	void Filtr();
    void Write(QString);
    void Read();

private:
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *menuFile = new QMenu("File");
	QPushButton *startPushButton = new QPushButton("&START");
	QPushButton *stopPushButton = new QPushButton("&STOP");
	QPushButton *filterPushButton = new QPushButton("&FILTER");
    QCheckBox *graphCheckBox = new QCheckBox("&Graph");
    QCustomPlot *customPlot = new QCustomPlot(this);
	QCPCurve *cpcurve;

	QDialog dialog;
    QComboBox *comboBox = new QComboBox(this);
    QComboBox *comboBox2 = new QComboBox(this);
    QComboBox *comboBox4 = new QComboBox(this);
    QLabel *label = new QLabel("CR");
    QComboBox *comboBox5 = new QComboBox(this);
	QSpinBox *spinBox = new QSpinBox();
    QComboBox *comboBox6 = new QComboBox(this);
    QSpinBox *digitsSpinBox = new QSpinBox(this);
    QLineEdit *lineEdit = new QLineEdit(this);
    QPushButton *portPushButton = new QPushButton("&CONNECT");

    QFile *file = new QFile(this);
    QSerialPort *port = new QSerialPort(this);
	QTime time;
	QTextStream stream;

    bool bolen;
    bool boolDataReadyMultimetr;
    QByteArray byteArrayMultimetr;
};
#endif // MAINWINDOW_H
