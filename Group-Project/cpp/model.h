// The expense record, the category, the date, and conversion between text and those types.
//
// C++17 has no calendar in its standard library, so a date is three integers that this unit
// validates by hand, including the leap-year rule. A category is an enum class, so the compiler
// rejects an invalid one before the program runs.

#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

struct Date {
    int year;
    int month;
    int day;
};

// The five categories of the specification, in the fixed order summary prints them.
enum class Category {
    Food,
    Transport,
    Utilities,
    Entertainment,
    Other
};

struct Expense {
    int id;
    Date date;
    double amount;
    Category category;
    std::string description;
};

// The five categories in their fixed order, for iteration.
const std::vector<Category>& all_categories();

// The text in lower case, for the case-insensitive matching the specification asks for.
std::string to_lower(const std::string& text);

// The display name of a category, such as "Food".
std::string category_name(Category category);

// The five display names joined for the category error message and the prompt.
std::string category_names_joined();

// Why a piece of typed text could not become a value. The two date failures stay apart because
// the specification gives them different messages.
enum class ParseError {
    None,
    DateForm,       // not YYYY-MM-DD
    DateNotReal,    // right shape, no such day, such as 2026-02-31
    AmountForm,     // not a number
    AmountNotPositive,
    UnknownCategory
};

// Reads a category name, case-insensitively. Leaves category untouched when the name is unknown.
ParseError parse_category(const std::string& text, Category& category);

// Reads YYYY-MM-DD. Tells a wrong shape apart from a day that does not exist.
ParseError parse_date(const std::string& text, Date& date);

// Reads an amount with std::stod and rejects anything that is not wholly a positive number.
ParseError parse_amount(const std::string& text, double& amount);

// True when left is the earlier of the two dates, or the same day.
bool date_on_or_before(const Date& left, const Date& right);

// YYYY-MM-DD, zero padded.
std::string format_date(const Date& date);

// Exactly two decimals and no currency sign, such as 12.50. This is the shape an edit prompt
// shows inside its square brackets.
std::string format_amount_plain(double amount);

// A dollar sign and exactly two decimals, such as $12.50.
std::string format_amount(double amount);

#endif
