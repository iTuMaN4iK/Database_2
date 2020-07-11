#pragma once
#include "date.h"
#include <memory>
enum class Comparison {
  Less,
  LessOrEqual,
  Greater,
  GreaterOrEqual,
  Equal,
  NotEqual
};
enum class LogicalOperation { Or, And };
class Node {
public:
  virtual bool Evaluate(const Date &date, const std::string &event) const = 0;
};

class DateComparisonNode : public Node {
public:
  DateComparisonNode(Comparison cmp, const Date &date);
  bool Evaluate(const Date &date, const std::string &event) const override;

private:
  const Comparison cmp_;
  const Date date_;
};

class EventComparisonNode : public Node {
public:
  EventComparisonNode(Comparison cmp, const std::string &value);
  bool Evaluate(const Date &date, const std::string &event) const override;

private:
  const Comparison cmp_;
  const std::string value_;
};

class EmptyNode : public Node {
public:
  EmptyNode() = default;
  bool Evaluate(const Date &date, const std::string &event) const override;
};

class LogicalOperationNode : public Node {
public:
  LogicalOperationNode(LogicalOperation op, std::shared_ptr<Node> left,
                       std::shared_ptr<Node> right);
  bool Evaluate(const Date &date, const std::string &event) const override;

private:
  const LogicalOperation op_;
  const std::shared_ptr<Node> left_, right_;
};