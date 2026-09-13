"""The command loop. Reads a command word, dispatches it, and repeats until the user quits."""

import commands
import store


def print_menu() -> None:
    """Print the application title and the command words."""
    print("Expense Tracker")
    print("Commands: add, list, filter, summary, edit, delete, quit")


def main() -> None:
    """Run the command loop until the user quits or the input ends."""
    store.seed_data()
    print_menu()

    while True:
        line = commands.read_line("> ")
        if line is None:
            print("Goodbye.")
            break
        if not line:
            continue  # an empty line asks again

        command = line.lower()
        if command == "add":
            commands.add()
        elif command == "list":
            commands.list_expenses()
        elif command == "filter":
            commands.filter_expenses()
        elif command == "summary":
            commands.summary()
        elif command == "edit":
            commands.edit()
        elif command == "delete":
            commands.delete_expense()
        elif command == "quit":
            print("Goodbye.")
            break
        else:
            print(f"Unknown command: {line}.")
            print_menu()


if __name__ == "__main__":
    main()
