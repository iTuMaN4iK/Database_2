#pragma once
#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
class Date {
public:
  Date() = default;
  Date(std::string &date);
  Date(int Year, int Month, int Day);

  void CheckDate(const std::string &date);
  int GetYear() const;
  int GetMonth() const;
  int GetDay() const;
  std::string getDate() const;

private:
  int year;
  int month;
  int day;
};

bool operator<(const Date &lhs, const Date &rhs);
bool operator<=(const Date &lhs, const Date &rhs);
bool operator>(const Date &lhs, const Date &rhs);
bool operator>=(const Date &lhs, const Date &rhs);
bool operator==(const Date &lhs, const Date &rhs);
bool operator!=(const Date &lhs, const Date &rhs);

std::ostream &operator<<(std::ostream &stream, const Date &date);

Date ParseDate(std::istream &is);