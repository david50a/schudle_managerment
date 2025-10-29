#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <string>
#include <vector>
#include <utility>

class Employee {
private:
    std::string name;
    int ID;
    std::vector<std::pair<int, int>> constraints;
    std::pair<int, int> vacations;
public:
    Employee(int e_ID, std::string e_name) : ID(e_ID), name(std::move(e_name)), vacations({-1, -1}) {}
    std::string getName() const { return name; }
    int getID() const { return ID; }
    void addVacation(int start_day, int end_day){ vacations = {start_day, end_day};}
    void addConstraint(int day, int shift) {constraints.push_back({day, shift});}
    const std::vector<std::pair<int, int>>& getConstraints() const {return constraints;}
    const std::pair<int, int>& getVacations() const {return vacations;}
    bool canWork(int day, int shift) const;
};

#endif // EMPLOYEE_H
