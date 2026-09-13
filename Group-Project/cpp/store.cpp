#include "store.h"

#include <algorithm>

namespace {

// The twelve expenses every run starts from. Hard-coded so the two implementations can be run
// side by side and compared directly.
const std::vector<Expense>& seed_expenses() {
    static const std::vector<Expense> seeded = {
        { 1, {2026, 1,  5},  12.50, Category::Food,          "Coffee and pastry"},
        { 2, {2026, 1,  8},  45.00, Category::Transport,     "Monthly transit top-up"},
        { 3, {2026, 1, 14}, 120.75, Category::Utilities,     "Electricity bill"},
        { 4, {2026, 1, 22},  18.99, Category::Entertainment, "Cinema ticket"},
        { 5, {2026, 1, 28},  32.40, Category::Food,          "Grocery run"},
        { 6, {2026, 2,  3},   9.25, Category::Other,         "Stationery"},
        { 7, {2026, 2, 11},  62.10, Category::Food,          "Dinner with friends"},
        { 8, {2026, 2, 17},  28.00, Category::Transport,     "Airport taxi"},
        { 9, {2026, 2, 25},  95.40, Category::Utilities,     "Internet and phone"},
        {10, {2026, 3,  4},  15.00, Category::Entertainment, "Streaming subscription"},
        {11, {2026, 3, 12},  41.85, Category::Food,          "Weekly groceries"},
        {12, {2026, 3, 20},  22.30, Category::Other,         "Pharmacy"}
    };
    return seeded;
}

// The one test that says which expense is wanted. Every lookup below goes through it.
auto has_id(int id) {
    return [id](const Expense& expense) { return expense.id == id; };
}

}  // namespace

ExpenseStore::ExpenseStore() : expenses_(seed_expenses()), next_id_(13) {
    rebuild_category_index();
}

void ExpenseStore::rebuild_category_index() {
    index_.clear();
    // Every category gets an entry, so a category with no expenses still reports a total of zero.
    for (Category category : all_categories()) {
        index_[category];
    }
    for (const Expense& expense : expenses_) {
        index_[expense.category].push_back(&expense);
    }
}

int ExpenseStore::add(const Date& date, double amount, Category category,
                      const std::string& description) {
    const int id = next_id_++;
    expenses_.push_back(Expense{id, date, amount, category, description});
    // push_back can reallocate, which would leave every pointer in the index dangling.
    rebuild_category_index();
    return id;
}

const Expense* ExpenseStore::find(int id) const {
    const auto position = std::find_if(expenses_.begin(), expenses_.end(), has_id(id));
    return position == expenses_.end() ? nullptr : &*position;
}

void ExpenseStore::update(const Expense& edited) {
    const auto position = std::find_if(expenses_.begin(), expenses_.end(), has_id(edited.id));
    if (position == expenses_.end()) {
        return;
    }
    *position = edited;
    // The vector has not moved, but an edited category puts the expense under another key.
    rebuild_category_index();
}

bool ExpenseStore::delete_expense(int id) {
    const auto position = std::find_if(expenses_.begin(), expenses_.end(), has_id(id));
    if (position == expenses_.end()) {
        return false;
    }
    expenses_.erase(position);
    // erase shifts everything after the hole, so the index points at the wrong expenses now.
    rebuild_category_index();
    return true;
}

std::vector<const Expense*> ExpenseStore::filter(const FilterCriteria& criteria) const {
    std::vector<const Expense*> matches;

    // With a category given, the index already holds that group, so nothing else is looked at.
    const std::vector<const Expense*>* candidates = nullptr;
    std::vector<const Expense*> everything;
    if (criteria.category.has_value()) {
        candidates = &index_.at(*criteria.category);
    } else {
        everything.reserve(expenses_.size());
        for (const Expense& expense : expenses_) {
            everything.push_back(&expense);
        }
        candidates = &everything;
    }

    for (const Expense* expense : *candidates) {
        if (criteria.start.has_value() && !date_on_or_before(*criteria.start, expense->date)) {
            continue;
        }
        if (criteria.end.has_value() && !date_on_or_before(expense->date, *criteria.end)) {
            continue;
        }
        matches.push_back(expense);
    }

    std::sort(matches.begin(), matches.end(),
              [](const Expense* left, const Expense* right) { return left->id < right->id; });
    return matches;
}

double ExpenseStore::category_total(Category category) const {
    double total = 0.0;
    for (const Expense* expense : index_.at(category)) {
        total += expense->amount;
    }
    return total;
}

double ExpenseStore::overall_total() const {
    double total = 0.0;
    for (Category category : all_categories()) {
        total += category_total(category);
    }
    return total;
}
