# Electricity Board User Management System

A console-based **C11** application that manages electricity board customers and their connection requests using simple text-file persistence. The program runs interactively in any terminal and retains all data between sessions.

---

## Features

| # | Feature |
|---|---------|
| 1 | **Add New User** – Register a customer with full name (including spaces) and a validated 10-digit mobile number. Prevents duplicate entries. |
| 2 | **View Registered Users** – Display all customers in a formatted table with User ID, name, and mobile number. |
| 3 | **Add New Connection Request** – Create a connection request for a registered customer (look up by mobile number or User ID). Assigns a unique Request ID and sets status to *Pending*. |
| 4 | **Edit Existing Connection Request** – Update the address and/or status of an existing request. Supports *Pending*, *Approved*, and *Rejected* statuses. |
| 5 | **View Connection Requests** – Display all requests in a formatted table cross-referenced with customer names and mobile numbers. |
| 6 | **Exit** – Exits the program cleanly. |

---

## Technologies Used

- **Language:** C (C11 standard)
- **Compiler:** GCC (tested with MinGW GCC 6.3.0 on Windows)
- **Data storage:** Plain text files with pipe (`|`) delimiters
- **Libraries:** Standard C library only (`stdio.h`, `stdlib.h`, `string.h`, `ctype.h`, `errno.h`)

---

## Prerequisites

- GCC compiler (MinGW on Windows, or system GCC on Linux/macOS)
- A terminal or command prompt

### Install GCC (Windows)

Download MinGW from [mingw.org](https://www.mingw.org/) or [winlibs.com](https://winlibs.com/).  
Add the `bin` directory (e.g. `C:\MinGW\bin`) to your system `PATH`.

---

## Compilation

```bash
gcc electrictiy.c -o electricity.exe -std=c11 -Wall -Wextra -Wpedantic
```

The source file name `electrictiy.c` is intentionally preserved from the original project.

**Linux / macOS:**

```bash
gcc electrictiy.c -o electricity -std=c11 -Wall -Wextra -Wpedantic
```

Expected result: **zero warnings, zero errors**.

---

## How to Run

**Windows:**
```
electricity.exe
```

**Linux / macOS:**
```bash
./electricity
```

The program runs continuously and returns to the main menu after each operation until you choose **Exit (6)**.

---

## Menu Descriptions

```
======================================
   Electricity Board Management System
======================================
 1. Add New User
 2. View Registered Users
 3. Add New Connection Request
 4. Edit Existing Connection Request
 5. View Connection Requests
 6. Exit
--------------------------------------
```

### 1. Add New User

- Prompts for the customer's full name (spaces allowed, e.g. `Yatharth Rajput`).
- Prompts for a 10-digit mobile number (stored as a string to preserve leading zeros).
- Validates the name and mobile number.
- Rejects duplicate mobile numbers.
- Generates a unique User ID automatically.
- Appends the record to `users.txt`.

### 2. View Registered Users

- Loads all records from `users.txt`.
- Displays a formatted table: `User ID | Full Name | Mobile Number`.
- Shows a message when no users are registered.

### 3. Add New Connection Request

- Prompts for the registered user's mobile number **or** User ID.
- Validates that the user exists — rejects requests for unknown users.
- Prompts for a connection address.
- Assigns a unique Request ID, sets status to `Pending`.
- Appends the request to `requests.txt`.

### 4. Edit Existing Connection Request

- Prompts for the Request ID.
- Verifies the request exists.
- Allows updating the connection address (leave blank to keep the current value).
- Allows changing the status: `1=Pending`, `2=Approved`, `3=Rejected`, `0=keep current`.
- Rewrites `requests.txt` via a safe temporary file so no data is lost if an error occurs.

### 5. View Connection Requests

- Displays all requests in a formatted table.
- Cross-references User IDs with `users.txt` to show customer names and mobile numbers.
- Shows `N/A` if a user record cannot be found.

### 6. Exit

Prints a goodbye message and exits with code 0.

---

## Data Files

### `users.txt`

Pipe-delimited text file. One user per line:

```
<UserID>|<Full Name>|<MobileNumber>
```

**Example:**
```
1|Yatharth Rajput|9876543210
2|Priya Sharma|9123456789
```

### `requests.txt`

Pipe-delimited text file. One request per line:

```
<RequestID>|<UserID>|<Address>|<Status>
```

**Example:**
```
1|1|123 Main Street Mumbai MH 400001|Approved
2|2|45 Green Park Delhi 110016|Pending
```

> **Note:** Pipe characters (`|`) in addresses are automatically replaced with `/` to protect the delimiter.

Both files are created automatically when the first record is added. They are **not** included in the Git repository to avoid committing customer data.

---

## Input Validation and Error Handling

| Scenario | Behaviour |
|----------|-----------|
| Non-numeric menu choice | Rejected with a clear error message; menu is shown again |
| Out-of-range menu number | Rejected with a clear error message |
| Name with only whitespace | Rejected: "Invalid name" |
| Mobile number ≠ 10 digits | Rejected: "must be exactly 10 digits" |
| Duplicate mobile number | Rejected: "already registered" |
| Non-existent user for request | Rejected: "No registered user found" |
| Non-existent request ID for edit | Rejected: "not found" |
| File open / write failure | Error reported via `perror()` or `fprintf(stderr, ...)` |
| Temp-file write failure during edit | Original data preserved; temp file deleted |
| Missing or empty data files | Treated as zero records; appropriate message shown |
| Malformed records in data files | Warning printed to stderr; record skipped |
| EOF on stdin | Detected and treated as a clean exit |

---

## Current Limitations and Future Improvements

| Limitation | Suggested Improvement |
|------------|----------------------|
| Fixed in-memory cap of 500 users / 500 requests | Switch to dynamic linked list or database |
| Linear search for user lookup | Add a hash map or sorted index for faster lookups |
| No authentication | Add role-based access (admin vs. field officer) |
| No date/time tracking | Record timestamps for user registration and request creation |
| Address truncated in table view to 29 chars | Use a pager or wider terminal output |
| Data files not backed up before edit | Implement an automatic backup before rewriting |
| No search/filter for requests | Add filter by status, user ID, or date range |
| Console-only UI | Port to a GUI (GTK, ncurses) or a web interface |

---

## File Structure

```
electrictiy.c      ← Main C source file (all logic in one file)
electricity.exe    ← Compiled binary (Windows, excluded from Git)
users.txt          ← Runtime data: registered customers (excluded from Git)
requests.txt       ← Runtime data: connection requests (excluded from Git)
README.md          ← This documentation file
.gitignore         ← Git ignore rules
```

---

## License

This project is open source and available for educational use.
