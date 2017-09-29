//
// Created by thomas on 29/09/17.
//

#ifndef PROJECT_LOAD_JAVA_CLASS_H
#define PROJECT_LOAD_JAVA_CLASS_H

#include <util/symbol_table.h>

symbol_tablet load_java_class(
  const std::string java_class_name,
  const std::string &class_path);

#endif //PROJECT_LOAD_JAVA_CLASS_H
