/*
 * electrictiy.c
 * Electricity Board User Management System
 *
 * A console-based C11 application that manages electricity board customers
 * and their connection requests using simple text-file persistence.
 *
 * Features:
 *  1. Add New User
 *  2. View Registered Users
 *  3. Add New Connection Request
 *  4. Edit Existing Connection Request
 *  5. View Connection Requests
 *  6. Exit
 *
 * Data files:
 *  users.txt    – registered customers  (pipe-delimited: id|name|mobile)
 *  requests.txt – connection requests   (pipe-delimited: id|uid|address|status)
 *
 * Compile (MinGW GCC):
 *   gcc electrictiy.c -o electricity.exe -std=c11 -Wall -Wextra -Wpedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* ── Constants ─────────────────────────────────────────────────────────── */

#define USERS_FILE    "users.txt"
#define REQUESTS_FILE "requests.txt"
#define TEMP_FILE     "requests_tmp.txt"

#define MAX_NAME    100
#define MAX_MOBILE   15
#define MAX_ADDRESS 200
#define MAX_STATUS   20
#define MAX_USERS   500
#define MAX_REQUESTS 500

#define MOBILE_LEN   10   /* expected digit count */

/* ── Structures ─────────────────────────────────────────────────────────── */

typedef struct {
    int  id;
    char name[MAX_NAME];
    char mobile[MAX_MOBILE];
} User;

typedef struct {
    int  id;
    int  user_id;
    char address[MAX_ADDRESS];
    char status[MAX_STATUS];   /* "Pending", "Approved", "Rejected" */
} ConnectionRequest;

/* ── Utility helpers ────────────────────────────────────────────────────── */

/*
 * strip_newline – remove trailing '\n' or '\r' from a string.
 */
static void strip_newline(char *s)
{
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
        s[--len] = '\0';
}

/*
 * read_line – read one line from stdin into buf (max buf_size chars).
 * Returns 1 on success, 0 on EOF or error.
 * Always NUL-terminates buf and strips the trailing newline.
 */
static int read_line(char *buf, int buf_size)
{
    if (!fgets(buf, buf_size, stdin))
        return 0;
    strip_newline(buf);
    return 1;
}


/*
 * is_digits – return 1 if every character in s is a digit, 0 otherwise.
 */
static int is_digits(const char *s)
{
    if (*s == '\0') return 0;
    for (; *s; s++)
        if (!isdigit((unsigned char)*s))
            return 0;
    return 1;
}

/*
 * is_valid_name – return 1 if name contains at least one non-space character.
 */
static int is_valid_name(const char *s)
{
    for (; *s; s++)
        if (!isspace((unsigned char)*s))
            return 1;
    return 0;
}

/*
 * is_valid_mobile – exactly MOBILE_LEN digits.
 */
static int is_valid_mobile(const char *s)
{
    return (strlen(s) == MOBILE_LEN && is_digits(s));
}

/* ── ID generation ──────────────────────────────────────────────────────── */

/*
 * next_user_id – scan users.txt and return (max existing id) + 1.
 */
static int next_user_id(void)
{
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return 1;

    int max_id = 0, id;
    char name[MAX_NAME], mobile[MAX_MOBILE], line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d|%99[^|]|%14s", &id, name, mobile) == 3)
            if (id > max_id) max_id = id;
    }
    fclose(fp);
    return max_id + 1;
}

/*
 * next_request_id – scan requests.txt and return (max existing id) + 1.
 */
static int next_request_id(void)
{
    FILE *fp = fopen(REQUESTS_FILE, "r");
    if (!fp) return 1;

    int max_id = 0, id, uid;
    char address[MAX_ADDRESS], status[MAX_STATUS], line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d|%d|%199[^|]|%19s", &id, &uid, address, status) == 4)
            if (id > max_id) max_id = id;
    }
    fclose(fp);
    return max_id + 1;
}

/* ── User helpers ───────────────────────────────────────────────────────── */

/*
 * load_users – fill array, return count (up to MAX_USERS).
 * Returns -1 if the file cannot be opened (file-not-found is treated as 0 users).
 */
static int load_users(User *users)
{
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return 0;   /* no file yet – zero users */

    int count = 0;
    char line[512];
    while (count < MAX_USERS && fgets(line, sizeof(line), fp)) {
        strip_newline(line);
        if (strlen(line) == 0) continue;
        User u;
        if (sscanf(line, "%d|%99[^|]|%14s", &u.id, u.name, u.mobile) == 3) {
            users[count++] = u;
        } else {
            fprintf(stderr, "[warn] Skipping malformed user record: %s\n", line);
        }
    }
    fclose(fp);
    return count;
}

/*
 * find_user_by_mobile – linear search; returns index or -1.
 */
static int find_user_by_mobile(const User *users, int count, const char *mobile)
{
    for (int i = 0; i < count; i++)
        if (strcmp(users[i].mobile, mobile) == 0)
            return i;
    return -1;
}

/*
 * find_user_by_id – linear search; returns index or -1.
 */
static int find_user_by_id(const User *users, int count, int id)
{
    for (int i = 0; i < count; i++)
        if (users[i].id == id)
            return i;
    return -1;
}

/* ── Request helpers ────────────────────────────────────────────────────── */

/*
 * load_requests – fill array, return count (up to MAX_REQUESTS).
 */
static int load_requests(ConnectionRequest *reqs)
{
    FILE *fp = fopen(REQUESTS_FILE, "r");
    if (!fp) return 0;

    int count = 0;
    char line[512];
    while (count < MAX_REQUESTS && fgets(line, sizeof(line), fp)) {
        strip_newline(line);
        if (strlen(line) == 0) continue;
        ConnectionRequest r;
        if (sscanf(line, "%d|%d|%199[^|]|%19s", &r.id, &r.user_id, r.address, r.status) == 4) {
            reqs[count++] = r;
        } else {
            fprintf(stderr, "[warn] Skipping malformed request record: %s\n", line);
        }
    }
    fclose(fp);
    return count;
}

/* ── Menu features ──────────────────────────────────────────────────────── */

/*
 * Feature 1: Add New User
 */
static void add_new_user(void)
{
    printf("\n----- Add New User -----\n");

    /* Load existing users to check duplicates */
    User *users = malloc(MAX_USERS * sizeof(User));
    if (!users) { fprintf(stderr, "[error] Out of memory.\n"); return; }
    int count = load_users(users);

    char name[MAX_NAME], mobile[MAX_MOBILE];

    /* Get name */
    printf("Enter customer full name: ");
    if (!read_line(name, sizeof(name)) || !is_valid_name(name)) {
        printf("[error] Invalid name. Operation cancelled.\n");
        free(users);
        return;
    }

    /* Get and validate mobile */
    printf("Enter 10-digit mobile number: ");
    if (!read_line(mobile, sizeof(mobile))) {
        printf("[error] Failed to read input. Operation cancelled.\n");
        free(users);
        return;
    }
    if (!is_valid_mobile(mobile)) {
        printf("[error] Mobile number must be exactly 10 digits. Operation cancelled.\n");
        free(users);
        return;
    }

    /* Duplicate check */
    if (find_user_by_mobile(users, count, mobile) >= 0) {
        printf("[error] A user with mobile number %s is already registered.\n", mobile);
        free(users);
        return;
    }

    free(users);

    /* Assign ID and write */
    int new_id = next_user_id();

    FILE *fp = fopen(USERS_FILE, "a");
    if (!fp) {
        perror("[error] Cannot open " USERS_FILE);
        return;
    }
    if (fprintf(fp, "%d|%s|%s\n", new_id, name, mobile) < 0) {
        fprintf(stderr, "[error] Write failed to " USERS_FILE ".\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    printf("[success] User registered successfully!\n");
    printf("  User ID : %d\n", new_id);
    printf("  Name    : %s\n", name);
    printf("  Mobile  : %s\n", mobile);
}

/*
 * Feature 2: View Registered Users
 */
static void view_users(void)
{
    printf("\n----- Registered Users -----\n");

    User *users = malloc(MAX_USERS * sizeof(User));
    if (!users) { fprintf(stderr, "[error] Out of memory.\n"); return; }
    int count = load_users(users);

    if (count == 0) {
        printf("No users registered yet.\n");
        free(users);
        return;
    }

    printf("%-8s %-30s %-15s\n", "User ID", "Full Name", "Mobile Number");
    printf("%-8s %-30s %-15s\n",
           "-------", "-----------------------------", "--------------");
    for (int i = 0; i < count; i++)
        printf("%-8d %-30s %-15s\n", users[i].id, users[i].name, users[i].mobile);

    printf("\nTotal users: %d\n", count);
    free(users);
}

/*
 * Feature 3: Add New Connection Request
 */
static void add_connection_request(void)
{
    printf("\n----- Add New Connection Request -----\n");

    User *users = malloc(MAX_USERS * sizeof(User));
    if (!users) { fprintf(stderr, "[error] Out of memory.\n"); return; }
    int count = load_users(users);

    if (count == 0) {
        printf("No users registered. Please add a user first.\n");
        free(users);
        return;
    }

    /* Look up user by mobile or ID */
    printf("Enter registered user's mobile number or User ID to look up: ");
    char input[MAX_MOBILE];
    if (!read_line(input, sizeof(input))) {
        printf("[error] Failed to read input.\n");
        free(users);
        return;
    }

    int user_idx = -1;

    if (is_digits(input)) {
        /* Try as mobile (10 digits) */
        if (strlen(input) == MOBILE_LEN) {
            user_idx = find_user_by_mobile(users, count, input);
        }
        /* Try as user ID */
        if (user_idx < 0) {
            int uid = atoi(input);
            user_idx = find_user_by_id(users, count, uid);
        }
    }

    if (user_idx < 0) {
        printf("[error] No registered user found for: %s\n", input);
        free(users);
        return;
    }

    User *u = &users[user_idx];
    printf("  Found user -> ID: %d | Name: %s | Mobile: %s\n",
           u->id, u->name, u->mobile);

    /* Get connection address */
    char address[MAX_ADDRESS];
    printf("Enter connection address: ");
    if (!read_line(address, sizeof(address)) || !is_valid_name(address)) {
        printf("[error] Invalid address. Operation cancelled.\n");
        free(users);
        return;
    }

    /* Replace any pipe characters to protect the delimiter */
    for (int i = 0; address[i]; i++)
        if (address[i] == '|') address[i] = '/';

    int new_id = next_request_id();
    int user_id = u->id;
    free(users);

    FILE *fp = fopen(REQUESTS_FILE, "a");
    if (!fp) {
        perror("[error] Cannot open " REQUESTS_FILE);
        return;
    }
    if (fprintf(fp, "%d|%d|%s|Pending\n", new_id, user_id, address) < 0) {
        fprintf(stderr, "[error] Write failed to " REQUESTS_FILE ".\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    printf("[success] Connection request created!\n");
    printf("  Request ID : %d\n", new_id);
    printf("  User ID    : %d\n", user_id);
    printf("  Address    : %s\n", address);
    printf("  Status     : Pending\n");
}

/*
 * Feature 4: Edit Existing Connection Request
 */
static void edit_connection_request(void)
{
    printf("\n----- Edit Connection Request -----\n");

    ConnectionRequest *reqs = malloc(MAX_REQUESTS * sizeof(ConnectionRequest));
    if (!reqs) { fprintf(stderr, "[error] Out of memory.\n"); return; }
    int count = load_requests(reqs);

    if (count == 0) {
        printf("No connection requests found.\n");
        free(reqs);
        return;
    }

    printf("Enter Request ID to edit: ");
    char input[32];
    if (!read_line(input, sizeof(input)) || !is_digits(input)) {
        printf("[error] Invalid request ID.\n");
        free(reqs);
        return;
    }
    int req_id = atoi(input);

    /* Find the request */
    int found = -1;
    for (int i = 0; i < count; i++)
        if (reqs[i].id == req_id) { found = i; break; }

    if (found < 0) {
        printf("[error] Request ID %d not found.\n", req_id);
        free(reqs);
        return;
    }

    ConnectionRequest *r = &reqs[found];
    printf("  Current address : %s\n", r->address);
    printf("  Current status  : %s\n", r->status);

    /* New address */
    char new_address[MAX_ADDRESS];
    printf("Enter new connection address (leave blank to keep current): ");
    if (!read_line(new_address, sizeof(new_address))) {
        printf("[error] Failed to read input.\n");
        free(reqs);
        return;
    }
    if (strlen(new_address) > 0) {
        /* Replace pipe characters */
        for (int i = 0; new_address[i]; i++)
            if (new_address[i] == '|') new_address[i] = '/';
        strncpy(r->address, new_address, MAX_ADDRESS - 1);
        r->address[MAX_ADDRESS - 1] = '\0';
    }

    /* New status */
    printf("Enter new status (1=Pending, 2=Approved, 3=Rejected, 0=keep current): ");
    char schoice[8];
    if (!read_line(schoice, sizeof(schoice))) {
        printf("[error] Failed to read input.\n");
        free(reqs);
        return;
    }
    if (strcmp(schoice, "1") == 0)      strncpy(r->status, "Pending",  MAX_STATUS - 1);
    else if (strcmp(schoice, "2") == 0) strncpy(r->status, "Approved", MAX_STATUS - 1);
    else if (strcmp(schoice, "3") == 0) strncpy(r->status, "Rejected", MAX_STATUS - 1);
    else if (strcmp(schoice, "0") != 0) {
        printf("[warn] Unrecognised choice '%s' – status unchanged.\n", schoice);
    }
    r->status[MAX_STATUS - 1] = '\0';

    /* Rewrite requests file via temp file */
    FILE *tmp = fopen(TEMP_FILE, "w");
    if (!tmp) {
        perror("[error] Cannot create temporary file");
        free(reqs);
        return;
    }
    int write_error = 0;
    for (int i = 0; i < count; i++) {
        if (fprintf(tmp, "%d|%d|%s|%s\n",
                    reqs[i].id, reqs[i].user_id,
                    reqs[i].address, reqs[i].status) < 0) {
            write_error = 1;
            break;
        }
    }
    fclose(tmp);
    free(reqs);

    if (write_error) {
        fprintf(stderr, "[error] Write failed; original data preserved.\n");
        remove(TEMP_FILE);
        return;
    }

    /* Atomically replace the original file */
    if (remove(REQUESTS_FILE) != 0 && errno != ENOENT) {
        perror("[error] Cannot remove original requests file");
        remove(TEMP_FILE);
        return;
    }
    if (rename(TEMP_FILE, REQUESTS_FILE) != 0) {
        perror("[error] Cannot rename temp file to requests file");
        return;
    }

    printf("[success] Request ID %d updated successfully.\n", req_id);
}

/*
 * Feature 5: View Connection Requests
 */
static void view_connection_requests(void)
{
    printf("\n----- Connection Requests -----\n");

    ConnectionRequest *reqs = malloc(MAX_REQUESTS * sizeof(ConnectionRequest));
    if (!reqs) { fprintf(stderr, "[error] Out of memory.\n"); return; }
    int count = load_requests(reqs);

    if (count == 0) {
        printf("No connection requests found.\n");
        free(reqs);
        return;
    }

    /* Load users for cross-referencing */
    User *users = malloc(MAX_USERS * sizeof(User));
    if (!users) { fprintf(stderr, "[error] Out of memory.\n"); free(reqs); return; }
    int ucount = load_users(users);

    printf("%-6s %-8s %-25s %-15s %-30s %-10s\n",
           "ReqID", "UserID", "Customer Name", "Mobile", "Address", "Status");
    printf("%-6s %-8s %-25s %-15s %-30s %-10s\n",
           "-----", "------", "------------------------",
           "--------------", "-----------------------------", "---------");

    for (int i = 0; i < count; i++) {
        const char *uname  = "N/A";
        const char *mobile = "N/A";
        int idx = find_user_by_id(users, ucount, reqs[i].user_id);
        if (idx >= 0) {
            uname  = users[idx].name;
            mobile = users[idx].mobile;
        }
        printf("%-6d %-8d %-25.24s %-15s %-30.29s %-10s\n",
               reqs[i].id, reqs[i].user_id,
               uname, mobile,
               reqs[i].address, reqs[i].status);
    }

    printf("\nTotal requests: %d\n", count);
    free(reqs);
    free(users);
}

/* ── Main menu loop ─────────────────────────────────────────────────────── */

static void print_menu(void)
{
    printf("\n======================================\n");
    printf("   Electricity Board Management System\n");
    printf("======================================\n");
    printf(" 1. Add New User\n");
    printf(" 2. View Registered Users\n");
    printf(" 3. Add New Connection Request\n");
    printf(" 4. Edit Existing Connection Request\n");
    printf(" 5. View Connection Requests\n");
    printf(" 6. Exit\n");
    printf("--------------------------------------\n");
    printf(" Enter your choice: ");
}

int main(void)
{
    printf("\n----- Welcome to Electricity Board Management System -----\n");

    char choice_buf[8];
    while (1) {
        print_menu();

        if (!read_line(choice_buf, sizeof(choice_buf))) {
            /* EOF on stdin – treat as exit */
            printf("\n[info] End of input detected. Exiting.\n");
            break;
        }

        /* Validate that the input is a single integer */
        if (!is_digits(choice_buf) || strlen(choice_buf) == 0) {
            printf("[error] Invalid choice. Please enter a number from 1 to 6.\n");
            continue;
        }

        int choice = atoi(choice_buf);
        switch (choice) {
            case 1: add_new_user();              break;
            case 2: view_users();                break;
            case 3: add_connection_request();    break;
            case 4: edit_connection_request();   break;
            case 5: view_connection_requests();  break;
            case 6:
                printf("\nThank you for using the Electricity Board Management System. Goodbye!\n");
                return 0;
            default:
                printf("[error] Invalid choice '%d'. Please enter a number from 1 to 6.\n", choice);
                break;
        }
    }

    return 0;
}