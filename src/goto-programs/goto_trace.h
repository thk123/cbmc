/*******************************************************************\

Module: Traces of GOTO Programs

Author: Daniel Kroening

Date: July 2005

\*******************************************************************/

/// \file
/// Traces of GOTO Programs

#ifndef CPROVER_GOTO_PROGRAMS_GOTO_TRACE_H
#define CPROVER_GOTO_PROGRAMS_GOTO_TRACE_H

#include <iosfwd>
#include <vector>

#include <util/message.h>
#include <util/namespace.h>
#include <util/options.h>
#include <util/ssa_expr.h>

#include <goto-programs/goto_program.h>

/// A single step of a \ref goto_tracet
/// \ingroup gr_goto_symex
class goto_trace_stept
{
public:
  /// The index of the step within the trace, the first step is index 0??
  std::size_t step_nr;

  bool is_assignment() const      { return type==typet::ASSIGNMENT; }
  bool is_assume() const          { return type==typet::ASSUME; }
  bool is_assert() const          { return type==typet::ASSERT; }
  bool is_goto() const            { return type==typet::GOTO; }
  bool is_constraint() const      { return type==typet::CONSTRAINT; }
  bool is_function_call() const   { return type==typet::FUNCTION_CALL; }
  bool is_function_return() const { return type==typet::FUNCTION_RETURN; }
  bool is_location() const        { return type==typet::LOCATION; }
  bool is_output() const          { return type==typet::OUTPUT; }
  bool is_input() const           { return type==typet::INPUT; }
  bool is_decl() const            { return type==typet::DECL; }
  bool is_dead() const            { return type==typet::DEAD; }
  bool is_shared_read() const     { return type==typet::SHARED_READ; }
  bool is_shared_write() const    { return type==typet::SHARED_WRITE; }
  bool is_spawn() const           { return type==typet::SPAWN; }
  bool is_memory_barrier() const  { return type==typet::MEMORY_BARRIER; }
  bool is_atomic_begin() const    { return type==typet::ATOMIC_BEGIN; }
  bool is_atomic_end() const      { return type==typet::ATOMIC_END; }


  enum class typet
  {
    NONE,
    ASSIGNMENT,
    ASSUME,
    ASSERT,
    GOTO,
    LOCATION,
    INPUT,
    OUTPUT,
    DECL,
    DEAD,
    FUNCTION_CALL,
    FUNCTION_RETURN,
    CONSTRAINT,
    SHARED_READ,
    SHARED_WRITE,
    SPAWN,
    MEMORY_BARRIER,
    ATOMIC_BEGIN,
    ATOMIC_END
  };

  /// The kind of statement that this step relates to
  typet type;

  /// we may choose to hide a step
  /// TODO: expand to what steps are hidden
  bool hidden;

  /// this is related to an internal expression
  /// TODO: expand to what steps are internal
  bool internal;

  /// we categorize
  /// TODO: Literally what?
  enum class assignment_typet { STATE, ACTUAL_PARAMETER };
  assignment_typet assignment_type;

  /// The function that created this step
  /// For FUNCTION_CALL steps, the function_id is the caller
  /// For FUNCTION_RETURN steps the function_id is the callee
  /// For example:
  /// 0: foo(); // function_id = CPROVER_START
  /// 1: return; // function_id = foo
  /// 2: return // function_id = CPROVER_START
  irep_idt function_id;

  /// A iterator into the GOTO statement that this trace step corresponds to
  goto_programt::const_targett pc;

  /// this transition done by given thread number
  unsigned thread_nr;

  /// For \ref typet::ASSUME, \ref typet::ASSERT, \ref typet::GOTO this
  /// contains the evaluated condition value
  bool cond_value;
  /// See \ref cond_value
  exprt cond_expr;

  /// For \ref typet::ASSERT steps, this is the id of the associated goal
  /// TODO: identify appropriate reference
  irep_idt property_id;
  /// For \ref typet::ASSERT steps, this is the comment of the associated goal
  /// TODO: identify appropriate reference
  std::string comment;

  /// The full, original lhs expression, after dereferencing
  /// TODO: are these only applicable in ASSIGNMENT
  exprt full_lhs;

  /// The l-value object being assigned (i.e. after walking up member and index access
  /// and skipping type casts).
  /// TODO: are these only applicable in ASSIGNMENT
  optionalt<symbol_exprt> get_lhs_object() const;

  /// A constant with the new value of the lhs, that is, the rhs.
  /// TODO: are these only applicable in ASSIGNMENT
  exprt full_lhs_value;

  /// for INPUT/OUTPUT
  /// TODO: find out what these strings actually are
  irep_idt format_string, io_id;
  typedef std::list<exprt> io_argst;
  io_argst io_args;
  bool formatted;

  /// For \ref typet::FUNCTION_CALL stores the function that has been called
  /// (in the example in \ref function_id), step 0, this would have the value foo)
  irep_idt called_function;

  /// For \ref typet::FUNCTION_CALL, stores the evaluated?? values for the function
  /// arguments
  std::vector<exprt> function_arguments;

  /// Outputs the trace step in ASCII to a given stream
  /// TODO: document what this looks like
  void output(
    const class namespacet &ns,
    std::ostream &out) const;

  goto_trace_stept():
    step_nr(0),
    type(typet::NONE),
    hidden(false),
    internal(false),
    assignment_type(assignment_typet::STATE),
    thread_nr(0),
    cond_value(false),
    formatted(false)
  {
    full_lhs.make_nil();
    full_lhs_value.make_nil();
    cond_expr.make_nil();
  }
};

/// Represents a trace that has come out of the solver
/// TODO: is this the same class that is representing a trace
/// before and after SSA conversion?
class goto_tracet
{
public:
  typedef std::list<goto_trace_stept> stepst;
  stepst steps;

  /// The language of the trace, TODO: find appropriate reference
  irep_idt mode;

  /// TODO: find the purpose of this method
  void clear()
  {
    mode.clear();
    steps.clear();
  }

  /// Outputs the trace in ASCII to a given stream
  /// TODO: document the format
  /// See \ref goto_trace_stept::output
  void output(
    const class namespacet &ns,
    std::ostream &out) const;

  void swap(goto_tracet &other)
  {
    other.steps.swap(steps);
    other.mode.swap(mode);
  }

  /// Append a step to the trace.
  void add_step(const goto_trace_stept &step)
  {
    steps.push_back(step);
  }

  /// Retrieves the final step in the trace for manipulation
  /// (used to fill a trace from code, hence non-const)
  goto_trace_stept &get_last_step()
  {
    return steps.back();
  }

  /// Returns the property IDs of all assertions in the trace
  std::vector<irep_idt> get_all_property_ids() const;

  /// delete all steps after (not including) s
  void trim_after(stepst::iterator s)
  {
    PRECONDITION(s != steps.end());
    steps.erase(++s, steps.end());
  }
};


/// Configuration options for use in outputing the trace
/// See \ref show_goto_trace
struct trace_optionst
{
  /// Should the \ref goto_trace_stept::full_lhs include a full
  /// structured irep dump of the lhs.
  /// TODO: find out if this is only for json
  bool json_full_lhs;
  /// TODO: find out what this controls
  bool hex_representation;
  /// TODO: find out what this controls
  bool base_prefix;
  /// TODO: find out what this controls
  bool show_function_calls;
  /// TODO: find out what this controls
  bool show_code;
  /// TODO: find out what this controls
  bool compact_trace;
  /// TODO: find out what this controls
  bool stack_trace;

  /// The default trace printing options. See \ref trace_optionst::trace_optionst()
  /// for what these values are.
  static const trace_optionst default_options;

  /// Configure the trace printing options from the command line.
  explicit trace_optionst(const optionst &options)
  {
    json_full_lhs = options.get_bool_option("trace-json-extended");
    hex_representation = options.get_bool_option("trace-hex");
    base_prefix = hex_representation;
    show_function_calls = options.get_bool_option("trace-show-function-calls");
    show_code = options.get_bool_option("trace-show-code");
    compact_trace = options.get_bool_option("compact-trace");
    stack_trace = options.get_bool_option("stack-trace");
  };

private:
  trace_optionst()
  {
    json_full_lhs = false;
    hex_representation = false;
    base_prefix = false;
    show_function_calls = false;
    show_code = false;
    compact_trace = false;
    stack_trace = false;
  };
};

// NOLINTNEXTLINE
/// See \ref show_goto_trace(messaget::mstreamt &, const namespacet &, const goto_tracet &, const trace_optionst &
void show_goto_trace(
  messaget::mstreamt &out,
  const namespacet &,
  const goto_tracet &);

/// Output the goto trace to the provided stream
void show_goto_trace(
  messaget::mstreamt &out,
  const namespacet &,
  const goto_tracet &,
  const trace_optionst &);

/// TODO: figure out what this does
void trace_value(
  messaget::mstreamt &out,
  const namespacet &,
  const optionalt<symbol_exprt> &lhs_object,
  const exprt &full_lhs,
  const exprt &value,
  const trace_optionst &);

#define OPT_GOTO_TRACE                                                         \
  "(trace-json-extended)"                                                      \
  "(trace-show-function-calls)"                                                \
  "(trace-show-code)"                                                          \
  "(trace-hex)"                                                                \
  "(compact-trace)"                                                            \
  "(stack-trace)"

#define HELP_GOTO_TRACE                                                        \
  " --trace-json-extended        add rawLhs property to trace\n"               \
  " --trace-show-function-calls  show function calls in plain trace\n"         \
  " --trace-show-code            show original code in plain trace\n"          \
  " --trace-hex                  represent plain trace values in hex\n"        \
  " --compact-trace              give a compact trace\n"                       \
  " --stack-trace                give a stack trace only\n"

#define PARSE_OPTIONS_GOTO_TRACE(cmdline, options)                             \
  if(cmdline.isset("trace-json-extended"))                                     \
    options.set_option("trace-json-extended", true);                           \
  if(cmdline.isset("trace-show-function-calls"))                               \
    options.set_option("trace-show-function-calls", true);                     \
  if(cmdline.isset("trace-show-code"))                                         \
    options.set_option("trace-show-code", true);                               \
  if(cmdline.isset("trace-hex"))                                               \
    options.set_option("trace-hex", true);                                     \
  if(cmdline.isset("compact-trace"))                                           \
    options.set_option("compact-trace", true);                                 \
  if(cmdline.isset("stack-trace"))                                             \
    options.set_option("stack-trace", true);

#endif // CPROVER_GOTO_PROGRAMS_GOTO_TRACE_H
