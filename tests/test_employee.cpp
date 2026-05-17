#include <iostream>
#include <string>
#include "../src/common/employee.h"

using namespace std;

int testsPassed = 0;
int testsFailed = 0;

void assertTrue(bool condition, const string& testName) {
    if (condition) {
        cout << "[PASS] " << testName << endl;
        testsPassed++;
    }
    else {
        cout << "[FAIL] " << testName << endl;
        testsFailed++;
    }
}

void testDefaultConstructor() {
    Employee e;
    assertTrue(e.num == 0, "DefaultConstructor - num is 0");
    assertTrue(e.hours == 0.0, "DefaultConstructor - hours is 0");
    assertTrue(e.name[0] == '\0', "DefaultConstructor - name is empty");
}

void testParameterizedConstructor() {
    Employee e(123, "John", 40.5);
    assertTrue(e.num == 123, "ParameterizedConstructor - num is 123");
    assertTrue(e.hours == 40.5, "ParameterizedConstructor - hours is 40.5");
    assertTrue(string(e.name) == "John", "ParameterizedConstructor - name is John");
}

void testNameTruncation() {
    Employee e(1, "VeryLongName", 10.0);

  
    string actual(e.name);
    string expected = "VeryLongN";  

    cout << "[DEBUG] Actual: '" << actual << "'" << endl;
    cout << "[DEBUG] Expected: '" << expected << "'" << endl;

    assertTrue(actual == expected, "NameTruncation - name truncated to 9 chars");
    assertTrue(e.name[9] == '\0', "NameTruncation - name[9] is null terminator");
}

void testNameExactNine() {
    Employee e(2, "123456789", 20.0);
    string actual(e.name);
    string expected = "123456789";

    cout << "[DEBUG] Actual: '" << actual << "'" << endl;
    cout << "[DEBUG] Expected: '" << expected << "'" << endl;

    assertTrue(actual == expected, "NameExactNine - exactly 9 chars fits");
    assertTrue(e.name[9] == '\0', "NameExactNine - name[9] is null terminator");
}

int main() {
    cout << "\n=== Running Unit Tests ===\n\n";

    testDefaultConstructor();
    testParameterizedConstructor();
    testNameTruncation();
    testNameExactNine();

    cout << "\n=== Results ===\n";
    cout << "Passed: " << testsPassed << endl;
    cout << "Failed: " << testsFailed << endl;
    cout << "Total:  " << (testsPassed + testsFailed) << endl;

    if (testsFailed == 0) {
        cout << "\n ALL TESTS PASSED!\n" << endl;
    }
    else {
        cout << "\n SOME TESTS FAILED!\n" << endl;
    }

    system("pause");
    return 0;
}