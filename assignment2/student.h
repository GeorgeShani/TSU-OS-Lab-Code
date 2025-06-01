#pragma once
#include <iostream>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <errno.h>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstring>
using namespace std;

const size_t TOTAL_STUDENTS = 1000;
const size_t TOP_STUDENTS_COUNT = 50;

const size_t TOTAL_GRADES = 15;
const size_t MAX_NAME_LENGTH = 50;

class Student {
private:
    int studentId;
    char lastName[MAX_NAME_LENGTH];
    char firstName[MAX_NAME_LENGTH];
    float subjectGrades[TOTAL_GRADES];
    float gradePointAverage;

public:
    Student() : studentId(0), gradePointAverage(0.0)
    {
        memset(lastName, 0, MAX_NAME_LENGTH);
        memset(firstName, 0, MAX_NAME_LENGTH);

        for (size_t i = 0; i < TOTAL_GRADES; i++)
            subjectGrades[i] = 0.0;
    }

    Student(int student_id, const string &last, const string &first) : studentId(student_id), gradePointAverage(0.0)
    {
        strncpy(lastName, last.c_str(), MAX_NAME_LENGTH - 1);
        lastName[MAX_NAME_LENGTH - 1] = '\0';

        strncpy(firstName, first.c_str(), MAX_NAME_LENGTH - 1);
        firstName[MAX_NAME_LENGTH - 1] = '\0';

        for (size_t i = 0; i < TOTAL_GRADES; i++)
            subjectGrades[i] = 0.0;
    }

    void calculateGradePointAverage()
    {
        float totalSum = accumulate(begin(subjectGrades), end(subjectGrades), 0.0);
        gradePointAverage = round((totalSum / TOTAL_GRADES) * 0.04 * 100.0) / 100.0;
    }

    bool operator>(const Student &other) const
    {
        return gradePointAverage > other.gradePointAverage;
    }

    bool operator<(const Student &other) const
    {
        return gradePointAverage < other.gradePointAverage;
    }

    bool operator>=(const Student &other) const
    {
        return gradePointAverage >= other.gradePointAverage;
    }

    bool operator<=(const Student &other) const
    {
        return gradePointAverage <= other.gradePointAverage;
    }

    friend istream &operator>>(istream &inputStream, Student &student)
    {
        string tempLast, tempFirst;
        inputStream >> student.studentId >> tempLast >> tempFirst;

        strncpy(student.lastName, tempLast.c_str(), MAX_NAME_LENGTH - 1);
        student.lastName[MAX_NAME_LENGTH - 1] = '\0';

        strncpy(student.firstName, tempFirst.c_str(), MAX_NAME_LENGTH - 1);
        student.firstName[MAX_NAME_LENGTH - 1] = '\0';

        for (size_t i = 0; i < TOTAL_GRADES; i++)
            inputStream >> student.subjectGrades[i];

        return inputStream;
    }

    friend ostream &operator<<(ostream &outputStream, const Student &student)
    {
        outputStream << fixed << setprecision(2);
        outputStream << "ID: " << student.studentId
                     << ", Name: " << student.firstName << " " << student.lastName
                     << ", GPA: " << student.gradePointAverage << ", Grades: ";

        for (size_t i = 0; i < TOTAL_GRADES; i++)
        {
            outputStream << student.subjectGrades[i];
            if (i < TOTAL_GRADES - 1)
                outputStream << " ";
        }

        return outputStream;
    }

    int getStudentId() const { return studentId; }
    string getLastName() const { return string(lastName); }
    string getFirstName() const { return string(firstName); }
    float getGradePointAverage() const { return gradePointAverage; }

    int countSubjectsWithGradesAbove90() const
    {
        int excellentGradesCount = 0;
        for (size_t i = 0; i < TOTAL_GRADES; i++)
            if (subjectGrades[i] > 90)
                excellentGradesCount++;

        return excellentGradesCount;
    }
};