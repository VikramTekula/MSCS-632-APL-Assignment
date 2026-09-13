# Expense Tracker

MSCS-632 group project, deliverable 1. One terminal application, implemented in Python and C++.

The application records expenses, lists them, filters them by date range and category, and totals them by category and overall. 
Both implementations offer the same feature sets over the same record structure. The data lives in memory for the length of one run.

The purpose of building the same application in 2 different languages is to compare how the two languages handle the same job, in data structures, memory management, and error handling. 
The comparison is written up separately in the project report.

## Layout and ownership

| Directory | Holds | Owned by |
| --- | --- | --- |
| `python/` | the Python implementation | Sabin Ranabhat |
| `cpp/` | the C++ implementation | Nguyen Vo |


Both implementations use the same four units, which is what lets the report put the two implementations side by side.

| Unit | Holds |
| --- | --- |
| `model` | the expense record, the category, the date, and conversion between text and those types |
| `store` | the collection, the category index, and the operations over them |
| `commands` | prompting, validation, error messages, and output formatting |
| `main` | the command loop |


## Running it

**Python** needs Python 3.8 or later.

```sh
cd python
python3 main.py
```

**C++** needs a compiler with C++17 support.

```sh
cd cpp
make
./expense-tracker
```

`make clean` removes the binary.

Both implementations end at the end of the input. Ctrl-D at the `> ` prompt ends the program.
Ctrl-D inside a command abandons that command and leaves the data as it was.

## Commands

| Command | Does |
| --- | --- |
| `add` | prompts for date, amount, category and description, then adds an expense |
| `list` | prints every expense, sorted by id |
| `filter` | prompts for a start date, an end date and a category, any of which may be left blank |
| `summary` | prints a total for each category and an overall total |
| `edit` | prompts for an id, then for each field, where blank keeps the current value |
| `delete` | prompts for an id and removes that expense |
| `quit` | ends the program |

## Using the commands

Start either program. Both print the menu, then the `> ` prompt.

Type one command word at the `> ` prompt and press Enter. The command then asks for each field it needs, one prompt at a time.
Answer each prompt and press Enter. The command ends, prints its result, and the `> ` prompt comes back.

Both implementations ask the same questions in the same order and print the same answers, so the steps below hold for either one.
Every sample here is the output of a real run, over the 12 expenses the program starts with.

### add

Type `add`, then answer four prompts.

```
> add
Date (YYYY-MM-DD): 2026-04-05
Amount: 42.50
Category (Food, Transport, Utilities, Entertainment, Other): Food
Description: Team lunch
Added expense 13.
```

The new expense gets the next unused id. The category is not case sensitive; `food` is read as `Food`.

A bad answer prints a message and asks the same question again. The command is not abandoned, and nothing is recorded until every field has passed.

```
> add
Date (YYYY-MM-DD): 2026-13-01
Date must be in YYYY-MM-DD form.
Date (YYYY-MM-DD): 2026-04-05
Amount: abc
Amount must be a number.
Amount: 42.50
Category (Food, Transport, Utilities, Entertainment, Other): Books
Category must be one of: Food, Transport, Utilities, Entertainment, Other.
Category (Food, Transport, Utilities, Entertainment, Other): Food
Description: Team lunch
Added expense 13.
```

### list

Type `list`. There are no prompts.

```
> list
  ID  DATE           AMOUNT  CATEGORY         DESCRIPTION
   1  2026-01-05     $12.50  Food             Coffee and pastry
   2  2026-01-08     $45.00  Transport        Monthly transit top-up
   3  2026-01-14    $120.75  Utilities        Electricity bill
```

Only the first three rows are shown here. The program prints every expense it holds.
When it holds none, it prints `No expenses.`

### filter

Type `filter`, then answer three prompts. Leave a prompt blank to put no limit on that field.

```
> filter
Start date (YYYY-MM-DD, blank for none): 2026-01-01
End date (YYYY-MM-DD, blank for none): 2026-01-31
Category (blank for all): Food
  ID  DATE           AMOUNT  CATEGORY         DESCRIPTION
   1  2026-01-05     $12.50  Food             Coffee and pastry
   5  2026-01-28     $32.40  Food             Grocery run
```

All three blank gives the same result as `list`. When nothing matches, the program prints `No expenses match.`

### summary

Type `summary`. There are no prompts.

```
> summary
Food              $148.85
Transport          $73.00
Utilities         $216.15
Entertainment      $33.99
Other              $31.55

Overall           $503.54
```

### edit

Type `edit`, then give the id of the expense to change.

Each field prompt shows the stored value in square brackets. Leave the prompt blank to keep that value. The id itself cannot be changed.

```
> edit
Expense id: 3
Date [2026-01-14] (blank to keep): 
Amount [120.75] (blank to keep): 135.00
Category [Utilities] (blank to keep): 
Description [Electricity bill] (blank to keep): Electricity and gas bill
Updated expense 3.
```

### delete

Type `delete`, then give the id.

```
> delete
Expense id: 1
Deleted expense 1.
```

A deleted id is never given out again.

### quit

Type `quit`.

```
> quit
Goodbye.
```

### When an id is not there

`edit` and `delete` both print the same message and end, leaving the data as it was.

```
> delete
Expense id: 99
No expense with id 99.
```


## Transcripts

Each implementation carries one recorded session, which is what the deliverable asks for. The two record different sessions.

| File | Holds |
| --- | --- |
| `python/transcript.txt` | one recorded session of the Python implementation |
| `cpp/session-transcript.txt` | one recorded session of the C++ implementation |
