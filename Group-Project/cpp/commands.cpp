#include "commands.h"

#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace commands {
namespace {

const char* const kAmountNotNumber = "Amount must be a number.";
const char* const kAmountNotPositive = "Amount must be greater than zero.";
const char* const kDateForm = "Date must be in YYYY-MM-DD form.";
const char* const kDateNotReal = "That date does not exist.";

std::string trim(const std::string& text) {
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Prints the prompt and reads one trimmed line. False at the end of the input, which abandons
// whatever command was being typed and leaves the data untouched.
bool read_line(const std::string& prompt, std::string& line) {
    std::cout << prompt << std::flush;
    std::string raw;
    if (!std::getline(std::cin, raw)) {
        std::cout << "\n";
        return false;
    }
    line = trim(raw);
    return true;
}

std::string category_error() {
    return "Category must be one of: " + category_names_joined() + ".";
}

std::string not_found_error(const std::string& id_text) {
    return "No expense with id " + id_text + ".";
}

void print_parse_error(ParseError error) {
    switch (error) {
        case ParseError::DateForm:          std::cout << kDateForm << "\n"; break;
        case ParseError::DateNotReal:       std::cout << kDateNotReal << "\n"; break;
        case ParseError::AmountForm:        std::cout << kAmountNotNumber << "\n"; break;
        case ParseError::AmountNotPositive: std::cout << kAmountNotPositive << "\n"; break;
        case ParseError::UnknownCategory:   std::cout << category_error() << "\n"; break;
        case ParseError::None:              break;
    }
}

// What a prompt that can be left blank came back with.
enum class Answer {
    Given,
    Blank,
    EndOfInput
};

// Asks until the answer parses or, when blank is allowed, until it is blank. The parser writes
// the value where its caller wants it; all this loop needs back is whether the text was accepted.
Answer prompt_until_valid(const std::string& prompt, bool blank_allowed,
                          const std::function<ParseError(const std::string&)>& parse) {
    std::string line;
    while (read_line(prompt, line)) {
        if (line.empty() && blank_allowed) {
            return Answer::Blank;
        }
        const ParseError error = parse(line);
        if (error == ParseError::None) {
            return Answer::Given;
        }
        print_parse_error(error);
    }
    return Answer::EndOfInput;
}

std::string table_header() {
    std::ostringstream row;
    row << std::right << std::setw(4) << "ID" << "  "
        << std::left << std::setw(10) << "DATE" << "  "
        << std::right << std::setw(9) << "AMOUNT" << "  "
        << std::left << std::setw(15) << "CATEGORY" << "  "
        << "DESCRIPTION";
    return row.str();
}

std::string table_row(const Expense& expense) {
    std::ostringstream row;
    row << std::right << std::setw(4) << expense.id << "  "
        << std::left << std::setw(10) << format_date(expense.date) << "  "
        << std::right << std::setw(9) << format_amount(expense.amount) << "  "
        << std::left << std::setw(15) << category_name(expense.category) << "  "
        << expense.description;
    // An empty description would otherwise leave the padding of the category column dangling.
    std::string line = row.str();
    return line.substr(0, line.find_last_not_of(' ') + 1);
}

void print_table(const std::vector<const Expense*>& expenses) {
    std::cout << table_header() << "\n";
    for (const Expense* expense : expenses) {
        std::cout << table_row(*expense) << "\n";
    }
}

// The whole of the text as an integer, or nothing when it is not one. std::stoi stops at the
// first character it cannot use, so a partial read counts as no read.
std::optional<int> read_int(const std::string& text) {
    try {
        std::size_t consumed = 0;
        const int value = std::stoi(text, &consumed);
        if (consumed == text.size()) {
            return value;
        }
    } catch (const std::invalid_argument&) {
        // falls through to nothing
    } catch (const std::out_of_range&) {
        // falls through to nothing
    }
    return std::nullopt;
}

// Asks for an id and hands back the expense carrying it, printing the not-found message itself and
// quoting the text as it was typed. Null at the end of the input too, since both leave the command.
const Expense* prompt_for_expense(const ExpenseStore& store) {
    std::string id_text;
    if (!read_line("Expense id: ", id_text)) {
        return nullptr;
    }
    const std::optional<int> id = read_int(id_text);
    const Expense* found = id.has_value() ? store.find(*id) : nullptr;
    if (found == nullptr) {
        std::cout << not_found_error(id_text) << "\n";
    }
    return found;
}

}  // namespace

void print_menu() {
    std::cout << "Expense Tracker\n"
              << "Commands: add, list, filter, summary, edit, delete, quit\n";
}

bool read_command(std::string& command, std::string& command_as_typed) {
    if (!read_line("> ", command_as_typed)) {
        return false;
    }
    command = to_lower(command_as_typed);
    return true;
}

void add(ExpenseStore& store) {
    Date date{};
    if (prompt_until_valid("Date (YYYY-MM-DD): ", false,
                           [&date](const std::string& line) { return parse_date(line, date); })
        != Answer::Given) {
        return;
    }
    double amount = 0.0;
    if (prompt_until_valid("Amount: ", false,
                           [&amount](const std::string& line) { return parse_amount(line, amount); })
        != Answer::Given) {
        return;
    }
    Category category = Category::Other;
    const std::string category_prompt = "Category (" + category_names_joined() + "): ";
    if (prompt_until_valid(category_prompt, false,
                           [&category](const std::string& line) {
                               return parse_category(line, category);
                           })
        != Answer::Given) {
        return;
    }
    std::string description;
    if (!read_line("Description: ", description)) {
        return;
    }

    // Nothing reaches the store until every field has passed.
    const int id = store.add(date, amount, category, description);
    std::cout << "Added expense " << id << ".\n";
}

void list_expenses(const ExpenseStore& store) {
    // No criterion at all, which is what the specification says a wholly blank filter comes to.
    const std::vector<const Expense*> expenses = store.filter(FilterCriteria{});
    if (expenses.empty()) {
        std::cout << "No expenses.\n";
        return;
    }
    print_table(expenses);
}

void filter_expenses(const ExpenseStore& store) {
    FilterCriteria criteria;

    Date start{};
    const Answer start_answer =
        prompt_until_valid("Start date (YYYY-MM-DD, blank for none): ", true,
                           [&start](const std::string& line) { return parse_date(line, start); });
    if (start_answer == Answer::EndOfInput) {
        return;
    }
    if (start_answer == Answer::Given) {
        criteria.start = start;
    }

    Date end{};
    const Answer end_answer =
        prompt_until_valid("End date (YYYY-MM-DD, blank for none): ", true,
                           [&end](const std::string& line) { return parse_date(line, end); });
    if (end_answer == Answer::EndOfInput) {
        return;
    }
    if (end_answer == Answer::Given) {
        criteria.end = end;
    }

    Category category = Category::Other;
    const Answer category_answer =
        prompt_until_valid("Category (blank for all): ", true,
                           [&category](const std::string& line) {
                               return parse_category(line, category);
                           });
    if (category_answer == Answer::EndOfInput) {
        return;
    }
    if (category_answer == Answer::Given) {
        criteria.category = category;
    }

    const std::vector<const Expense*> matches = store.filter(criteria);
    if (matches.empty()) {
        std::cout << "No expenses match.\n";
        return;
    }
    print_table(matches);
}

void summary(const ExpenseStore& store) {
    for (Category category : all_categories()) {
        std::cout << std::left << std::setw(15) << category_name(category)
                  << std::right << std::setw(10) << format_amount(store.category_total(category))
                  << "\n";
    }
    std::cout << "\n"
              << std::left << std::setw(15) << "Overall"
              << std::right << std::setw(10) << format_amount(store.overall_total()) << "\n";
}

void edit(ExpenseStore& store) {
    const Expense* current = prompt_for_expense(store);
    if (current == nullptr) {
        return;
    }

    // A copy, so an abandoned command leaves the stored expense as it was.
    Expense edited = *current;

    Date date{};
    const Answer date_answer =
        prompt_until_valid("Date [" + format_date(edited.date) + "] (blank to keep): ", true,
                           [&date](const std::string& line) { return parse_date(line, date); });
    if (date_answer == Answer::EndOfInput) {
        return;
    }
    if (date_answer == Answer::Given) {
        edited.date = date;
    }

    double amount = 0.0;
    const Answer amount_answer =
        prompt_until_valid("Amount [" + format_amount_plain(edited.amount) + "] (blank to keep): ",
                           true,
                           [&amount](const std::string& line) { return parse_amount(line, amount); });
    if (amount_answer == Answer::EndOfInput) {
        return;
    }
    if (amount_answer == Answer::Given) {
        edited.amount = amount;
    }

    Category category = Category::Other;
    const Answer category_answer =
        prompt_until_valid("Category [" + category_name(edited.category) + "] (blank to keep): ",
                           true,
                           [&category](const std::string& line) {
                               return parse_category(line, category);
                           });
    if (category_answer == Answer::EndOfInput) {
        return;
    }
    if (category_answer == Answer::Given) {
        edited.category = category;
    }

    std::string description;
    if (!read_line("Description [" + edited.description + "] (blank to keep): ", description)) {
        return;
    }
    if (!description.empty()) {
        edited.description = description;
    }

    store.update(edited);
    std::cout << "Updated expense " << edited.id << ".\n";
}

void delete_expense(ExpenseStore& store) {
    const Expense* target = prompt_for_expense(store);
    if (target == nullptr) {
        return;
    }
    // The id came out of the store, so the removal cannot miss, and the pointer dies with it.
    const int id = target->id;
    store.delete_expense(id);
    std::cout << "Deleted expense " << id << ".\n";
}

void unknown(const std::string& command) {
    std::cout << "Unknown command: " << command << ".\n";
    print_menu();
}

}  // namespace commands
