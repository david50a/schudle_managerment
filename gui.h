#ifndef GUI_H
#define GUI_H
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include <map>
#include <string>
#include <nlohmann/json.hpp>
#include "Work.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onGenerateScheduleClicked();
    void Constraint();
    void removeEmployee();
    void addEmployee();
    void addVacation();
    void removeFromJSON(int ID);
    void setEmployeeVaction(int id,int start,int end);
    void setConstraints(int id,std::vector<std::pair<QString,int>> constraints);
    std::map<int,std::string> mapFromJSON();
    void add2JSON(int ID,std::string name);
private:
    QWidget *centralWidget;
    QVBoxLayout *layout;
    nlohmann::json j;
    Work work_schedule;
    std::vector<QCheckBox*> allCheckboxes;
    std::vector<QLineEdit*> dayInputs;

    int maxShiftsPerWeek = 2; 
};

#endif
