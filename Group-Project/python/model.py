"""The expense record, the category, the date, and conversion between text and those types.

An expense is a dict with the keys id, date, amount, category and description. Categories are the
five names the specification fixes. Date work goes through datetime, and every conversion from
typed text happens here rather than at the prompt.
"""

import datetime
import math
import re

CATEGORIES = [
    "Food",
    "Transport",
    "Utilities",
    "Entertainment",
    "Other",
]

_CATEGORY_LOOKUP = {cat.lower(): cat for cat in CATEGORIES}

# Digits with at most one decimal point, and an optional sign. Nothing else is a sum of money.
_AMOUNT_SHAPE = re.compile(r"^[+-]?([0-9]+\.?[0-9]*|\.[0-9]+)$")

# Why a piece of typed text could not become a value. The two date failures stay apart because the
# specification gives them different messages. The wording itself belongs to commands.
DATE_FORM = "date_form"
DATE_NOT_REAL = "date_not_real"
AMOUNT_FORM = "amount_form"
AMOUNT_NOT_POSITIVE = "amount_not_positive"
UNKNOWN_CATEGORY = "unknown_category"


class ParseError(ValueError):
    """Typed text that could not become a value. A ValueError, so one except clause covers it."""

    def __init__(self, reason: str):
        super().__init__(reason)
        self.reason = reason


def parse_date(text: str) -> datetime.date:
    """Read YYYY-MM-DD, telling a wrong shape apart from a day that does not exist.

    The shape check covers the layout and the month only. A day the month never had, such as
    2026-02-31 or 2026-01-32, has the right shape and is left for strptime to turn away, which is
    what the C++ half does with its own calendar check.
    """
    if not re.match(r"^\d{4}-\d{2}-\d{2}$", text):
        raise ParseError(DATE_FORM)
    if not 1 <= int(text[5:7]) <= 12:
        raise ParseError(DATE_FORM)
    try:
        return datetime.datetime.strptime(text, "%Y-%m-%d").date()
    except ValueError:
        raise ParseError(DATE_NOT_REAL)


def parse_amount(text: str) -> float:
    """Read an amount and reject anything that is not wholly a positive number."""
    # float() also reads "nan", "inf" and "1e3", none of which is a sum of money, so the shape is
    # checked before the conversion rather than after it.
    if not _AMOUNT_SHAPE.match(text):
        raise ParseError(AMOUNT_FORM)
    try:
        amount = float(text)
    except ValueError:
        raise ParseError(AMOUNT_FORM)
    # A number too long for a float still reads, as inf.
    if not math.isfinite(amount):
        raise ParseError(AMOUNT_FORM)
    if amount <= 0:
        raise ParseError(AMOUNT_NOT_POSITIVE)
    return amount


def parse_category(text: str) -> str:
    """Read a category name case-insensitively and return it in its canonical spelling."""
    cleaned = text.strip().lower()
    if cleaned not in _CATEGORY_LOOKUP:
        raise ParseError(UNKNOWN_CATEGORY)
    return _CATEGORY_LOOKUP[cleaned]


def parse_expense_fields(
    date_str: str,
    amount_str: str,
    category_str: str,
    description_str: str,
) -> dict:
    """Turn the four typed strings into an expense dict.

    The record takes its shape here, at run time, and is declared nowhere. The store adds the id,
    which is the one field that never comes from typed text.
    """
    return {
        "date": parse_date(date_str),
        "amount": parse_amount(amount_str),
        "category": parse_category(category_str),
        "description": description_str,
    }
