#include "readinfo.h"

ReadInfo::ReadInfo(QObject *parent) : QObject(parent)
{
    controller = Controller::getInstance();
}

void ReadInfo::readDataFromController()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "logs/six-r-log-%d-%m-%Y %H-%M-%S.csv");
    auto csv_file_name = oss.str();

    std::fstream fout;
    fout.open(csv_file_name, std::ios::out | std::ios::app);


    while (true) {
         controller->beckhoff->ReadInfoFromRobot();

         auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

         fout << current_time.count();


         for(int i=0; i< controller->beckhoff->NumberOfRobotMotors; i++) {
             fout << ",";
             fout << controller->beckhoff->ActualPositions[i];
         }

         fout << "\n" << std::flush;

         std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
