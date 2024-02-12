#include <signal.h>
#include <stdio.h>

void my_handler(int nsig) {
  /*
  As far as I've understood, this task is supposed to be an easy one,
  and there is no need to workaround issues like impossibility to set handler
  for SIGKILL and SIGSTOP, possible undefined behaviour of process that does not exit()
  in case of getting some signals (e.g. SIGFPE, SIGSEGV) and 
  non-async-signal-safety of printf()
  */
  printf("Received signal %d\n", nsig);
}

int main(void){ 
  size_t max_signal = SIGRTMAX;
  for (size_t i = 0; i <= max_signal; ++i) {
    signal(i, my_handler);
  }
  while(1);
  return 0;
}
