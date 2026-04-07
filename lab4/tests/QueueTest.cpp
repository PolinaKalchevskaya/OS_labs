#include "../src/Shared/QueueHeader.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;


class UnitTest {
private:
    int passed = 0;
    int failed = 0;

public:
    void AssertEqual(const string& expected, const string& actual, const string& testName) {
        if (expected == actual) {
            cout << "[PASS] " << testName << endl;
            passed++;
        }
        else {
            cout << "[FAIL] " << testName << " - expected: \"" << expected
                << "\", actual: \"" << actual << "\"" << endl;
            failed++;
        }
    }

    void AssertTrue(bool condition, const string& testName) {
        if (condition) {
            cout << "[PASS] " << testName << endl;
            passed++;
        }
        else {
            cout << "[FAIL] " << testName << endl;
            failed++;
        }
    }

    void AssertFalse(bool condition, const string& testName) {
        AssertTrue(!condition, testName);
    }

    void PrintResults() {
        cout << "\n========== RESULTS ==========" << endl;
        cout << "Passed: " << passed << endl;
        cout << "Failed: " << failed << endl;
        cout << "Total:  " << (passed + failed) << endl;
        cout << "=============================" << endl;
    }

    int GetFailedCount() const { return failed; }
};

wstring GetTestFileName() {
    return L"test_queue.bin";
}

void Cleanup() {
    DeleteFileW(GetTestFileName().c_str());
}


void TestCreateQueue(UnitTest& test) {
    Cleanup();
    try {
        MessageQueue queue(GetTestFileName(), 5, true);
        test.AssertTrue(true, "CreateQueue");
    }
    catch (...) {
        test.AssertTrue(false, "CreateQueue");
    }
    Cleanup();
}

void TestWriteAndRead(UnitTest& test) {
    Cleanup();
    try {
        MessageQueue queue(GetTestFileName(), 5, true);

        string testMsg = "Hello World!";
        test.AssertTrue(queue.WriteMessage(testMsg), "WriteMessage");

        string readMsg;
        test.AssertTrue(queue.ReadMessage(readMsg), "ReadMessage");
        test.AssertEqual(testMsg, readMsg, "MessageContent");
    }
    catch (...) {
        test.AssertTrue(false, "WriteAndRead");
    }
    Cleanup();
}

void TestQueueFull(UnitTest& test) {
    Cleanup();
    try {
        MessageQueue queue(GetTestFileName(), 3, true);

        test.AssertTrue(queue.WriteMessage("Msg1"), "Write1");
        test.AssertTrue(queue.WriteMessage("Msg2"), "Write2");
        test.AssertTrue(queue.WriteMessage("Msg3"), "Write3");
        test.AssertTrue(queue.IsFull(), "IsFull");
        test.AssertFalse(queue.WriteMessage("Msg4"), "WriteWhenFull");
    }
    catch (...) {
        test.AssertTrue(false, "QueueFull");
    }
    Cleanup();
}

void TestFIFOOrder(UnitTest& test) {
    Cleanup();
    try {
        MessageQueue queue(GetTestFileName(), 5, true);

        vector<string> messages = { "First", "Second", "Third" };
        for (const auto& msg : messages) {
            queue.WriteMessage(msg);
        }

        for (const auto& expected : messages) {
            string actual;
            queue.ReadMessage(actual);
            test.AssertEqual(expected, actual, "FIFO_" + expected);
        }
    }
    catch (...) {
        test.AssertTrue(false, "FIFOOrder");
    }
    Cleanup();
}

void TestMaxMessageLength(UnitTest& test) {
    Cleanup();
    try {
        MessageQueue queue(GetTestFileName(), 5, true);

        string exactMsg(MAX_MESSAGE_LEN, 'A');
        string longMsg(MAX_MESSAGE_LEN + 1, 'B');

        test.AssertTrue(queue.WriteMessage(exactMsg), "ExactLengthMessage");
        test.AssertFalse(queue.WriteMessage(longMsg), "TooLongMessage");
    }
    catch (...) {
        test.AssertTrue(false, "MaxMessageLength");
    }
    Cleanup();
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    cout << "========== UNIT TESTS ==========" << endl;
    cout << "Testing MessageQueue (кольцевая очередь FIFO)\n" << endl;

    UnitTest test;

    TestCreateQueue(test);
    TestWriteAndRead(test);
    TestQueueFull(test);
    TestFIFOOrder(test);
    TestMaxMessageLength(test);

    test.PrintResults();

    return test.GetFailedCount();
}