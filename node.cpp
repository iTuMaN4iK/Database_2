#include "node.h"
DateComparisonNode::DateComparisonNode(Comparison cmp, const Date &date)
    : cmp_(cmp), date_(date) {}
bool DateComparisonNode::Evaluate(const Date &date,
                                  const std::string &event) const {
  if (cmp_ == Comparison::Less) {
    return date < date_;
  } else if (cmp_ == Comparison::LessOrEqual) {
    return date <= date_;
  } else if (cmp_ == Comparison::Greater) {
    return date > date_;
  } else if (cmp_ == Comparison::GreaterOrEqual) {
    return date >= date_;
  } else if (cmp_ == Comparison::Equal) {
    return date == date_;
  } else if (cmp_ == Comparison::NotEqual) {
    return date != date_;
  }
  return false; // make compiler happy
}
EventComparisonNode::EventComparisonNode(Comparison cmp,
                                         const std::string &value)
    : cmp_(cmp), value_(value) {}
bool EventComparisonNode::Evaluate(const Date &date,
                                   const std::string &event) const {
  if (cmp_ == Comparison::Less) {
    return event < value_;
  } else if (cmp_ == Comparison::LessOrEqual) {
    return event <= value_;
  } else if (cmp_ == Comparison::Greater) {
    return event > value_;
  } else if (cmp_ == Comparison::GreaterOrEqual) {
    return event >= value_;
  } else if (cmp_ == Comparison::Equal) {
    return event == value_;
  } else if (cmp_ == Comparison::NotEqual) {
    return event != value_;
  }
}
bool EmptyNode::Evaluate(const Date &date, const std::string &event) const {
  return true;
};
LogicalOperationNode::LogicalOperationNode(LogicalOperation op,
                                           std::shared_ptr<Node> left,
                                           std::shared_ptr<Node> right)
    : op_(op), left_(left), right_(right) {}

bool LogicalOperationNode::Evaluate(const Date &date,
                                    const std::string &event) const {
  if (op_ == LogicalOperation::Or)
    return left_->Evaluate(date, event) || right_->Evaluate(date, event);
  return left_->Evaluate(date, event) && right_->Evaluate(date, event);
}
