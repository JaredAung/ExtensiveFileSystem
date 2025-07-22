#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mfs.h"

int main(void) {
  char buf[256];
  struct fs_stat st;

  /* fs_getcwd */
  assert(fs_getcwd(buf,sizeof buf) && strcmp(buf,"/")==0);

  /* fs_mkdir + fs_isDir/fs_isFile */
  assert(fs_mkdir("/dir1",0)==0);
  assert(fs_isDir("/dir1")==1);
  assert(fs_isFile("/dir1")==0);

  /* fs_setcwd + fs_getcwd */
  assert(fs_setcwd("/dir1")==0);
  assert(fs_getcwd(buf,sizeof buf) && strcmp(buf,"/dir1/")==0);

  /* back up */
  assert(fs_setcwd("/")==0);
  assert(fs_getcwd(buf,sizeof buf) && strcmp(buf,"/")==0);

  /* nested mkdir */
  assert(fs_mkdir("/dir1/sub",0)==0);
  assert(fs_isDir("/dir1/sub")==1);

  /* fs_isFile on missing */
  assert(fs_isFile("/nope")==0);

  /* fs_stat on root */
  assert(fs_stat("/",&st)==0);
  printf("ROOT: size=%u blocks=%u bsize=%u\n",
         (unsigned)st.st_size,
         (unsigned)st.st_blocks,
         (unsigned)st.st_blksize);

  /* fs_stat missing */
  assert(fs_stat("/ghost",&st)==-1);

  puts("All tests have been passed.");
  return 0;
}
