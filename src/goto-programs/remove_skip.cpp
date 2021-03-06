/*******************************************************************\

Module: Program Transformation

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// Program Transformation

#include "remove_skip.h"
#include "goto_model.h"

bool is_skip(const goto_programt &body, goto_programt::const_targett it)
{
  // we won't remove labelled statements
  // (think about error labels or the like)

  if(!it->labels.empty())
    return false;

  if(it->is_skip())
    return !it->code.get_bool(ID_explicit);

  if(it->is_goto())
  {
    if(it->guard.is_false())
      return true;

    goto_programt::const_targett next_it = it;
    next_it++;

    if(next_it == body.instructions.end())
      return false;

    // A branch to the next instruction is a skip
    // We also require the guard to be 'true'
    return it->guard.is_true() &&
           it->get_target()==next_it;
  }

  if(it->is_other())
  {
    if(it->code.is_nil())
      return true;

    const irep_idt &statement=it->code.get_statement();

    if(statement==ID_skip)
      return true;
    else if(statement==ID_expression)
    {
      const code_expressiont &code_expression=to_code_expression(it->code);
      const exprt &expr=code_expression.expression();
      if(expr.id()==ID_typecast &&
         expr.type().id()==ID_empty &&
         to_typecast_expr(expr).op().is_constant())
      {
        // something like (void)0
        return true;
      }
    }

    return false;
  }

  return false;
}

/// remove unnecessary skip statements
void remove_skip(goto_programt &goto_program)
{
  // This needs to be a fixed-point, as
  // removing a skip can turn a goto into a skip.
  std::size_t old_size;

  do
  {
    old_size=goto_program.instructions.size();

    // maps deleted instructions to their replacement
    typedef std::map<goto_programt::targett, goto_programt::targett>
      new_targetst;
    new_targetst new_targets;

    // remove skip statements

    for(goto_programt::instructionst::iterator
        it=goto_program.instructions.begin();
        it!=goto_program.instructions.end();)
    {
      goto_programt::targett old_target=it;

      // for collecting labels
      std::list<irep_idt> labels;

      while(is_skip(goto_program, it))
      {
        // don't remove the last skip statement,
        // it could be a target
        if(it==--goto_program.instructions.end())
          break;

        // save labels
        labels.splice(labels.end(), it->labels);
        it++;
      }

      goto_programt::targett new_target=it;

      // save labels
      it->labels.splice(it->labels.begin(), labels);

      if(new_target!=old_target)
      {
        for(; old_target!=new_target; ++old_target)
          new_targets[old_target]=new_target; // remember the old targets
      }
      else
        it++;
    }

    // adjust gotos

    Forall_goto_program_instructions(i_it, goto_program)
      if(i_it->is_goto() || i_it->is_start_thread() || i_it->is_catch())
      {
        for(goto_programt::instructiont::targetst::iterator
            t_it=i_it->targets.begin();
            t_it!=i_it->targets.end();
            t_it++)
        {
          new_targetst::const_iterator
            result=new_targets.find(*t_it);

          if(result!=new_targets.end())
            *t_it=result->second;
        }
      }

    // now delete the skips -- we do so after adjusting the
    // gotos to avoid dangling targets
    for(const auto &new_target : new_targets)
      goto_program.instructions.erase(new_target.first);

    // remove the last skip statement unless it's a target
    goto_program.compute_incoming_edges();

    if(
      !goto_program.instructions.empty() &&
      is_skip(goto_program, --goto_program.instructions.end()) &&
      !goto_program.instructions.back().is_target())
      goto_program.instructions.pop_back();
  }
  while(goto_program.instructions.size()<old_size);
}

/// remove unnecessary skip statements
void remove_skip(goto_functionst &goto_functions)
{
  Forall_goto_functions(f_it, goto_functions)
    remove_skip(f_it->second.body);

  // we may remove targets
  goto_functions.update();
}

void remove_skip(goto_modelt &goto_model)
{
  remove_skip(goto_model.goto_functions);
}

