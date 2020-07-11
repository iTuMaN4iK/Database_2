#pragma once

#include "date.h"
#include "node.h"

#include <iostream>
#include <memory>
using namespace std;

shared_ptr<Node> ParseCondition(istream &is);

void TestParseCondition();