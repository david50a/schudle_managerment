#ifndef WORK_H
#define WORK_H

#include "Employee.h"
#include <vector>
#include <set>
#include <map>
#include <iostream>

class Work {
public:
    static const int NUM_OF_SHIFTS = 2;
    static const int DAYS = 6;
    int createPDF(const Work& work_schedule);

private:
    std::vector<Employee> employees;
    std::vector<std::vector<std::vector<int>>> schedule;
    std::vector<std::vector<int>> employees_per_shift;

public:
    Work();
    void setEmployeesPerShift(int day, int shift, int num_of_employees);
    void addEmployee(const Employee& e);
    const std::vector<Employee>& getEmployees() const;
    const std::vector<std::vector<std::vector<int>>>& getSchedule() const;
    bool createSchedule();

private:
    bool solve(int day, int shift);
    bool solveForShift(int day, int shift, int employee_idx, std::set<int>& working_today);
    friend void printScheduleTable(const Work& work_schedule);
};

void printScheduleTable(const Work& work_schedule);

#endif // WORK_H
