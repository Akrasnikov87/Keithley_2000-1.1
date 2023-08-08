#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	setWindowTitle("Keithley 2000 1.1");
    setWindowIcon(QIcon(":/k_2000.jpg"));

	menuBar->addMenu(menuFile);

    QAction *action = new QAction("Settings",menuFile);
	menuFile->addAction(action);
    connect (action,SIGNAL(triggered()),this,SLOT(settings()));

    QAction *action2 = new QAction("Save",menuFile);
	menuFile->addAction(action2);
    connect (action2,SIGNAL(triggered()),this,SLOT(Save()));

	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->addWidget(menuBar);
    menuBar->setMaximumHeight(25);
	QHBoxLayout *hlayout2 = new QHBoxLayout;
	hlayout2->addWidget(startPushButton,1);
	hlayout2->addWidget(stopPushButton,1);
	hlayout2->addWidget(filterPushButton,1);
    hlayout2->addWidget(graphCheckBox,1);
    hlayout2->addStretch(10);
	QHBoxLayout *hlayout3 = new QHBoxLayout;
	hlayout3->addWidget(customPlot);
	QVBoxLayout *vbxLayout = new QVBoxLayout;
	vbxLayout->addLayout(hlayout);
	vbxLayout->addLayout(hlayout2);
	vbxLayout->addLayout(hlayout3);
	QWidget *widget = new QWidget(this);
	widget->setLayout(vbxLayout);
    setCentralWidget(widget);

	QFormLayout *parametersLayout = new QFormLayout;
	parametersLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    parametersLayout->addRow("Port:",comboBox);
    parametersLayout->addRow("Baud:",comboBox2);
    parametersLayout->addRow("Flow:",comboBox4);
    parametersLayout->addRow("Tx Term:",label);
    parametersLayout->addRow("Type:",comboBox5);
    parametersLayout->addRow("Speed:",comboBox6);
    parametersLayout->addRow("Digits:",digitsSpinBox);
    parametersLayout->addRow("Filter:",spinBox);
    parametersLayout->addRow("File name:",lineEdit);

	QVBoxLayout *settingsLayout = new QVBoxLayout(&dialog);
    settingsLayout->addLayout(parametersLayout);
    settingsLayout->addWidget(portPushButton);

    dialog.setWindowTitle("SETTIINGS");
    dialog.setWindowIcon(QIcon(":/images/k_2000.jpg"));

    setMinimumWidth(1100);
    setMinimumHeight(600);
    showMaximized();

    foreach (const QSerialPortInfo &serialPortInfo,QSerialPortInfo::availablePorts()) {
        comboBox->addItem(serialPortInfo.portName());
    }

    QStringList stringList;

    stringList.clear();
    stringList << "19200" << "9600" << "4800" << "2400" << "1200" << "600" << "300";
    comboBox2->addItems(stringList);

    stringList.clear();
    stringList << "NONE" << "XON XOFF";
    comboBox4->addItems(stringList);

    stringList.clear();
    stringList << "DC Voltage" << "AC Voltage" << "DC Current" << "AC Current" << "Resistance 2" << "Resistance 4" << "FREQ" << "TEMP";
    comboBox5->addItems(stringList);

    stringList.clear();
    stringList << "0.01" << "1.0" << "10.0";
    comboBox6->addItems(stringList);
    comboBox6->setCurrentIndex(1);

	spinBox->setMinimum(1);
	spinBox->setMaximum(100);

    digitsSpinBox->setMinimum(4);
    digitsSpinBox->setMaximum(7);
    digitsSpinBox->setValue(7);

    graphCheckBox->setChecked(true);

//---------------------настройка графика--------------------//
    customPlot->setBackground(palette().window());
    customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    customPlot->xAxis->grid()->setPen(QPen(Qt::black,1.0,Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridVisible(true);
    customPlot->xAxis->grid()->setSubGridPen(QPen(Qt::gray,0.5,Qt::DotLine));
	customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
	customPlot->yAxis->grid()->setPen(QPen(Qt::black,1.0,Qt::DotLine));
	customPlot->yAxis->grid()->setSubGridVisible(true);
	customPlot->yAxis->grid()->setSubGridPen(QPen(Qt::gray,0.5,Qt::DotLine));
    customPlot->axisRect()->setupFullAxesBox(true);
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    connect (customPlot,&QCustomPlot::mouseRelease,[&](QMouseEvent *event) {
        if (event->button() == Qt::RightButton) {
			customPlot->rescaleAxes();
			customPlot->replot();
		}
	});
    cpcurve = new QCPCurve(customPlot->xAxis,customPlot->yAxis);
//---------------------настройка графика end--------------------//

    connect (lineEdit,&QLineEdit::textChanged,this,&MainWindow::Check);
    connect (portPushButton,&QPushButton::clicked,this,&MainWindow::Connect);
    connect (startPushButton,&QPushButton::clicked,this,&MainWindow::Start);
    connect (stopPushButton,&QPushButton::clicked,this,&MainWindow::Stop);
    connect (filterPushButton,&QPushButton::clicked,this,&MainWindow::Filtr);
    connect (lineEdit,&QLineEdit::textChanged,file,&QFile::setFileName);
    connect (port,&QSerialPort::readyRead,this,&MainWindow::Read);

	stream.setDevice(file);

    portPushButton->setEnabled(false);
    portPushButton->setCheckable(true);
	startPushButton->setEnabled(false);
	stopPushButton->setEnabled(false);
    filterPushButton->setCheckable(true);
}
MainWindow::~MainWindow() {
    if (port->isOpen()) {
        port->write(":SYSTem:LOCal\r");
        port->waitForBytesWritten(1000);
        QThread::msleep(50);
        port->close();
    }
    if ((stream.device())->isOpen()) {
        qobject_cast<QFile*>(stream.device())->flush();
        qobject_cast<QFile*>(stream.device())->close();
    }
}
void MainWindow::Save() {
    QString fileNameString = QFileDialog::getSaveFileName(this,tr("Save Address Book"), "",tr("JPG (*.jpg);;PNG (*.png);;All Files (*)"));
    QFile file(fileNameString);
    if (file.open(QIODevice::WriteOnly))
        customPlot->saveJpg(fileNameString);
}
void MainWindow::Connect() {
    if (port->isOpen())
        port->close();
    port->setPortName(comboBox->currentText());
    port->setBaudRate(comboBox2->currentText().toInt());
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    if (comboBox4->currentIndex() == 0)
        port->setFlowControl(QSerialPort::NoFlowControl);
    if (comboBox4->currentIndex() == 1)
        port->setFlowControl(QSerialPort::SoftwareControl);

    if (!(port->open(QIODevice::ReadWrite))) {
        port->close();
        dialog.close();
        QMessageBox::critical(this,"error","error");
    } else {
        Write("*RST\r");
        byteArrayMultimetr.clear();
        Write("*IDN?\r");

        time = QTime::currentTime().addMSecs(1000);
        while (QTime::currentTime() < time) qApp->processEvents();

        if (byteArrayMultimetr.split(' ').at(0) == "KEITHLEY") {
            startPushButton->setEnabled(true);
            stopPushButton->setEnabled(true);
            filterPushButton->setEnabled(true);

            if (comboBox5->currentIndex() == 0) {
                port->write(QString(":CONFigure:VOLTage:DC\r").toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":VOLTage:DC:NPLCycles %1\r").arg(comboBox6->currentText()).toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":VOLTage:DC:DIGits %1\r").arg(digitsSpinBox->value()).toUtf8());
                port->waitForBytesWritten(1000);

                customPlot->yAxis->setLabel("VOLTEGE DC, [V]");
                customPlot->xAxis->setLabel("TIME, [s]");
            }
            if (comboBox5->currentIndex() == 1) {
                port->write(QString(":CONFigure:VOLTage:AC\r").toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":VOLTage:AC:DIGits %1\r").arg(digitsSpinBox->value()).toUtf8());
                port->waitForBytesWritten(1000);

                customPlot->yAxis->setLabel("VOLTEGE AC, [V]");
                customPlot->xAxis->setLabel("TIME, [s]");
            }
            if (comboBox5->currentIndex() == 2) {
                port->write(QString(":CONFigure:CURRent:DC\r").toUtf8());
                port->waitForBytesWritten(1000);
                QThread::msleep(50);

                port->write(QString(":CURRent:DC:NPLCycles %1\r").arg(comboBox6->currentText()).toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":CURRent:DC:DIGits %1\r").arg(digitsSpinBox->value()).toUtf8());
                port->waitForBytesWritten(1000);

                customPlot->yAxis->setLabel("CURRENT DC, [A]");
                customPlot->xAxis->setLabel("TIME, [s]");
            }
            if (comboBox5->currentIndex() == 3) {
                port->write(QString(":CONFigure:CURRent:AC\r").toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":CURRent:AC:DIGits %1\r").arg(digitsSpinBox->value()).toUtf8());
                port->waitForBytesWritten(1000);

                customPlot->yAxis->setLabel("CURRENT AC, [A]");
                customPlot->xAxis->setLabel("TIME, [s]");
            }
            if (comboBox5->currentIndex() == 4) {
                port->write(QString(":CONFigure:RESistance\r").toUtf8());
                port->waitForBytesWritten(1000);
                QThread::msleep(50);

                port->write(QString(":RESistance:NPLCycles %1\r").arg(comboBox6->currentText()).toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":RESistance:DIGits %1\r").arg(digitsSpinBox->value()).toUtf8());
                port->waitForBytesWritten(1000);

                customPlot->yAxis->setLabel("RESISTANCE 2, [Ohm]");
                customPlot->xAxis->setLabel("TIME, [s]");
            }
            if (comboBox5->currentIndex() == 5) {
                port->write(QString(":CONFigure:FRESistance\r").toUtf8());
                port->waitForBytesWritten(1000);
                QThread::msleep(50);

                port->write(QString(":FRESistance:NPLCycles %1\r").arg(comboBox6->currentText()).toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":FRESistance:DIGits %1\r").arg(digitsSpinBox->value()).toUtf8());
                port->waitForBytesWritten(1000);

                customPlot->yAxis->setLabel("RESISTANCE 4, [Ohm]");
                customPlot->xAxis->setLabel("TIME, [s]");
            }
            if (comboBox5->currentIndex() == 6) {
                port->write(QString(":CONFigure:FREQuency\r").toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":FREQuency:DIGits %1\r").arg(digitsSpinBox->value()).toUtf8());
                port->waitForBytesWritten(1000);

                customPlot->yAxis->setLabel("FREQUENCY, [HZ]");
                customPlot->xAxis->setLabel("TIME, [s]");
            }
            if (comboBox5->currentIndex() == 7) {
                port->write(QString(":CONFigure:TEMPerature\r").toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":TEMPerature:NPLCycles %1\r").arg(comboBox2->currentText()).toUtf8());
                port->waitForBytesWritten(1000);

                port->write(QString(":TEMPerature:DIGits %1\r").arg(digitsSpinBox->value()).toUtf8());
                port->waitForBytesWritten(1000);

                customPlot->yAxis->setLabel("TEMPERATURE, [Deg]");
                customPlot->xAxis->setLabel("TIME, [s]");
            }

            dialog.close();
        } else {
            port->close();
            dialog.close();
            QMessageBox::critical(this,"error","error");
        }
    }
}
void MainWindow::Start() {
	qobject_cast<QFile*>(stream.device())->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);
    cpcurve->data().data()->clear();
    bolen = true;
	startPushButton->setEnabled(false);

	time.start();
    while (bolen) {
        Write(":READ?");
        while (boolDataReadyMultimetr && bolen)
            qApp->processEvents();
        qApp->processEvents();
	}

    qobject_cast<QFile*>(stream.device())->flush();
	qobject_cast<QFile*>(stream.device())->close();
}
void MainWindow::Stop() {
    bolen = false;
    startPushButton->setEnabled(true);
}
void MainWindow::Filtr() {
    if (filterPushButton->isCheckable()) {
        filterPushButton->setCheckable(false);
        if (comboBox5->currentIndex() == 0) {
            Write(QString(":VOLT:DC:AVER:COUNT %1").arg(spinBox->value()));
            Write(":VOLT:DC:AVER:STAT ON");
        }
        if (comboBox5->currentIndex() == 1) {
            Write(QString(":VOLT:AC:AVER:COUNT %1").arg(spinBox->value()));
            Write(":VOLT:AC:AVER:STAT ON");
        }
        if (comboBox5->currentIndex() == 2) {
            Write(QString(":CURR:DC:AVER:COUNT %1").arg(spinBox->value()));
            Write(":CURR:DC:AVER:STAT ON");
        }
        if (comboBox5->currentIndex() == 3) {
            Write(QString(":CURR:AC:AVER:COUNT %1").arg(spinBox->value()));
            Write(":CURR:AC:AVER:STAT ON");
        }
        if (comboBox5->currentIndex() == 4) {
            Write(QString(":RESistance:AVER:COUNT %1").arg(spinBox->value()));
            Write(":RESistance:AVER:STAT ON");
        }
        if (comboBox5->currentIndex() == 5) {
            Write(QString(":FRESistance:AVER:COUNT %1").arg(spinBox->value()));
            Write(":FRESistance:AVER:STAT ON");
        }
    } else {
        filterPushButton->setCheckable(true);
        if (comboBox5->currentIndex() == 0) Write(":VOLT:DC:AVER:STAT OFF");
        if (comboBox5->currentIndex() == 1) Write(":VOLT:AC:AVER:STAT OFF");
        if (comboBox5->currentIndex() == 2) Write(":CURR:DC:AVER:STAT OFF");
        if (comboBox5->currentIndex() == 3) Write(":CURR:AC:AVER:STAT OFF");
        if (comboBox5->currentIndex() == 4) Write(":RESistance:AVER:STAT OFF");
        if (comboBox5->currentIndex() == 5) Write(":FRESistance:AVER:STAT OFF");
    }
}
void MainWindow::Write(QString string) {
    byteArrayMultimetr.clear();
    boolDataReadyMultimetr = true;
    if (port->isOpen()) {
        port->write(string.toUtf8().append('\r'));
        port->waitForBytesWritten(1000);
    }
}
void MainWindow::Read() {
    byteArrayMultimetr += port->readAll();
    if (byteArrayMultimetr.contains('\r')) {
        if ((stream.device())->isOpen()) {
            stream << QString("%1 %2 %3 %4").arg(QString::number(byteArrayMultimetr.simplified().toDouble(),'f',digitsSpinBox->value())).arg(time.fromMSecsSinceStartOfDay(time.elapsed()).toString("hh:mm:ss:zzz")).arg(time.currentTime().toString("hh:mm:ss:zzz")).arg(time.elapsed() / 1000.0) << endl;
            cpcurve->addData(time.elapsed() / 1000.0,byteArrayMultimetr.simplified().toDouble());
            if (graphCheckBox->isChecked()) {
                customPlot->xAxis->setRange(time.elapsed() / 1000.0,time.elapsed() / 1000.0,Qt::AlignRight);
                customPlot->replot();
            }
        }
        boolDataReadyMultimetr = false;
    }
}
