#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "binaryevolvinggraphreader.h"
#include "graphviewer.h"
#include <QDebug>
#include <QLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <limits>
#include <QTextStream>

#include "agent.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));

    this->setGeometry(100,100,600,600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open()
{
//    QString filename = QFileDialog::getOpenFileName(this, "Ouvrir une trace", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "Fichier trace (*.csv);;Tous les fichiers (*.*)");
    QString filename = "/Users/ben/Data/dublin-bus-analysis/busAnalysis/trace.csv";
    QFile file(filename);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        return;
    }

    gv = new GraphViewer();

    quint64 startTimestamp = ULLONG_MAX;
    quint64 endTimestamp = 0;
    double minLatitude = std::numeric_limits<float>::max();
    double minLongitude = std::numeric_limits<float>::max();
    double maxLatitude = -std::numeric_limits<float>::max();
    double maxLongitude = -std::numeric_limits<float>::max();

    qDebug() << startTimestamp << endTimestamp;

    // We read the trace file and extract all the agents == nodes to show
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString line = in.readLine();
//        qDebug() << line;
        QStringList splitted = line.split(";");

        if (splitted.size() != 4){
            // Avoid errors
            continue;
        }
        // Line formatted this way: "timestamp;id;longitude;latitude\n"
        quint64 ts = splitted[0].toLongLong();
        int id = splitted[1].toInt();
        double lon = splitted[3].toFloat();
        double lat = splitted[2].toFloat();

        // Update the min and max latitude and longitude
        if(lon < minLongitude) { minLongitude = lon; }
        if(lon > maxLongitude) { maxLongitude = lon; }
        if(lat < minLatitude)  { minLatitude  = lat; }
        if(lat > maxLatitude)  { maxLatitude  = lat; }

        // Update the start and end of the simulation
        if(ts < startTimestamp) { startTimestamp = ts; }
        if(ts > endTimestamp)   { endTimestamp = ts; }

        if(gv->agents.contains(id)) {
            // if instance of agent id exists, add the position with the matching timestamp
            gv->agents[id]->addPosition(ts, QGeoCoordinate(lat, lon));
        } else {
            // otherwise create new Agent instance and add the position of the point
            gv->agents.insert(id, new Agent(id,100));
            gv->agents[id]->addPosition(ts, QGeoCoordinate(lat, lon));
        }
    }
    file.close();

    // Set the start and end of the simulation
    gv->setStartTimestamp(startTimestamp);
    gv->setEndTimestamp(endTimestamp);
    gv->setTopLeft(QGeoCoordinate(minLatitude,minLongitude));
    gv->setBottomRight(QGeoCoordinate(maxLatitude,maxLongitude));

    qDebug() << startTimestamp << endTimestamp << gv->getTopLeft() << gv->getBottomRight();

    Agent * agent0 = gv->agents[1];
    Agent * agent1 = gv->agents[2];
    agent0->contacts(agent1, 1e6);

//    qDebug() << "Writing contacts";
//    QFile file1("/Users/ben/Data/dublin-bus-analysis/busAnalysis/contacts_out.txt");
//    file1.open(QIODevice::WriteOnly | QIODevice::Text);
//    QTextStream out(&file1);
//    quint64 start = qMax(agent0->positions.keys().first(), agent1->positions.keys().first());
//    quint64 end  = qMin(agent0->positions.keys().last(), agent1->positions.keys().last());
//    for (quint64 ts = start; ts <= end; ts += 1e6) {
//        QGeoCoordinate pos1 = agent0->getPosition(ts);
//        QGeoCoordinate pos2 = agent1->getPosition(ts);
//        qreal distance = pos1.distanceTo(pos2);
////        qDebug() << ts << distance << pos1.toString() << pos2.toString() << "\n";
//        out << ts << "\t" << distance << "\n";
//    }
//    qDebug() << "End writing contacts";



    this->setCentralWidget(gv);
//    gv->startTimer();
}
