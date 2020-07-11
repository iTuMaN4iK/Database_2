#include "database.h"
#include <algorithm>
#include <iterator>
void Database::Add(const Date &date, const std::string &event) {
  if (events.count(date) == 0) {
    events[date].push_back(event);
    return;
  }
  auto f = std::find(events.at(date).begin(), events.at(date).end(), event);
  if (f == events.at(date).end()) {
    events[date].push_back(event);
  }
};

bool Database::DeleteEvent(const Date &date, const std::string &event) {
  // if (events.find(date) != events.end()) {
  //   if (events[date].find(event) != events[date].end()) {
  //     events[date].erase(event);
  //     return true;
  //   }
  // }

  return false;
}
int Database::DeleteDate(const Date &date) {
  int size = 0;
  if (events.find(date) != events.end()) {
    size = events[date].size();
    events.erase(date);
  }
  return size;
}

void Database::Find(const Date &date) const {
  if (events.find(date) != events.end()) {
    for (const auto &i : events.at(date))
      std::cout << i << std::endl;
  }
};

void Database::Print(std::ostream &out) const {

  for (const auto &i : events) {
    for (const auto &j : events.at(i.first)) {
      out << i.first << " " << j << std::endl;
    }
  }
};

int Database::RemoveIf(
    const std::function<bool(const Date &, const std::string &)> predicate) {
  int count = 0;
  auto itmap = events.begin();
  while (itmap != events.end()) {
    auto it = std::stable_partition(itmap->second.begin(), itmap->second.end(),
                                    [&predicate, itmap](const auto &iset) {
                                      return !predicate(itmap->first, iset);
                                    });
    if (it != itmap->second.end()) {
      count += std::distance(it, itmap->second.end());
      itmap->second.erase(it, itmap->second.end());
    }
    if (itmap->second.empty()) {
      itmap = events.erase(itmap);
    } else {
      itmap++;
    }
  }

  return count;
}

std::vector<std::string> Database::FindIf(
    const std::function<bool(const Date &, const std::string &)> predicate)
    const {
  std::vector<std::string> entries;
  for (const auto &e : events) {
    auto it = e.second.begin();
    while (it != e.second.end()) {
      it = std::find_if(it, e.second.end(), [&predicate, e](const auto &iset) {
        return predicate(e.first, iset);
      });
      if (it != e.second.end()) {
        entries.emplace_back(e.first.getDate() + " " + *it);
        it++;
      }
    }
  }
  return entries;
}

std::string Database::Last(const Date &date) const {
  auto it = events.upper_bound(date);
  if (it == events.begin())
    throw std::invalid_argument("Last not found");
  it--;
  return {it->first.getDate() + " " + it->second.back()};
}