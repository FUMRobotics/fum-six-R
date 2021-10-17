#ifndef READINFO_H
#define READINFO_H


#include "Model/Controller/controller.h"
#include "Model/Controller/beckhoff.h"
#include <iomanip>
#include <iostream>
#include <ctime>
#include <sstream>
#include <chrono>
#include <fstream>

#include <QObject>

class ReadInfo: public QObject
{
    Q_OBJECT
public:
    explicit ReadInfo(QObject *parent = nullptr);
public Q_SLOTS:
    void readDataFromController();
private:
    Controller *controller;
};

#endif // READINFO_H
