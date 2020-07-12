#include "database.h"
#include <algorithm>
#include <iterator>
void Database::Add(const Date &date, const std::string &event) {
  if (eventsLast.count(date) == 0) {
    eventsLast[date].push_back(event);
    events[date].insert(event);
    return;
  }
  auto res = events.at(date).insert(event);
  if (res.second) {
    eventsLast[date].push_back(event);
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
  for (const auto &i : eventsLast) {
    for (const auto &j : eventsLast.at(i.first)) {
      out << i.first << " " << j << std::endl;
    }
  }
};

int Database::RemoveIf(
    const std::function<bool(const Date &, const std::string &)> predicate) {
  int count = 0;

  auto mit = eventsLast.begin();
  while (mit != eventsLast.end()) {
    bool bErase = false;
    auto it = std::stable_partition(mit->second.begin(), mit->second.end(),
                                    [&predicate, mit](const auto &iset) {
                                      return !predicate(mit->first, iset);
                                    });
    if (it != mit->second.end()) {
      bErase = true;
      count += std::distance(it, mit->second.end());
      mit->second.erase(it, mit->second.end());
    }
    if (bErase) {
      events.at(mit->first).clear();
      std::copy(
          mit->second.begin(), mit->second.end(),
          std::inserter(events.at(mit->first), events.at(mit->first).begin()));
    }
    if (mit->second.empty()) {
      events.erase(mit->first);
      mit = eventsLast.erase(mit);
    } else {
      mit++;
    }
  }

  return count;
}

std::vector<std::string> Database::FindIf(
    const std::function<bool(const Date &, const std::string &)> predicate)
    const {
  std::vector<std::string> entries;
  for (const auto &e : eventsLast) {
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
  auto it = eventsLast.upper_bound(date);
  if (it == eventsLast.begin())
    throw std::invalid_argument("Last not found");
  it--;
  return {it->first.getDate() + " " + it->second.back()};
}