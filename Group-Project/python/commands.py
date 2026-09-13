"""Prompting, validation, error messages, and output formatting.

One function per command. Each prompts for its fields, rejects bad input with the message the
specification fixes, and asks the same question again rather than abandoning the command.
"""

from typing import Callable, List, Optional, Tuple
import model
import store

# The wording of every rejection. The specification fixes it, and the C++ half prints the same.
_PARSE_MESSAGES = {
    model.DATE_FORM: "Date must be in YYYY-MM-DD form.",
    model.DATE_NOT_REAL: "That date does not exist.",
    model.AMOUNT_FORM: "Amount must be a number.",
    model.AMOUNT_NOT_POSITIVE: "Amount must be greater than zero.",
    model.UNKNOWN_CATEGORY: "Category must be one of: " + ", ".join(model.CATEGORIES) + ".",
}

# What a prompt came back with, when a blank answer and the end of the input both have to be told
# apart from a value.
GIVEN = "given"
BLANK = "blank"
END_OF_INPUT = "end_of_input"


def read_line(prompt: str) -> Optional[str]:
    """Print the prompt and read one trimmed line. None at the end of the input.

    The end of the input abandons whatever command was being typed and leaves the data untouched.
    """
    try:
        return input(prompt).strip()
    except (EOFError, KeyboardInterrupt):
        print()
        return None


def prompt_until_valid(
    prompt: str,
    parse: Callable[[str], object],
    blank_allowed: bool,
) -> Tuple[str, str, object]:
    """Ask until the answer parses or, when blank is allowed, until it is blank.

    Returns what the prompt came back with, the text as it was typed, and the parsed value.
    """
    while True:
        line = read_line(prompt)
        if line is None:
            return END_OF_INPUT, "", None
        if not line and blank_allowed:
            return BLANK, "", None
        try:
            return GIVEN, line, parse(line)
        # ParseError is a ValueError, so this one clause covers every rejection a parser can raise.
        except model.ParseError as error:
            print(_PARSE_MESSAGES[error.reason])


def find_expense(id_text: str) -> Optional[dict]:
    """Look up an expense by its typed id, printing the not-found message when there is none."""
    try:
        expense_id = int(id_text)
    except ValueError:
        print(f"No expense with id {id_text}.")
        return None

    expense = store.get_expense(expense_id)
    if expense is None:
        print(f"No expense with id {id_text}.")
    return expense


def format_table(expense_list: List[dict]) -> str:
    """Format a list of expense dicts into the standard table output."""
    header = f"{'ID':>4}  {'DATE':<10}  {'AMOUNT':>9}  {'CATEGORY':<15}  {'DESCRIPTION'}"
    rows = [header]
    for exp in expense_list:
        amount_str = f"${exp['amount']:.2f}"
        row = (
            f"{exp['id']:>4}  "
            f"{str(exp['date']):<10}  "
            f"{amount_str:>9}  "
            f"{exp['category']:<15}  "
            f"{exp['description']}"
        )
        # An empty description would otherwise leave the padding of the category column dangling.
        rows.append(row.rstrip())
    return "\n".join(rows)


def format_summary(category_totals: List[tuple], overall: float) -> str:
    """Format category totals and overall total into the summary block."""
    lines = []
    for category, total in category_totals:
        amount_str = f"${total:.2f}"
        lines.append(f"{category:<15}{amount_str:>10}")
    lines.append("")
    overall_str = f"${overall:.2f}"
    lines.append(f"{'Overall':<15}{overall_str:>10}")
    return "\n".join(lines)


def add() -> None:
    """Prompt for the four fields in order, validating each, and record a new expense."""
    answer, date_text, _ = prompt_until_valid("Date (YYYY-MM-DD): ", model.parse_date, False)
    if answer == END_OF_INPUT:
        return

    answer, amount_text, _ = prompt_until_valid("Amount: ", model.parse_amount, False)
    if answer == END_OF_INPUT:
        return

    category_prompt = f"Category ({', '.join(model.CATEGORIES)}): "
    answer, category_text, _ = prompt_until_valid(category_prompt, model.parse_category, False)
    if answer == END_OF_INPUT:
        return

    description = read_line("Description: ")
    if description is None:
        return

    # The record is built in this one call, from the four strings exactly as they were typed.
    record = model.parse_expense_fields(date_text, amount_text, category_text, description)
    print(f"Added expense {store.add_expense(record)}.")


def list_expenses() -> None:
    """Print every expense sorted by id ascending."""
    items = store.list_expenses()
    if not items:
        print("No expenses.")
    else:
        print(format_table(items))


def filter_expenses() -> None:
    """Prompt for a date range and a category, any of which may be blank, and print the matches."""
    answer, _, start_date = prompt_until_valid(
        "Start date (YYYY-MM-DD, blank for none): ", model.parse_date, True
    )
    if answer == END_OF_INPUT:
        return

    answer, _, end_date = prompt_until_valid(
        "End date (YYYY-MM-DD, blank for none): ", model.parse_date, True
    )
    if answer == END_OF_INPUT:
        return

    answer, _, category = prompt_until_valid(
        "Category (blank for all): ", model.parse_category, True
    )
    if answer == END_OF_INPUT:
        return

    matches = store.filter_expenses(start_date=start_date, end_date=end_date, category=category)
    if not matches:
        print("No expenses match.")
    else:
        print(format_table(matches))


def summary() -> None:
    """Print a total for each category and an overall total."""
    category_totals, overall = store.get_summary()
    print(format_summary(category_totals, overall))


def edit() -> None:
    """Prompt for an id and each editable field, where a blank answer keeps the stored value."""
    id_text = read_line("Expense id: ")
    if id_text is None:
        return

    expense = find_expense(id_text)
    if expense is None:
        return

    answer, _, date = prompt_until_valid(
        f"Date [{expense['date']}] (blank to keep): ", model.parse_date, True
    )
    if answer == END_OF_INPUT:
        return
    if answer == BLANK:
        date = expense["date"]

    answer, _, amount = prompt_until_valid(
        f"Amount [{expense['amount']:.2f}] (blank to keep): ", model.parse_amount, True
    )
    if answer == END_OF_INPUT:
        return
    if answer == BLANK:
        amount = expense["amount"]

    answer, _, category = prompt_until_valid(
        f"Category [{expense['category']}] (blank to keep): ", model.parse_category, True
    )
    if answer == END_OF_INPUT:
        return
    if answer == BLANK:
        category = expense["category"]

    description = read_line(f"Description [{expense['description']}] (blank to keep): ")
    if description is None:
        return
    if not description:
        description = expense["description"]

    # A kept answer carries the stored value straight over, so keeping an amount cannot round it.
    updated = {
        "date": date,
        "amount": amount,
        "category": category,
        "description": description,
    }
    expense_id = expense["id"]
    store.update_expense(expense_id, updated)
    print(f"Updated expense {expense_id}.")


def delete_expense() -> None:
    """Prompt for an id and remove that expense."""
    id_text = read_line("Expense id: ")
    if id_text is None:
        return

    expense = find_expense(id_text)
    if expense is None:
        return

    expense_id = expense["id"]
    store.delete_expense(expense_id)
    print(f"Deleted expense {expense_id}.")
