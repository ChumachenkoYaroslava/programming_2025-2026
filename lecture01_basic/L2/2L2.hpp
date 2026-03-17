#pragma once
#include <string>
#include <iostream>
#include <vector>
using namespace std;

class Clock {
private:
    int day;
    int month;
    int year;
    int hours;
    int minutes;
    string dayOfWeek;
    vector<string> tasks;
    
    bool isValidDayOfWeek(const string& dow);
    bool isValidDate(int d, int m, int y, int h, int min);

public:
    Clock() : day(1), month(1), year(2000), hours(0), minutes(0), dayOfWeek("Понедельник"), tasks({}) {
        cout << "Вызван конструктор по умолчанию" << endl;
    }

    Clock(int d, int m, int y, int h, int min, const string& dow, vector<string>tasks);
    
    Clock(const Clock& other);
    
    ~Clock();
    
    vector<string> getTasks() const { return tasks; }
    int getDay() const { return day; }
    int getMonth() const { return month; }
    int getYear() const { return year; }
    int getHours() const { return hours; }
    int getMinutes() const { return minutes; }
    string getDayOfWeek() const { return dayOfWeek; }
    
    void setDayOfWeek(const string& dow) {
        if (isValidDayOfWeek(dow)) {
            dayOfWeek = dow;
        } else {
            cout << "Ошибка: неверный формат дня недели" << endl;
        }
    }
    
    void setDay(int d) {
        if (isValidDate(d, month, year, hours, minutes)) {
            day = d;
        } else {
            cout << "Ошибка: неверное значение дня" << endl;
        }
    }
    
    void setMonth(int m) {
        if (isValidDate(day, m, year, hours, minutes)) {
            month = m;
        } else {
            cout << "Ошибка: неверное значение месяца" << endl;
        }
    }
    
    void setYear(int y) {
        if (isValidDate(day, month, y, hours, minutes)) {
            year = y;
        } else {
            cout << "Ошибка: неверное значение года" << endl;
        }
    }
    
    void setHours(int h) {
        if (isValidDate(day, month, year, h, minutes)) {
            hours = h;
        } else {
            cout << "Ошибка: неверное значение часов" << endl;
        }
    }
    
    void setMinutes(int min) {
        if (isValidDate(day, month, year, hours, min)) {
            minutes = min;
        } else {
            cout << "Ошибка: неверное значение минут" << endl;
        }
    }

    void setTasks(vector<string> tasks_){
        tasks = tasks_;
    }
    
    void displayInfo() const;
    
    void addMinutes(int x);

    Clock operator+(const Clock& other);
    Clock operator/(const Clock& other);
};

