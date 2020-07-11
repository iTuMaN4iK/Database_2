#pragma once
#include "date.h"
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
class Database {
public:
  void Add(const Date &date, const std::string &event);
  bool DeleteEvent(const Date &date, const std::string &event);
  int DeleteDate(const Date &date);
  void Find(const Date &date) const;
  void Print(std::ostream &out) const;
  int RemoveIf(
      const std::function<bool(const Date &, const std::string &)> predicate);
  std::vector<std::string>
  FindIf(const std::function<bool(const Date &, const std::string &)> predicate)
      const;
  std::string Last(const Date &date) const;

private:
  std::map<Date, std::set<std::string>> events;
  std::map<Date, std::string> eventsLast;
};