#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <errno.h>
#include <stddef.h>

#include "signal.h"

static void
interrupting_signal_handler(int signo)
{
  /* Its only job is to interrupt system calls--like read()--when
   * certain signals arrive--like Ctrl-C.
   */
}

static struct sigaction ignore_action = {.sa_handler = SIG_IGN},
                        interrupt_action = {.sa_handler =
                                                interrupting_signal_handler},
                        old_sigtstp, old_sigint, old_sigttou;

/* Ignore certain signals.
 * 
 * @returns 0 on succes, -1 on failure
 *
 *
 * The list of signals to ignore:
 *   - SIGTSTP
 *   - SIGINT
 *   - SIGTTOU
 *
 * Should be called immediately on entry to main() 
 *
 * Saves old signal dispositions for a later call to signal_restore()
 */
int
signal_init(void)
{
  /* BGDID Initialize signals, store old actions 
   *
   * e.g. sigaction(SIGNUM, &new_handler, &saved_old_handler);
   *
   * */

  if (sigaction(SIGTSTP, &ignore_action, &old_sigtstp) < 0) return -1;

  if (sigaction(SIGINT, &ignore_action, &old_sigint) < 0) return -1;

  if (sigaction(SIGTTOU, &ignore_action, &old_sigttou) < 0) return -1;

  return 0;

}

/** enable signal to interrupt blocking syscalls (read/getline, etc) 
 *
 * @returns 0 on succes, -1 on failure
 *
 * does not save old signal disposition
 */
int
signal_enable_interrupt(int sig)
{
  /* BGDID set the signal disposition for signal to interrupt  */
  // Used in bigshell: if (signal_enable_interrupt(SIGINT) < 0) goto err;
  if (sigaction(sig, &interrupt_action, NULL) < 0) return -1;   // also could use if (signal(sig, interrupting_signal_handler) == SIG_ERR) return -1;
  return 0;

}


/** ignore a signal
 *
 * @returns 0 on success, -1 on failure
 *
 * does not save old signal disposition
 */
int
signal_ignore(int sig)
{
  /* TODO set the signal disposition for signal back to its old state */  //BG ?????
  // Use in the line in BigShell: if (signal_ignore(SIGINT) < 0) goto err;

// This feels like it could be very wrong (because completely ignoring the above comment)
if (sigaction(sig, &ignore_action, NULL) < 0) return -1;
return 0;
}

/** Restores signal dispositions to what they were when bigshell was invoked
 *
 * @returns 0 on success, -1 on failure
 *
 */
int
signal_restore(void)
{
  /* BGDID restore old actions 
   *
   * e.g. sigaction(SIGNUM, &saved_old_handler, NULL);
   *
   * */
  if (sigaction(SIGTSTP, &old_sigtstp, NULL) < 0) return -1;

  if (sigaction(SIGINT, &old_sigint, NULL) < 0) return -1;

  if (sigaction(SIGTTOU, &old_sigttou, NULL) < 0) return -1;

  return 0;
}
