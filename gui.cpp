#include "gui.h"
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <filesystem>
#include <utility>
#include "Employee.h"
#include "Work.h"
#include "nlohmann/json.hpp"
#include <QMessageBox>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QCheckBox>
#include <QObject>
#include <QLineEdit>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    if(!std::filesystem::exists("employees.json"))std::ofstream outFile("employees.json");
    std::map<int,std::string> map = mapFromJSON();
    for(auto e:map){work_schedule.addEmployee(Employee(e.first,e.second));}
    QWidget *centralWidget = new QWidget(this); 
    setCentralWidget(centralWidget);
    
    centralWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #f5f5f5;"
        "}"
        "QPushButton {"
        "   background-color: #2196F3;"
        "   color: white;"
        "   border: none;"
        "   padding: 12px 24px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border-radius: 6px;"
        "   margin: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0D47A1;"
        "}"
    );
    
    QPushButton *generatePDF = new QPushButton("📅 Generate Schedule", this);
    QPushButton *addEmployee = new QPushButton("➕ Add Employee", this);
    QPushButton *removeEmployee = new QPushButton("➖ Remove Employee", this);
    QPushButton *addConstraint = new QPushButton("⚙️ Add Constraint", this);
    QPushButton *addVacation = new QPushButton("🏖️ Add Vacation", this);
    
    generatePDF->setStyleSheet(generatePDF->styleSheet() + "QPushButton { background-color: #4CAF50; } QPushButton:hover { background-color: #45a049; }");
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(15);
    
    QLabel *titleLabel = new QLabel("Schedule Management System");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333; margin-bottom: 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    layout->addWidget(addEmployee);
    layout->addWidget(removeEmployee);
    layout->addWidget(addConstraint);
    layout->addWidget(addVacation);
    layout->addWidget(generatePDF);
    
    centralWidget->setLayout(layout);
    setWindowTitle("Schedule Management");
    resize(600, 500);
    
    connect(generatePDF, &QPushButton::clicked, this, &MainWindow::onGenerateScheduleClicked);
    connect(addConstraint, &QPushButton::clicked, this, &MainWindow::Constraint);
    connect(addEmployee, &QPushButton::clicked, this, &MainWindow::addEmployee);
    connect(removeEmployee, &QPushButton::clicked, this, &MainWindow::removeEmployee);
    connect(addVacation, &QPushButton::clicked, this, &MainWindow::addVacation);
}

void MainWindow::Constraint() {
    QWidget *constraintWindow = new QWidget();
    constraintWindow->setAttribute(Qt::WA_DeleteOnClose); // Auto-delete when closed
    constraintWindow->setWindowTitle("Add Constraints");
    constraintWindow->resize(600, 550);
    
    constraintWindow->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "}"
        "QLabel {"
        "   color: #333;"
        "   font-size: 13px;"
        "   padding: 5px;"
        "}"
        "QComboBox {"
        "   border: 2px solid #ddd;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "   background-color: white;"
        "   font-size: 13px;"
        "   min-height: 25px;"
        "}"
        "QComboBox:hover {"
        "   border-color: #2196F3;"
        "}"
        "QCheckBox {"
        "   spacing: 8px;"
        "   font-size: 12px;"
        "}"
        "QCheckBox::indicator {"
        "   width: 18px;"
        "   height: 18px;"
        "   border-radius: 3px;"
        "   border: 2px solid #ddd;"
        "}"
        "QCheckBox::indicator:checked {"
        "   background-color: #2196F3;"
        "   border-color: #2196F3;"
        "}"
        "QPushButton {"
        "   background-color: #2196F3;"
        "   color: white;"
        "   border: none;"
        "   padding: 12px 24px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border-radius: 6px;"
        "   margin-top: 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1976D2;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(constraintWindow);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);

    QLabel *titleLabel = new QLabel("⚙️ Set Employee Constraints");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2196F3; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #ddd;");
    layout->addWidget(divider);

    std::map<int, std::string> map = mapFromJSON();
    QLabel *employeeLabel = new QLabel("Select Employee:");
    employeeLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
    QComboBox *employee = new QComboBox();

    for (auto &e : map) {
        QString text = QString::number(e.first) + " - " + QString::fromStdString(e.second);
        employee->addItem(text, e.first);
    }

    layout->addWidget(employeeLabel);
    layout->addWidget(employee);
    
    QLabel *constraintsLabel = new QLabel("Select Unavailable Shifts (Max 2):");
    constraintsLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 15px;");
    layout->addWidget(constraintsLabel);
    
    std::vector<std::string> days = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    std::vector<std::string> shifts = {"Morning Shift", "Night Shift"};
    int choices = 2;

    // Use local vector instead of member variable
    std::vector<QCheckBox*> checkboxes;

    QFrame *shiftsFrame = new QFrame();
    shiftsFrame->setStyleSheet("QFrame { background-color: #f9f9f9; border-radius: 8px; padding: 15px; }");
    QVBoxLayout *shiftsLayout = new QVBoxLayout(shiftsFrame);

    for (int day = 0; day < Work::DAYS; ++day) {
        QHBoxLayout *dayLayout = new QHBoxLayout();
        dayLayout->setSpacing(20);
        
        QLabel *dayLabel = new QLabel(QString::fromStdString(days[day]));
        dayLabel->setStyleSheet("font-weight: bold; color: #555; min-width: 100px;");
        dayLayout->addWidget(dayLabel);

        for (int shift = 0; shift < Work::NUM_OF_SHIFTS; ++shift) {
            QCheckBox *checkBox = new QCheckBox(QString::fromStdString(shifts[shift]));
            dayLayout->addWidget(checkBox);
            checkboxes.push_back(checkBox);
        }

        dayLayout->addStretch();
        shiftsLayout->addLayout(dayLayout);
    }

    layout->addWidget(shiftsFrame);

    // Connect checkboxes with proper lifetime management
    for (QCheckBox *box : checkboxes) {
        QObject::connect(box, &QCheckBox::stateChanged, [checkboxes, choices]() {
            int selectedCount = 0;
            for (QCheckBox *b : checkboxes)
                if (b->isChecked()) selectedCount++;

            if (selectedCount >= choices) {
                for (QCheckBox *b : checkboxes)
                    if (!b->isChecked()) b->setEnabled(false);
            } else {
                for (QCheckBox *b : checkboxes)
                    b->setEnabled(true);
            }
        });
    }
    
    QPushButton *submit = new QPushButton("✓ Submit Constraints");
    layout->addWidget(submit);
    
    connect(submit, &QPushButton::clicked, constraintWindow, [=]() {
        int employeeID = employee->currentData().toInt();
        std::vector<std::pair<QString, int>> selectedConstraints;

        for (int i = 0; i < checkboxes.size(); ++i) {
            if (checkboxes[i]->isChecked()) {
                selectedConstraints.push_back({ checkboxes[i]->text(), i });
            }
        }

        if (selectedConstraints.empty()) {
            QMessageBox::warning(constraintWindow, "No Selection", "Please select at least one constraint.");
            return;
        }

        qDebug() << "Employee ID:" << employeeID;
        qDebug() << "Selected constraints:";
        for (const auto &c : selectedConstraints) {
            qDebug() << " -" << c.first << "at index" << c.second;
        }
        setConstraints(employeeID,selectedConstraints);
        QMessageBox::information(constraintWindow, "Success", "Constraints saved successfully!");
        constraintWindow->close();
    });

    constraintWindow->setLayout(layout);
    constraintWindow->show();
}

void MainWindow::addEmployee(){
    QWidget *employee = new QWidget();
    employee->setWindowTitle("Add Employee");
    employee->resize(500, 350);
    
    employee->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "}"
        "QLineEdit {"
        "   border: 2px solid #ddd;"
        "   border-radius: 6px;"
        "   padding: 10px;"
        "   font-size: 13px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #2196F3;"
        "}"
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   padding: 12px 24px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border-radius: 6px;"
        "   margin-top: 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
    );
    
    QVBoxLayout *layout = new QVBoxLayout(employee);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);
    
    QLabel *titleLabel = new QLabel("➕ Add New Employee");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #4CAF50; margin-bottom: 10px;");
    layout->addWidget(titleLabel);
    
    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #ddd;");
    layout->addWidget(divider);
    
    QLabel *nameLabel = new QLabel("Employee Name:");
    nameLabel->setStyleSheet("font-weight: bold; color: #555;");
    layout->addWidget(nameLabel);
    
    QLineEdit *name = new QLineEdit();
    name->setPlaceholderText("Enter full name");
    layout->addWidget(name);
    
    QLabel *idLabel = new QLabel("Employee ID:");
    idLabel->setStyleSheet("font-weight: bold; color: #555;");
    layout->addWidget(idLabel);
    
    QLineEdit *ID = new QLineEdit();
    ID->setPlaceholderText("Enter numeric ID");
    layout->addWidget(ID);
    
    QPushButton *add = new QPushButton("✓ Add Employee");
    layout->addWidget(add);
    
    employee->show();
    
    connect(add, &QPushButton::clicked, [=]() {
        std::map<int, std::string> map = mapFromJSON();
        bool ok;
        int id = ID->text().toInt(&ok);
        if (name->text().isEmpty() || ID->text().isEmpty() || !ok) {
            QMessageBox::warning(employee, "Error", "Please enter a valid name and numeric ID.");
            return;
        }
        if (map.find(id) != map.end()) {
            QMessageBox::warning(employee, "Error", "This ID already exists.");
            return;
        }
        map[id] = name->text().toStdString();
        add2JSON(id, name->text().toStdString());
        QMessageBox::information(employee, "Success", "Employee added successfully!");
        employee->close();
    });
}

void MainWindow::removeEmployee(){
    QWidget *employee = new QWidget();
    employee->setWindowTitle("Remove Employee");
    employee->resize(500, 300);
    
    employee->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "}"
        "QLineEdit {"
        "   border: 2px solid #ddd;"
        "   border-radius: 6px;"
        "   padding: 10px;"
        "   font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #f44336;"
        "}"
        "QPushButton {"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: none;"
        "   padding: 12px 24px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border-radius: 6px;"
        "   margin-top: 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #da190b;"
        "}"
    );
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);
    
    QLabel *titleLabel = new QLabel("➖ Remove Employee");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #f44336; margin-bottom: 10px;");
    layout->addWidget(titleLabel);
    
    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #ddd;");
    layout->addWidget(divider);
    
    QLabel *idLabel = new QLabel("Enter Employee ID to Remove:");
    idLabel->setStyleSheet("font-weight: bold; color: #555;");
    layout->addWidget(idLabel);
    
    QLineEdit *ID = new QLineEdit();
    ID->setPlaceholderText("Enter employee ID");
    layout->addWidget(ID);
    
    QPushButton *remove = new QPushButton("✗ Remove Employee");
    layout->addWidget(remove);
    
    employee->setLayout(layout);
    employee->show();
    
    connect(remove, &QPushButton::clicked, [=]() {
        std::map<int,std::string> map = mapFromJSON();
        bool ok;
        int id = ID->text().toInt(&ok);
        
        if (!ok || ID->text().isEmpty()) {
            QMessageBox::warning(employee, "Error", "Please enter a valid numeric ID.");
            return;
        }
        
        if(map.find(id)!=map.end()){
            removeFromJSON(id);
            QMessageBox::information(employee, "Success", "Employee removed successfully!");
            employee->close();
        }
        else{
            QMessageBox::warning(employee,"Error","Employee ID does not exist.");
        }
    });
}

void MainWindow::addVacation() {
    QWidget *vacation = new QWidget();
    vacation->setWindowTitle("Add Vacation");
    vacation->resize(550, 450);

    vacation->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;"
        "}"
        "QComboBox {"
        "   border: 2px solid #ddd;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "   background-color: white;"
        "   font-size: 13px;"
        "   min-height: 25px;"
        "}"
        "QComboBox:hover {"
        "   border-color: #FF9800;"
        "}"
        "QLabel {"
        "   color: #555;"
        "   font-size: 13px;"
        "}"
        "QPushButton {"
        "   background-color: #FF9800;"
        "   color: white;"
        "   border: none;"
        "   padding: 12px 24px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border-radius: 6px;"
        "   margin-top: 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #F57C00;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(vacation);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel *titleLabel = new QLabel("🏖️ Add Employee Vacation");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #FF9800; margin-bottom: 10px;");
    layout->addWidget(titleLabel);
    
    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #ddd;");
    layout->addWidget(divider);

    std::map<int, std::string> map = mapFromJSON();
    std::vector<std::string> days = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

    QLabel *employees = new QLabel("Select Employee:");
    employees->setStyleSheet("font-weight: bold; font-size: 14px;");
    QComboBox *employee = new QComboBox();
    for (auto &e : map) {
        QString displayText = QString::number(e.first) + " - " + QString::fromStdString(e.second);
        employee->addItem(displayText, e.first); 
    }

    layout->addWidget(employees);
    layout->addWidget(employee);
    
    QLabel *start = new QLabel("Vacation Start Day:");
    start->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
    QComboBox *startDay = new QComboBox();
    for (const std::string &day : days)startDay->addItem(QString::fromStdString(day));
    
    QLabel *end = new QLabel("Vacation End Day:");
    end->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
    QComboBox *endDay = new QComboBox();
    for (const std::string &day : days)
        endDay->addItem(QString::fromStdString(day));
        
    QPushButton *submit = new QPushButton("✓ Submit Vacation");
    
    layout->addWidget(start);
    layout->addWidget(startDay);
    layout->addWidget(end);
    layout->addWidget(endDay);
    layout->addWidget(submit);

    vacation->setLayout(layout);
    vacation->show();
    
    connect(submit, &QPushButton::clicked, vacation, [=]() {
        int employeeID = employee->currentData().toInt();
        int startIndex = startDay->currentIndex();
        int endIndex = endDay->currentIndex();

        qDebug() << "Employee ID:" << employeeID
                << "Start day index:" << startIndex
                << "End day index:" << endIndex;

        setEmployeeVaction(employeeID, startIndex, endIndex);
        QMessageBox::information(vacation, "Success", "Vacation added successfully!");
        vacation->close();
    });

}

void MainWindow::add2JSON(int ID, std::string name) {
    std::ifstream file("employees.json");
    j=nlohmann::json::array();
    if (file.is_open()) {
        if (file.good()&&file.peek()!=std::ifstream::traits_type::eof()) {
            try {file >> j;} 
            catch (const std::exception& e){j = nlohmann::json::array();}
        }
        file.close();
    }
    if (!j.is_array()) {j = nlohmann::json::array();}
    j.push_back({{"ID", ID}, {"Name", name}});
    std::ofstream outFile("employees.json");
    outFile << j.dump(4);
    outFile.close();
}

void MainWindow::removeFromJSON(int ID){
    std::ifstream file1("employees.json");
    file1 >> j;
    for(int i=0;i<j.size();i++){
        if(j[i]["ID"]==ID){j.erase(j.begin()+i);break;}
    }
    file1.close();
    std::ofstream file2("employees.json");
    file2 << j.dump(4);
    file2.close();
}

std::map<int,std::string> MainWindow::mapFromJSON(){
    std::map<int,std::string> map;
    std::ifstream file("employees.json");
    file >> j;
    for(auto e:j){map[e["ID"]] = e["Name"];}
    file.close();
    return map;
}

void MainWindow::setEmployeeVaction(int id,int start,int end  ){
    for(Employee worker:work_schedule.getEmployees()){
        if(worker.getID()==id){
            worker.addVacation(start,end);
        }
    }
}

void MainWindow::setConstraints(int id,std::vector<std::pair<QString,int>> constraints){
    std::map<int,std::string> map = mapFromJSON();
    for(Employee worker:work_schedule.getEmployees()){
        if(worker.getID()==id){
            for(auto c:constraints)worker.addConstraint(c.second/2,c.second%2);
            }
        }
}

void MainWindow::onGenerateScheduleClicked() {
    if (work_schedule.createSchedule()) {
        work_schedule.createPDF(work_schedule);
        QMessageBox::information(this, "Success", "Schedule created and 'schedule.pdf' has been generated successfully.");
        printScheduleTable(work_schedule);
    } else {
        QMessageBox::warning(this, "Failure", "Could not create schedule. The constraints might be too restrictive.");
    }
}