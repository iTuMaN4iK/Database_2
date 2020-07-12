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
void TestCommandLast() {
  { // Add
    Database db;
    istringstream is, iss;
    is.str("Add 2017-01-01 New Year");

    string command;
    is >> command;

    const auto date = ParseDate(is);
    const auto event = ParseEvent(is);
    db.Add(date, event);

    ostringstream os;
    db.Print(os);

    vector<string> events;
    events.push_back(os.str());
    AssertEqual(events, vector<string>{"2017-01-01 New Year\n"},
                "Parse Add 01");
  }

  { // Add
    Database db;
    istringstream is, iss;
    is.str("\
Add 2017-01-01 Holiday\n\
Add 2017-03-08 Holiday\n\
Add 2017-1-1 New Year\n\
Add 2017-1-1 New Year\n");

    for (int i = 0; i < 4; i++) {
      string command;
      is >> command;

      const auto date = ParseDate(is);
      const auto event = ParseEvent(is);
      db.Add(date, event);
    }

    ostringstream os;
    db.Print(os);

    vector<string> events;
    events.push_back(os.str());
    AssertEqual(events, vector<string>{"\
2017-01-01 Holiday\n\
2017-01-01 New Year\n\
2017-03-08 Holiday\n"},
                "Parse multi-Add 02");
  }

  { // Last
    Database db;
    istringstream is, iss;
    is.str("\
Add 2017-01-01 New Year\n\
Add 2017-03-08 Holiday\n\
Last 2016-12-31\n\
Last 2017-01-01\n\
Last 2017-03-09\n");

    for (int i = 0; i < 2; i++) {
      string command;
      is >> command;

      const auto date = ParseDate(is);
      const auto event = ParseEvent(is);
      db.Add(date, event);
    }

    ostringstream os;
    for (int i = 0; i < 3; i++) {
      string command;
      is >> command;

      const auto date = ParseDate(is);
      try {
        os << db.Last(date) << endl;
      } catch (invalid_argument &) {
        os << "No entries" << endl;
      }
    }

    vector<string> events;
    events.push_back(os.str());
    AssertEqual(events, vector<string>{"\
No entries\n\
2017-01-01 New Year\n\
2017-03-08 Holiday\n"},
                "Parse Last 03");
  }

  {
    Database db;
    db.Add((Date){2017, 1, 1}, "first");
    db.Add((Date){2017, 1, 1}, "second");
    db.Add((Date){2017, 1, 1}, "third");
    db.Add((Date){2017, 1, 1}, "fourth");
    db.Add((Date){2017, 1, 1}, "five");
    AssertEqual(2, DoRemove(db, R"(event == "second" OR event == "fourth")"),
                "Remove several");
    // AssertEqual("2017-01-01 first\n2017-01-01 third\n2017-01-01 five\n",
    // DoPrint(db), "Check print after remove several- 3");
  }

  //---------------------------------------------------------------------------------------------------------
  { // всего понемногу
    Database db;
    istringstream is, iss, is5;

    //------------------------------------------------------
    //                     Add
    //------------------------------------------------------
    is.str("\
Add 2017-11-21 Tuesday\n\
Add 2017-11-20 Monday\n\
Add 2017-11-21 Weekly meeting\n");

    for (int i = 0; i < 3; i++) {
      string command;
      is >> command;

      const auto date = ParseDate(is);
      const auto event = ParseEvent(is);
      db.Add(date, event);
    }

    //------------------------------------------------------
    //                     find
    //------------------------------------------------------

    is.str("Find event != \"Weekly meeting\"\n");
    string command;
    is >> command;

    auto condition = ParseCondition(is);
    auto predicate = [condition](const Date &date, const string &event) {
      return condition->Evaluate(date, event);
    };

    auto entries3 = db.FindIf(predicate);
    string end_ = "Found 2 entries";
    entries3.push_back(end_);
    AssertEqual(entries3,
                vector<string>{"2017-11-20 Monday", "2017-11-21 Tuesday",
                               "Found 2 entries"},
                "Parse find 05");

    //------------------------------------------------------
    //                        Del
    //------------------------------------------------------
    is.str("Del date > 2017-11-20\n");
    string command2;
    is >> command2;

    auto condition2 = ParseCondition(is);
    auto predicate3 = [condition2](const Date &date, const string &event) {
      return condition2->Evaluate(date, event);
    };

    int count = db.RemoveIf(predicate3);
    string tmp3 = "Removed " + to_string(count) + " entries";

    AssertEqual(tmp3, "Removed 3 entries", "Parse Del 06");

    //------------------------------------------------------
    //                        Last
    //------------------------------------------------------
    is5.str("Last 2017-11-30\n");
    string command3;
    is5 >> command3;

    auto d_date = ParseDate(is5);
    string tmp4;
    try {
      tmp4 = db.Last(d_date);
    } catch (invalid_argument &) {
      tmp4 = "No entries";
    }

    AssertEqual(tmp4, "No entries", "Parse Last 07");
  }
  //---------------------------------------------------------------------------------------------------------
  {
    // Add 2018-03-08 preved
    // Add 2018-03-08 medved
    // Del event !="medved"
    // Add 2018-03-08 krasavcheg
    // Last 2018-03-08
    // Add 2018-03-08 medved
    // Last 2018-03-08

    Database db;
    istringstream is, iss, is5, is6, is7, is8;

    //------------------------------------------------------
    //                     Add
    //------------------------------------------------------
    is.str("\
Add 2018-03-08 preved\n\
Add 2018-03-08 medved\n");

    for (int i = 0; i < 2; i++) {
      string command;
      is >> command;

      const auto date = ParseDate(is);
      const auto event = ParseEvent(is);
      db.Add(date, event);
    }

    //------------------------------------------------------
    //                        Del
    //------------------------------------------------------
    is.str("Del event !=\"medved\"\n");
    string command2;
    is >> command2;

    auto condition2 = ParseCondition(is);
    auto predicate3 = [condition2](const Date &date, const string &event) {
      return condition2->Evaluate(date, event);
    };

    int count = db.RemoveIf(predicate3);
    string tmp3 = "Removed " + to_string(count) + " entries";

    AssertEqual(tmp3, "Removed 1 entries", "Parse Del 063");
    //------------------------------------------------------
    //                     Add
    //------------------------------------------------------
    is7.str("Add 2018-03-08 krasavcheg\n");

    for (int i = 0; i < 1; i++) {
      string command;
      is7 >> command;

      const auto date = ParseDate(is7);
      const auto event = ParseEvent(is7);
      db.Add(date, event);
    }

    //------------------------------------------------------
    //                        Last
    //------------------------------------------------------
    is5.str("Last 2018-03-08\n");
    string command3;
    is5 >> command3;

    auto d_date = ParseDate(is5);
    string tmp4;
    try {
      tmp4 = db.Last(d_date);
    } catch (invalid_argument &) {
      tmp4 = "No entries";
    }
    AssertEqual(tmp4, "2018-03-08 krasavcheg", "Parse Last 071");
    //------------------------------------------------------
    //                     Add
    //------------------------------------------------------
    is8.str("Add 2018-03-08 medved\n");

    for (int i = 0; i < 1; i++) {
      string command;
      is8 >> command;

      const auto date = ParseDate(is8);
      const auto event = ParseEvent(is8);
      db.Add(date, event);
    }
    //------------------------------------------------------
    //                        Last
    //------------------------------------------------------
    is6.str("Last 2018-03-08\n");
    string command4;
    is6 >> command4;

    auto d_date2 = ParseDate(is6);
    string tmp5;
    try {
      tmp5 = db.Last(d_date2);
    } catch (invalid_argument &) {
      tmp5 = "No entries";
    }

    AssertEqual(tmp5, "2018-03-08 krasavcheg", "Parse Last 073");
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
  tr.RunTest(TestCommandLast, "TestCommandLast");

  // tr.RunTest(TestsMyCustom, "Мои тесты");
  // tr.RunTest(TestDatabase, "Тест базы данных с GitHub");
}
