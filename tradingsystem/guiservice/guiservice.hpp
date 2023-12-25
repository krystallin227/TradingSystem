#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
    // Create an ofstream object
    ofstream outputFile;

    // Open the file in append mode
    outputFile.open("example.txt", ios::app);

    // Check if the file is open
    if (outputFile.is_open()) {
        // Write to the file
        outputFile << "Adding this line to the file.\n";

        // Close the file
        outputFile.close();
    }
    else {
        cout << "Unable to open file";
    }

    return 0;
}
