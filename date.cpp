#include "date.h"
#include <vector>

Date::Date(std::string &date) {
  CheckDate(date);
  std::stringstream ss(date);
  ss >> year;
  ss.ignore();
  ss >> month;
  ss.ignore();
  ss >> day;
}
Date::Date(int Year, int Month, int Day) : year(Year), month(Month), day(Day) {}

void Date::CheckDate(const std::string &date) {
  std::stringstream ss(date);
  int in;
  ss >> in;
  if (ss.peek() != '-') {
    throw std::runtime_error("Wrong date format: " + date);
  }
  ss.ignore();
  ss >> in;
  if (ss.peek() != '-') {
    throw std::runtime_error("Wrong date format: " + date);
  }
  if (in < 1 || 12 < in) {
    throw std::runtime_error("Month value is invalid: " + std::to_string(in));

  } else {
    ss.ignore();
    ss >> in;
    if (in < 1 || 31 < in) {
      throw std::runtime_error("Day value is invalid: " + std::to_string(in));
    }
  }
  std::string tail;
  if (ss >> tail) {
    throw std::invalid_argument("Wrong date format: " + date);
  }
}
int Date::GetYear() const { return year; };
int Date::GetMonth() const { return month; };
int Date::GetDay() const { return day; };
std::string Date::getDate() const {
  std::stringstream stream;
  stream << std::setw(4) << std::setfill('0') << year << "-" << std::setw(2)
         << std::setfill('0') << month << "-" << std::setw(2)
         << std::setfill('0') << day;
  return stream.str();
}

bool operator<(const Date &lhs, const Date &rhs) {
  return std::vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} <
         std::vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
};
bool operator<=(const Date &lhs, const Date &rhs) {
  return std::vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} <=
         std::vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
}
bool operator>(const Date &lhs, const Date &rhs) {
  return std::vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} >
         std::vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
}
bool operator>=(const Date &lhs, const Date &rhs) {
  return std::vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} >=
         std::vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
}
bool operator==(const Date &lhs, const Date &rhs) {
  return std::vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} ==
         std::vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
}
bool operator!=(const Date &lhs, const Date &rhs) {
  return std::vector<int>{lhs.GetYear(), lhs.GetMonth(), lhs.GetDay()} !=
         std::vector<int>{rhs.GetYear(), rhs.GetMonth(), rhs.GetDay()};
}
std::ostream &operator<<(std::ostream &stream, const Date &date) {
  stream << std::setw(4) << std::setfill('0') << date.GetYear() << "-"
         << std::setw(2) << std::setfill('0') << date.GetMonth() << "-"
         << std::setw(2) << std::setfill('0') << date.GetDay();
  return stream;
}
Date ParseDate(std::istream &is) {
  std::string str;
  is >> str;
  return Date(str);
}
