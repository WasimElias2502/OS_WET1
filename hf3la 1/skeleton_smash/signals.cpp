#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation

    /////
    cout << "smash: got ctrl-C" << endl;
    pid_t forePid = SmallShell::getInstance().runningInForeground;
    if (forePid == -1){
        return;
    }
    if (killpg(forePid, SIGKILL) == -1){
        perror("smash error: killpg failed");
        return;
    }

    cout << "smash: process " << forePid << " was killed" << endl ;

    SmallShell::getInstance().runningInForeground = -1;

}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
}

