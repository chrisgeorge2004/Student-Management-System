#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FILE_NAME "students.dat"
#define TEMP_FILE "temp.dat"
#define SUBJECTS 5

typedef struct {
    int roll;
    char name[50];
    char course[50];
    float marks[SUBJECTS];
} Student;

// Input a line of text
void readText(const char *message, char *buffer, int size) {
    printf("%s", message);

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    // Clear remaining input if the line was too long
    if (strchr(buffer, '\0') == buffer + size - 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
}

// Input an integer safely
int readInt(const char *message) {
    char line[100];
    char *end;
    long value;

    while (1) {
        readText(message, line, sizeof(line));
        errno = 0;
        value = strtol(line, &end, 10);

        while (*end == ' ' || *end == '\t')
            end++;

        if (end != line && *end == '\0' &&
            errno == 0 && value >= 1 && value <= 1000000)
            return (int)value;

        printf("Please enter a valid positive number.\n");
    }
}

// Input marks between 0 and 100
float readMarks(const char *message) {
    char line[100];
    char *end;
    float value;

    while (1) {
        readText(message, line, sizeof(line));
        errno = 0;
        value = strtof(line, &end);

        while (*end == ' ' || *end == '\t')
            end++;

        if (end != line && *end == '\0' &&
            errno == 0 && value >= 0 && value <= 100)
            return value;

        printf("Enter marks between 0 and 100.\n");
    }
}

// Check whether roll number already exists
int rollExists(int roll) {
    FILE *fp = fopen(FILE_NAME, "rb");
    Student s;

    if (fp == NULL)
        return 0;

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.roll == roll) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

// Calculate total marks
float calculateTotal(Student s) {
    float total = 0;
    int i;

    for (i = 0; i < SUBJECTS; i++)
        total += s.marks[i];

    return total;
}

// Display one student
void displayStudent(Student s) {
    int i;
    float total = calculateTotal(s);

    printf("\n----------------------------------\n");
    printf("Roll Number : %d\n", s.roll);
    printf("Name        : %s\n", s.name);
    printf("Course      : %s\n", s.course);

    for (i = 0; i < SUBJECTS; i++)
        printf("Subject %d Marks: %.2f\n", i + 1, s.marks[i]);

    printf("Total Marks : %.2f / 500\n", total);
    printf("Percentage  : %.2f%%\n", total / 5);
    printf("----------------------------------\n");
}

// Add a student
void addStudent() {
    FILE *fp;
    Student s;
    int i;

    printf("\n--- ADD STUDENT ---\n");

    s.roll = readInt("Enter Roll Number: ");

    if (rollExists(s.roll)) {
        printf("This roll number already exists!\n");
        return;
    }

    readText("Enter Student Name: ", s.name, sizeof(s.name));
    readText("Enter Course: ", s.course, sizeof(s.course));

    for (i = 0; i < SUBJECTS; i++) {
        char prompt[50];
        snprintf(prompt, sizeof(prompt),
                 "Enter marks for Subject %d: ", i + 1);
        s.marks[i] = readMarks(prompt);
    }

    fp = fopen(FILE_NAME, "ab");

    if (fp == NULL) {
        printf("Unable to open student file!\n");
        return;
    }

    if (fwrite(&s, sizeof(Student), 1, fp) == 1)
        printf("\nStudent added successfully!\n");
    else
        printf("\nError saving student record!\n");

    fclose(fp);
}

// Display all students
void displayAll() {
    FILE *fp = fopen(FILE_NAME, "rb");
    Student s;
    int count = 0;

    printf("\n--- ALL STUDENT RECORDS ---\n");

    if (fp == NULL) {
        printf("No student records found.\n");
        return;
    }

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        displayStudent(s);
        count++;
    }

    if (count == 0)
        printf("No student records found.\n");
    else
        printf("\nTotal Students: %d\n", count);

    fclose(fp);
}

// Search student by roll number
void searchStudent() {
    FILE *fp = fopen(FILE_NAME, "rb");
    Student s;
    int roll, found = 0;

    printf("\n--- SEARCH STUDENT ---\n");
    roll = readInt("Enter Roll Number: ");

    if (fp == NULL) {
        printf("No records available.\n");
        return;
    }

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.roll == roll) {
            displayStudent(s);
            found = 1;
            break;
        }
    }

    if (!found)
        printf("Student not found!\n");

    fclose(fp);
}

// Update student information
void updateStudent() {
    FILE *fp = fopen(FILE_NAME, "rb+");
    Student s;
    int roll, found = 0, i;
    long position;

    printf("\n--- UPDATE STUDENT ---\n");
    roll = readInt("Enter Roll Number: ");

    if (fp == NULL) {
        printf("No records available.\n");
        return;
    }

    while ((position = ftell(fp)) >= 0 &&
           fread(&s, sizeof(Student), 1, fp) == 1) {

        if (s.roll == roll) {
            found = 1;

            printf("\nCurrent Record:");
            displayStudent(s);

            readText("Enter New Name: ", s.name, sizeof(s.name));
            readText("Enter New Course: ", s.course, sizeof(s.course));

            for (i = 0; i < SUBJECTS; i++) {
                char prompt[50];
                snprintf(prompt, sizeof(prompt),
                         "Enter new marks for Subject %d: ", i + 1);
                s.marks[i] = readMarks(prompt);
            }

            if (fseek(fp, position, SEEK_SET) == 0 &&
                fwrite(&s, sizeof(Student), 1, fp) == 1)
                printf("\nRecord updated successfully!\n");
            else
                printf("\nError updating record!\n");

            break;
        }
    }

    if (!found)
        printf("Student not found!\n");

    fclose(fp);
}

// Delete a student record
void deleteStudent() {
    FILE *fp, *temp;
    Student s;
    int roll, found = 0;
    int writeError = 0;

    printf("\n--- DELETE STUDENT ---\n");
    roll = readInt("Enter Roll Number: ");

    fp = fopen(FILE_NAME, "rb");

    if (fp == NULL) {
        printf("No records available.\n");
        return;
    }

    temp = fopen(TEMP_FILE, "wb");

    if (temp == NULL) {
        printf("Unable to create temporary file!\n");
        fclose(fp);
        return;
    }

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.roll == roll) {
            found = 1;
        } else {
            if (fwrite(&s, sizeof(Student), 1, temp) != 1) {
                writeError = 1;
                break;
            }
        }
    }

    if (ferror(fp))
        writeError = 1;

    fclose(fp);
    fclose(temp);

    if (writeError) {
        remove(TEMP_FILE);
        printf("Error processing records. Original file preserved.\n");
        return;
    }

    if (!found) {
        remove(TEMP_FILE);
        printf("Student not found!\n");
        return;
    }

    if (remove(FILE_NAME) != 0) {
        remove(TEMP_FILE);
        printf("Unable to replace student file!\n");
        return;
    }

    if (rename(TEMP_FILE, FILE_NAME) != 0) {
        printf("Error replacing student file!\n");
        return;
    }

    printf("Student record deleted successfully!\n");
}

// Main function
int main() {
    int choice;

    do {
        printf("\n====================================\n");
        printf("      STUDENT MANAGEMENT SYSTEM\n");
        printf("====================================\n");
        printf("1. Add Student\n");
        printf("2. Display All Students\n");
        printf("3. Search Student\n");
        printf("4. Update Student\n");
        printf("5. Delete Student\n");
        printf("6. Exit\n");
        printf("====================================\n");

        choice = readInt("Enter Your Choice: ");

        switch (choice) {
            case 1:
                addStudent();
                break;

            case 2:
                displayAll();
                break;

            case 3:
                searchStudent();
                break;

            case 4:
                updateStudent();
                break;

            case 5:
                deleteStudent();
                break;

            case 6:
                printf("\nThank you for using the system!\n");
                break;

            default:
                printf("Invalid choice!\n");
        }

    } while (choice != 6);

    return 0;
}
