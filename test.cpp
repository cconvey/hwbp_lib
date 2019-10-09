#include <iostream>
#include "hwbp.h"

using namespace std;

/*
 * Example of how to use this /library/.
 */
int handled = 0;

void handle(int s) {
	handled = 1;
	return;
}

int main(int argc, char **argv) {
	int a = 0;

	if (!install_breakpoint(&a, sizeof(a), 0, handle)) {
		cout << "failed to set the breakpoint!" << endl;
                return 1;
        }

        cout << "About to read 'a'." << endl;
        cout << "Done." << endl;
	cout << "handled: " << handled << endl;
        cout << endl;

        cout << "About to write 'a'." << endl;
	a = 1;
        cout << "Done." << endl;
	cout << "handled: " << handled << endl;
        cout << endl;

	if (!disable_breakpoint(0)) {
		cout << "failed to disable the breakpoint!" << endl;
                return 1;
        }
}
