#include "param.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"

//
// This file contains copyin_new() and copyinstr_new(), the
// replacements for copyin and coyinstr in vm.c.
//

static struct stats {
  int ncopyin;
  int ncopyinstr;
} stats;

int
statscopyin(char *buf, int sz) {
  int n;
  n = snprintf(buf, sz, "copyin: %d\n", stats.ncopyin);
  n += snprintf(buf+n, sz, "copyinstr: %d\n", stats.ncopyinstr);
  return n;
}

// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.
// Copy from user to kernel.
int
copyin_new(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
  while(len > 0){
    uint64 va0 = PGROUNDDOWN(srcva);

    // 从用户页表中查找用户虚拟地址对应的物理地址
    uint64 pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;

    // 计算当前页中可以复制的字节数
    uint64 n = PGSIZE - (srcva - va0);
    if(n > len)
      n = len;

    // 从物理地址复制到内核缓冲区
    memmove(dst, (void *)(pa0 + (srcva - va0)), n);

    len -= n;
    dst += n;
    srcva += n;
  }

  stats.ncopyin++;
  return 0;
}


// Copy a null-terminated string from user to kernel.
int
copyinstr_new(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max)
{
  stats.ncopyinstr++;

  while(max > 0){
    uint64 va0 = PGROUNDDOWN(srcva);

    // 查询用户页表
    uint64 pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;

    // 计算当前页剩余空间
    uint64 n = PGSIZE - (srcva - va0);
    if(n > max)
      n = max;

    char *p = (char *)(pa0 + (srcva - va0));

    // 逐字节复制，直到遇到字符串结束符
    for(uint64 i = 0; i < n; i++){
      char c = p[i];
      *dst = c;

      if(c == '\0')
        return 0;

      dst++;
    }

    srcva += n;
    max -= n;
  }

  return -1;
}
