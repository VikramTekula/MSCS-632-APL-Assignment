// Prompting, validation, error messages, and output formatting.
//
// Every prompt that rejects a value prints its message and asks the same question again, so a
// command is only ever abandoned by the user reaching the end of the input.

#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>

#include "store.h"

namespace commands {

// The title and the command words. Printed at startup and again after an unknown command.
void print_menu();

// Prompts with "> " and reads a command word. `command` comes back lowered, for matching, and
// `command_as_typed` keeps the user's own spelling for the unknown-command message. False at the
// end of the input.
bool read_command(std::string& command, std::string& command_as_typed);

void add(ExpenseStore& store);
void list_expenses(const ExpenseStore& store);
void filter_expenses(const ExpenseStore& store);
void summary(const ExpenseStore& store);
void edit(ExpenseStore& store);
void delete_expense(ExpenseStore& store);
void unknown(const std::string& command);

}  // namespace commands

#endif
