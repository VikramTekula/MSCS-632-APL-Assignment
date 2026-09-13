// The command loop. Reads a command word, dispatches it, and repeats until the user quits.
//
// The store lives here, on the stack. Every expense it holds is destroyed when this function
// returns, which is the whole of the memory management this program needs.

#include <iostream>
#include <string>

#include "commands.h"
#include "store.h"

int main() {
    ExpenseStore store;
    commands::print_menu();

    std::string command;
    std::string command_as_typed;
    while (commands::read_command(command, command_as_typed)) {
        if (command.empty()) {
            continue;  // an empty line asks again
        }
        if (command == "add") {
            commands::add(store);
        } else if (command == "list") {
            commands::list_expenses(store);
        } else if (command == "filter") {
            commands::filter_expenses(store);
        } else if (command == "summary") {
            commands::summary(store);
        } else if (command == "edit") {
            commands::edit(store);
        } else if (command == "delete") {
            commands::delete_expense(store);
        } else if (command == "quit") {
            break;
        } else {
            commands::unknown(command_as_typed);
        }
    }

    std::cout << "Goodbye.\n";
    return 0;
}
