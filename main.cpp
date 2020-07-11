#include "condition_parser.h"
#include "database.h"
#include "date.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
using namespace std;
#include "test_runner.h"

string ParseEvent(istream &is) {
  std::string tmp;
  getline(is, tmp);
  while (*tmp.begin() == ' ')
    tmp.erase(tmp.begin());
  return tmp;
}

void TestAll();

int main() {
  // TestAll();

  Database db;

  for (string line; getline(cin, line);) {
    istringstream is(line);

    string command;
    is >> command;
    if (command == "Add") {
      const auto date = ParseDate(is);
      const auto event = ParseEvent(is);
      db.Add(date, event);
    } else if (command == "Print") {
      db.Print(cout);
    } else if (command == "Del") {
      auto condition = ParseCondition(is);
      auto predicate = [condition](const Date &date, const string &event) {
        return condition->Evaluate(date, event);
      };
      int count = db.RemoveIf(predicate);
      cout << "Removed " << count << " entries" << endl;
    } else if (command == "Find") {
      auto condition = ParseCondition(is);
      auto predicate = [condition](const Date &date, const string &event) {
        return condition->Evaluate(date, event);
      };

      const auto entries = db.FindIf(predicate);
      for (const auto &entry : entries) {
        cout << entry << endl;
      }
      cout << "Found " << entries.size() << " entries" << endl;
    } else if (command == "Last") {
      try {
        cout << db.Last(ParseDate(is)) << endl;
      } catch (invalid_argument &) {
        cout << "No entries" << endl;
      }
    } else if (command.empty()) {
      continue;
    } else {
      throw logic_error("Unknown command: " + command);
    }
  }

  return 0;
}

void TestParseEvent() {
  {
    istringstream is("event");
    AssertEqual(ParseEvent(is), "event", "Parse event without leading spaces");
  }
  {
    istringstream is("   sport event ");
    AssertEqual(ParseEvent(is), "sport event ",
                "Parse event with leading spaces");
  }
  {
    istringstream is("  first event  \n  second event");
    vector<string> events;
    events.push_back(ParseEvent(is));
    events.push_back(ParseEvent(is));
    AssertEqual(events, vector<string>{"first event  ", "second event"},
                "Parse multiple events");
  }
}

void TestParseCondition() {
  {
    istringstream is("date != 2017-11-18");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(root->Evaluate({2017, 1, 1}, ""), "Parse condition 1");
    Assert(!root->Evaluate({2017, 11, 18}, ""), "Parse condition 2");
  }
  {
    istringstream is(R"(event == "sport event")");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(root->Evaluate({2017, 1, 1}, "sport event"), "Parse condition 3");
    Assert(!root->Evaluate({2017, 1, 1}, "holiday"), "Parse condition 4");
  }
  {
    istringstream is("date >= 2017-01-01 AND date < 2017-07-01");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(root->Evaluate({2017, 1, 1}, ""), "Parse condition 5");
    Assert(root->Evaluate({2017, 3, 1}, ""), "Parse condition 6");
    Assert(root->Evaluate({2017, 6, 30}, ""), "Parse condition 7");
    Assert(!root->Evaluate({2017, 7, 1}, ""), "Parse condition 8");
    Assert(!root->Evaluate({2016, 12, 31}, ""), "Parse condition 9");
  }
  {
    istringstream is(R"(event != "sport event" AND event != "Wednesday")");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(root->Evaluate({2017, 1, 1}, "holiday"), "Parse condition 10");
    Assert(!root->Evaluate({2017, 1, 1}, "sport event"), "Parse condition 11");
    Assert(!root->Evaluate({2017, 1, 1}, "Wednesday"), "Parse condition 12");
  }
  {
    istringstream is(R"(event == "holiday AND date == 2017-11-18")");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(!root->Evaluate({2017, 11, 18}, "holiday"), "Parse condition 13");
    Assert(!root->Evaluate({2017, 11, 18}, "work day"), "Parse condition 14");
    Assert(root->Evaluate({1, 1, 1}, "holiday AND date == 2017-11-18"),
           "Parse condition 15");
  }
  {
    istringstream is(R"(((event == "holiday" AND date == 2017-01-01)))");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(root->Evaluate({2017, 1, 1}, "holiday"), "Parse condition 16");
    Assert(!root->Evaluate({2017, 1, 2}, "holiday"), "Parse condition 17");
  }
  {
    istringstream is(
        R"(date > 2017-01-01 AND (event == "holiday" OR date < 2017-07-01))");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(!root->Evaluate({2016, 1, 1}, "holiday"), "Parse condition 18");
    Assert(root->Evaluate({2017, 1, 2}, "holiday"), "Parse condition 19");
    Assert(root->Evaluate({2017, 1, 2}, "workday"), "Parse condition 20");
    Assert(!root->Evaluate({2018, 1, 2}, "workday"), "Parse condition 21");
  }
  {
    istringstream is(
        R"(date > 2017-01-01 AND event == "holiday" OR date < 2017-07-01)");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(root->Evaluate({2016, 1, 1}, "event"), "Parse condition 22");
    Assert(root->Evaluate({2017, 1, 2}, "holiday"), "Parse condition 23");
    Assert(root->Evaluate({2017, 1, 2}, "workday"), "Parse condition 24");
    Assert(!root->Evaluate({2018, 1, 2}, "workday"), "Parse condition 25");
  }
  {
    istringstream is(R"(((date == 2017-01-01 AND event == "holiday")))");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(root->Evaluate({2017, 1, 1}, "holiday"), "Parse condition 26");
    Assert(!root->Evaluate({2017, 1, 2}, "holiday"), "Parse condition 27");
  }
  {
    istringstream is(R"(((event == "2017-01-01" OR date > 2016-01-01)))");
    shared_ptr<Node> root = ParseCondition(is);
    Assert(root->Evaluate({1, 1, 1}, "2017-01-01"), "Parse condition 28");
    Assert(!root->Evaluate({2016, 1, 1}, "event"), "Parse condition 29");
    Assert(root->Evaluate({2016, 1, 2}, "event"), "Parse condition 30");
  }
}
void TestEmptyNode() {
  {
    EmptyNode en;
    Assert(en.Evaluate(Date{0, 1, 1}, "abc"), "EmptyNode 1");
    Assert(en.Evaluate(Date{2017, 11, 18}, "def"), "EmptyNode 2");
    Assert(en.Evaluate(Date{9999, 12, 31}, "ghi"), "EmptyNode 3");
  }
}
void TestDbAdd() {
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 7}, "xmas");
    ostringstream out;
    db.Print(out);
    AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n", out.str(),
                "straight ordering");
  }
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 1}, "holiday");
    ostringstream out;
    db.Print(out);
    AssertEqual("2017-01-01 new year\n2017-01-01 holiday\n", out.str(),
                "several in one day");
  }
  {
    Database db;
    db.Add({2017, 1, 7}, "xmas");
    db.Add({2017, 1, 1}, "new year");
    ostringstream out;
    db.Print(out);
    AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n", out.str(),
                "reverse ordering");
  }
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 1}, "xmas");
    db.Add({2017, 1, 1}, "new year");
    ostringstream out;
    db.Print(out);
    AssertEqual("2017-01-01 new year\n2017-01-01 xmas\n", out.str(),
                "uniq adding");
  }
}
string DoFind(Database &db, const string &str) {
  istringstream is(str);
  auto condition = ParseCondition(is);
  auto predicate = [condition](const Date &date, const string &event) {
    return condition->Evaluate(date, event);
  };
  const auto entries = db.FindIf(predicate);
  ostringstream os;
  for (const auto &entry : entries) {
    os << entry << endl;
  }
  os << entries.size();
  return os.str();
}
void TestDbFind() {
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 7}, "xmas");
    AssertEqual("2017-01-01 new year\n1", DoFind(db, "date == 2017-01-01"),
                "simple find by date");
    AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2",
                DoFind(db, "date < 2017-01-31"), "multiple find by date");
    AssertEqual("2017-01-01 new year\n1", DoFind(db, R"(event != "xmas")"),
                "multiple find by holiday");
  }
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 1}, "new year2");
    db.Add({2017, 1, 7}, "xmas");
    AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2",
                DoFind(db, R"(date == 2017-01-07 OR event == "new year")"),
                "complex find or");
    AssertEqual("2017-01-01 new year\n1",
                DoFind(db, R"(date == 2017-01-01 AND event == "new year")"),
                "complex find and");
    AssertEqual("0",
                DoFind(db, R"(date == 2017-01-09 AND event == "new year")"),
                "complex find and, nothing");
  }
}
int DoRemove(Database &db, const string &str) {
  istringstream is(str);
  auto condition = ParseCondition(is);
  auto predicate = [condition](const Date &date, const string &event) {
    return condition->Evaluate(date, event);
  };
  return db.RemoveIf(predicate);
}
void TestDbLast() {
  Database db;
  db.Add({2017, 1, 1}, "new year");
  db.Add({2017, 1, 7}, "xmas");
  {
    try {
      db.Last({2016, 12, 31});
      Assert(false, "last, found no entries");
    } catch (...) {
      Assert(true, "last, found no entries");
    }
  }
  {
    ostringstream os;
    os << db.Last({2017, 1, 2});
    AssertEqual("2017-01-01 new year", os.str(), "greater than date");
  }
  {
    ostringstream os;
    os << db.Last({2017, 1, 1});
    AssertEqual("2017-01-01 new year", os.str(), "eq to date");
  }
  {
    ostringstream os;
    os << db.Last({2017, 1, 10});
    AssertEqual("2017-01-07 xmas", os.str(), "greater than max date");
  }
}
void TestDbRemoveIf() {
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 7}, "xmas");
    AssertEqual(0, DoRemove(db, R"(event == "something")"), "Remove nothing");
    AssertEqual(1, DoRemove(db, R"(date == "2017-01-01")"), "Remove by date");
    ostringstream out;
    db.Print(out);
    AssertEqual("2017-01-07 xmas\n", out.str(), "Remove by date, left");
  }
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 7}, "xmas");
    AssertEqual(1, DoRemove(db, R"(event == "xmas")"), "Remove by event");
    ostringstream out;
    db.Print(out);
    AssertEqual("2017-01-01 new year\n", out.str(), "Remove by event, left");
  }
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 7}, "xmas");
    db.Add({2017, 1, 7}, "new year");
    AssertEqual(2, DoRemove(db, R"(event == "new year")"),
                "Multiple remove by event");
    ostringstream out;
    db.Print(out);
    AssertEqual("2017-01-07 xmas\n", out.str(),
                "Multiple remove by event, left");
  }
}
void TestInsertionOrder() {
  {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 7}, "xmas");
    db.Add({2017, 1, 7}, "party");
    db.Add({2017, 1, 7}, "pie");
    ostringstream out;
    db.Print(out);
    AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2017-01-07 "
                "party\n2017-01-07 pie\n",
                out.str(), "Remove by date, left");
  }
}
void TestAll() {
  TestRunner tr;
  tr.RunTest(TestParseEvent, "TestParseEvent");
  tr.RunTest(TestParseCondition, "TestParseCondition");
  tr.RunTest(TestEmptyNode, "Тест 2 из Coursera");
  tr.RunTest(TestDbAdd, "Тест 3(1) из Coursera");
  tr.RunTest(TestDbFind, "Тест 3(2) из Coursera");
  tr.RunTest(TestDbLast, "Тест 3(3) из Coursera");
  tr.RunTest(TestDbRemoveIf, "Тест 3(4) из Coursera");
  tr.RunTest(TestInsertionOrder, "Тест на порядок вывода");
  // tr.RunTest(TestsMyCustom, "Мои тесты");
  // tr.RunTest(TestDatabase, "Тест базы данных с GitHub");
}
