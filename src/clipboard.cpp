/**
 * @file   clipboard.cpp
 *
 * @brief clipboard implentation
 *
 *
 */
#include "clipboard.hpp"
#include "ysid.hpp"
#include "fmtstr.hpp"

#include <cstdio>
#include <cstring>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#define sleep(x) Sleep((x) * 1000)
#else
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace ysid {
#ifdef WIN32
int clipboard_copy(const string &text) {
  if (OpenClipboard(NULL)) {
    EmptyClipboard();
    HGLOBAL hClipboardData = GlobalAlloc(
        GMEM_MOVEABLE | GMEM_SHARE, text.size() + 1);
    char * pchData = (char*)GlobalLock(hClipboardData);
    strcpy(pchData, text.c_str());
    GlobalUnlock(hClipboardData);
    SetClipboardData(CF_TEXT, hClipboardData);
    CloseClipboard();
    return 0;
  }
  return -1;
}
#else
int clipboard_copy(const string &text) {
  int fds[2];
  if (pipe(fds) == -1) {
    perror("pipe");
    exit(1);
  }
  pid_t pid = fork();
  if (pid == 0) {
    close(fds[1]);
    close(0);
    if (dup2(fds[0], 0) == -1) {
      fmterrlnt("dup2(0, %s) failed: %s", fds[0], strerror(errno));
      exit(1);
    }
    execlp("xsel", "xsel", "-bi", NULL);
    perror("execlp xsel failed");
    exit(1);
  }
  close(fds[0]);
  write(fds[1], (void*)text.data(), text.size());
  close(fds[1]);
  int status;
  waitpid(pid, &status, 0);
  return 0;
}
#endif

} // namespace ysid
