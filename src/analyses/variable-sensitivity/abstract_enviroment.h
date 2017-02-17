/*******************************************************************\

 Module: analyses variable-sensitivity

 Author: Thomas Kiley, thomas.kiley@diffblue.com

\*******************************************************************/
#ifndef CPROVER_ANALYSES_VARIABLE_SENSITIVITY_ABSTRACT_ENVIROMENT_H
#define CPROVER_ANALYSES_VARIABLE_SENSITIVITY_ABSTRACT_ENVIROMENT_H

#include <map>
#include <memory>

#include <util/std_expr.h>
#include <util/message.h>
#include <analyses/variable-sensitivity/abstract_object.h>

class abstract_environmentt
{
public:
  // These three are really the heart of the method
  virtual abstract_object_pointert eval(const exprt &expr) const;
  virtual bool assign(const exprt &expr, const abstract_object_pointert value);
  virtual bool assume(const exprt &expr);

  virtual abstract_object_pointert abstract_object_factory(
    const typet type, bool top = true) const;
  // For converting constants in the program
  // Maybe these two should be compacted to one call...
  virtual abstract_object_pointert abstract_object_factory(
    const typet type, const constant_exprt e) const;


  virtual bool merge(const abstract_environmentt &env);

  // This should be used as a default case / everything else has failed
  // The string is so that I can easily find and diagnose cases where this occurs
  virtual void havoc(const std::string &havoc_string);

  void make_top();
  void make_bottom();

  void output(
    std::ostream &out, const class ai_baset &ai, const namespacet &ns) const;

  // For the less havocy pointer we might need this, which can be applied
  // eagerly or lazily
  //virtual void havoc (std::string s, typet t);

protected:


 // We may need to break out more of these cases into these
 virtual abstract_object_pointert eval_logical(const exprt &e) const;

 // Hook for domain specific handling of operators
 virtual abstract_object_pointert eval_rest(const exprt &e) const;

 std::map<symbol_exprt, abstract_object_pointert> map;

};

#endif // CPROVER_ANALYSES_VARIABLE_SENSITIVITY_ABSTRACT_ENVIROMENT_H
