// The collection, the category index, and the operations over them.
//
// The vector owns the expenses by value: when the store is destroyed, they are destroyed with it.
// The category index holds raw pointers into that vector, which borrow and never own, so the index
// has to be rebuilt whenever the vector can have reallocated or shifted.

#ifndef STORE_H
#define STORE_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "model.h"

// What filter was asked for. An empty field means that criterion was skipped.
struct FilterCriteria {
    std::optional<Date> start;
    std::optional<Date> end;
    std::optional<Category> category;
};

class ExpenseStore {
public:
    // Starts with the twelve seeded expenses the specification fixes.
    ExpenseStore();

    // Appends an expense with the next unused id and returns that id. The id is the one field
    // the store owns, so it is the one field a caller cannot hand over.
    int add(const Date& date, double amount, Category category, const std::string& description);

    // The expense with this id, or nullptr. The pointer stays valid until the store changes.
    const Expense* find(int id) const;

    // Replaces the expense that carries the same id. The id itself is never changed.
    void update(const Expense& edited);

    // Removes the expense with this id. False when no expense carries it.
    bool delete_expense(int id);

    // The expenses matching every criterion given, in ascending id order.
    std::vector<const Expense*> filter(const FilterCriteria& criteria) const;

    // The total for one category, read from the index rather than by scanning every expense.
    // The index earns that on the read side and pays for it on the write side, where every change
    // rebuilds it whole. With twelve expenses that trade is not worth measuring; it is here because
    // the borrowing pointers are the point.
    double category_total(Category category) const;

    // The total over every category.
    double overall_total() const;

private:
    // Points the index at the expenses as they are now. Called after every change to the vector.
    void rebuild_category_index();

    std::vector<Expense> expenses_;                              // owns the records
    std::map<Category, std::vector<const Expense*>> index_;      // borrows them, never owns
    int next_id_;
};

#endif
