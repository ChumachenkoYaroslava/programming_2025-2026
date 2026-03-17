#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <algorithm>
#include<cstdlib>

#include "2L2.hpp"

using namespace std;


bool Clock::isValidDayOfWeek(const string& dow) {
    string validDays[] = {"Понедельник", "Вторник", "Среда", "Четверг", 
                            "Пятница", "Суббота", "Воскресенье"};
    for (int i = 0; i < 7; i++){
        if (dow == validDays[i]) {
            return true;
        }
    }
    return false;
}
bool Clock::isValidDate(int d, int m, int y, int h, int min) {
if (m < 1 || m > 12) return false;
if (h < 0 || h > 23) return false;
if (min < 0 || min > 59) return false;

int daysInMonth;
switch (m) {
    case 4: case 6: case 9: case 11:
        daysInMonth = 30;
        break;
    case 2:
        daysInMonth = 28;
        break;
    default:
        daysInMonth = 31;
}

if (d < 1 || d > daysInMonth) return false;
return true;
}

Clock::Clock(int d, int m, int y, int h, int min, const string& dow, vector<string> tasks_) {
    if (isValidDate(d, m, y, h, min) && isValidDayOfWeek(dow)) {
        day = d;
        month = m;
        year = y;
        hours = h;
        minutes = min;
        dayOfWeek = dow;
        tasks = tasks_;
    } else {
        day = 1;
        month = 1;
        year = 2000;
        hours = 0;
        minutes = 0;
        dayOfWeek = "Понедельник";
        tasks = {};
        cout << "Введены некорректные данные, установлены значения по умолчанию" << endl;
    }
    cout << "Вызван конструктор полного заполнения" << endl;
}

Clock::Clock(const Clock& other) {
    day = other.day;
    month = other.month;
    year = other.year;
    hours = other.hours;
    minutes = other.minutes;
    dayOfWeek = other.dayOfWeek;
    cout << "Вызван конструктор копирования" << endl;
}

Clock::~Clock() {
    tasks.clear();
    cout << "Вызван деструктор для объекта Clock" << endl;
}


void Clock::displayInfo() const {
    cout << "Текущая дата и время:" << endl;
    cout << setfill('0') 
            << setw(2) << day << "."
            << setw(2) << month << "."
            << setw(4) << year << " "
            << setw(2) << hours << ":"
            << setw(2) << minutes << endl;
    cout << "День недели: " << dayOfWeek << endl;
    cout << "Дела:" << endl;
    for (const string& el: tasks) cout << el << endl;
}

void Clock::addMinutes(int x) {
    int totalMinutes = hours * 60 + minutes + x;
    
    int daysToAdd = totalMinutes / (24 * 60);
    totalMinutes %= (24 * 60);
    
    hours = totalMinutes / 60;
    minutes = totalMinutes % 60;
    
    while (daysToAdd > 0) {
        day++;
        
        int daysInMonth;
        switch (month) {
            case 4: case 6: case 9: case 11:
                daysInMonth = 30;
                break;
            case 2:
                daysInMonth = 28;
                break;
            default:
                daysInMonth = 31;
        }
        
        if (day > daysInMonth) {
            day = 1;
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }
        daysToAdd--;
    }
    
    string daysOfWeek[] = {"Понедельник", "Вторник", "Среда", "Четверг", 
                            "Пятница", "Суббота", "Воскресенье"};
    
    int currentIndex = 0;
    for (int i = 0; i < 7; i++) {
        if (daysOfWeek[i] == dayOfWeek) {
            currentIndex = i;
            break;
        }
    }
    
    int newIndex = (currentIndex + (x / (24 * 60))) % 7;
    dayOfWeek = daysOfWeek[newIndex];
}

Clock Clock::operator+(const Clock& other){
    Clock res(*this);
    for (const string& el: other.tasks) tasks.push_back(el);
    // unique(tasks.begin(), tasks.end());
    res.addMinutes(10140);
    return res;
}

Clock Clock::operator/(const Clock& other){
    Clock res(*this);
    res.addMinutes(10140);
    vector<string> new_tasks;
    for (int i = 0; i < rand() % other.tasks.size(); i++) new_tasks.push_back(other.tasks[i]);
    for (int i = 0; i < rand() % tasks.size(); i++) new_tasks.push_back(tasks[i]);
    // unique(new_tasks.begin(), new_tasks.end());
    res.setTasks(new_tasks);
    return res;
}


int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    Clock clock1;
    cout << "\nОбъект clock1 (конструктор по умолчанию):" << endl;
    clock1.displayInfo();
    

    Clock clock2(15, 3, 2024, 14, 30, "Пятница", {"Сделать чаю", "Выпить чаю"});
    cout << "\nОбъект clock2 (конструктор полного заполнения):" << endl;
    clock2.displayInfo();
    
    Clock clock3(clock2);
    cout << "\nОбъект clock3 (конструктор копирования):" << endl;
    clock3.displayInfo();
    

    cout << "\nПроверка сеттеров и геттеров:" << endl;
    clock1.setDay(20);
    clock1.setMonth(12);
    clock1.setYear(2024);
    clock1.setHours(10);
    clock1.setMinutes(45);
    clock1.setDayOfWeek("Суббота");
    
    cout << "После установки значений:" << endl;
    cout << "День: " << clock1.getDay() << endl;
    cout << "Месяц: " << clock1.getMonth() << endl;
    cout << "Год: " << clock1.getYear() << endl;
    cout << "Часы: " << clock1.getHours() << endl;
    cout << "Минуты: " << clock1.getMinutes() << endl;
    cout << "День недели: " << clock1.getDayOfWeek() << endl;
    
    cout << "\nДобавление 150 минут к clock2:" << endl;
    clock2.addMinutes(150);
    clock2.displayInfo();
    
    cout << "\nПроверка на некорректные данные:" << endl;
    clock1.setDayOfWeek("InvalidDay");
    clock1.setHours(25);
    
    cout << "\nПрограмма завершена, начинается автоматический вызов деструкторов:" << endl;
    
    return 0;
}