#include <ftw.h>
#include <inttypes.h>
#include <signal.h>
#include <unistd.h>

int main() {
    sigset_t full_mask;
    sigfillset(&full_mask);
    sigprocmask(SIG_BLOCK, &full_mask, NULL);
    sigset_t mask;
    sigaddset(&mask, SIGRTMIN);

    while (1) {
        siginfo_t info;
        sigwaitinfo(&mask, &info);
        int32_t received_value = info.si_value.sival_int;
        if (received_value > 0) {
            sigqueue(info.si_pid, SIGRTMIN, (union sigval){.sival_int = received_value - 1});
            continue;
        }
        break;
    }
    return 0;
}

