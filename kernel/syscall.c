#include <stdio.h>
#include <libos/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>
#include <sys/uio.h>
#include <libos/syscall.h>
#include <libos/elfutils.h>
#include <libos/mmanutils.h>
#include <libos/spinlock.h>
#include <libos/trace.h>
#include <libos/strings.h>
#include <libos/cwd.h>
#include <sys/utsname.h>
#include <libos/mount.h>
#include "fdtable.h"
#include "eraise.h"

#define DEFAULT_PID (pid_t)1
#define DEFAULT_UID (uid_t)0
#define DEFAULT_GID (gid_t)0

jmp_buf _exit_jmp_buf;

static bool _trace_syscalls;

typedef struct _pair
{
    long num;
    const char* str;
}
pair_t;

static pair_t _pairs[] =
{
    { SYS_read, "SYS_read" },
    { SYS_write, "SYS_write" },
    { SYS_open, "SYS_open" },
    { SYS_close, "SYS_close" },
    { SYS_stat, "SYS_stat" },
    { SYS_fstat, "SYS_fstat" },
    { SYS_lstat, "SYS_lstat" },
    { SYS_poll, "SYS_poll" },
    { SYS_lseek, "SYS_lseek" },
    { SYS_mmap, "SYS_mmap" },
    { SYS_mprotect, "SYS_mprotect" },
    { SYS_munmap, "SYS_munmap" },
    { SYS_brk, "SYS_brk" },
    { SYS_rt_sigaction, "SYS_rt_sigaction" },
    { SYS_rt_sigprocmask, "SYS_rt_sigprocmask" },
    { SYS_rt_sigreturn, "SYS_rt_sigreturn" },
    { SYS_ioctl, "SYS_ioctl" },
    { SYS_pread64, "SYS_pread64" },
    { SYS_pwrite64, "SYS_pwrite64" },
    { SYS_readv, "SYS_readv" },
    { SYS_writev, "SYS_writev" },
    { SYS_access, "SYS_access" },
    { SYS_pipe, "SYS_pipe" },
    { SYS_select, "SYS_select" },
    { SYS_sched_yield, "SYS_sched_yield" },
    { SYS_mremap, "SYS_mremap" },
    { SYS_msync, "SYS_msync" },
    { SYS_mincore, "SYS_mincore" },
    { SYS_madvise, "SYS_madvise" },
    { SYS_shmget, "SYS_shmget" },
    { SYS_shmat, "SYS_shmat" },
    { SYS_shmctl, "SYS_shmctl" },
    { SYS_dup, "SYS_dup" },
    { SYS_dup2, "SYS_dup2" },
    { SYS_pause, "SYS_pause" },
    { SYS_nanosleep, "SYS_nanosleep" },
    { SYS_getitimer, "SYS_getitimer" },
    { SYS_alarm, "SYS_alarm" },
    { SYS_setitimer, "SYS_setitimer" },
    { SYS_getpid, "SYS_getpid" },
    { SYS_sendfile, "SYS_sendfile" },
    { SYS_socket, "SYS_socket" },
    { SYS_connect, "SYS_connect" },
    { SYS_accept, "SYS_accept" },
    { SYS_sendto, "SYS_sendto" },
    { SYS_recvfrom, "SYS_recvfrom" },
    { SYS_sendmsg, "SYS_sendmsg" },
    { SYS_recvmsg, "SYS_recvmsg" },
    { SYS_shutdown, "SYS_shutdown" },
    { SYS_bind, "SYS_bind" },
    { SYS_listen, "SYS_listen" },
    { SYS_getsockname, "SYS_getsockname" },
    { SYS_getpeername, "SYS_getpeername" },
    { SYS_socketpair, "SYS_socketpair" },
    { SYS_setsockopt, "SYS_setsockopt" },
    { SYS_getsockopt, "SYS_getsockopt" },
    { SYS_clone, "SYS_clone" },
    { SYS_fork, "SYS_fork" },
    { SYS_vfork, "SYS_vfork" },
    { SYS_execve, "SYS_execve" },
    { SYS_exit, "SYS_exit" },
    { SYS_wait4, "SYS_wait4" },
    { SYS_kill, "SYS_kill" },
    { SYS_uname, "SYS_uname" },
    { SYS_semget, "SYS_semget" },
    { SYS_semop, "SYS_semop" },
    { SYS_semctl, "SYS_semctl" },
    { SYS_shmdt, "SYS_shmdt" },
    { SYS_msgget, "SYS_msgget" },
    { SYS_msgsnd, "SYS_msgsnd" },
    { SYS_msgrcv, "SYS_msgrcv" },
    { SYS_msgctl, "SYS_msgctl" },
    { SYS_fcntl, "SYS_fcntl" },
    { SYS_flock, "SYS_flock" },
    { SYS_fsync, "SYS_fsync" },
    { SYS_fdatasync, "SYS_fdatasync" },
    { SYS_truncate, "SYS_truncate" },
    { SYS_ftruncate, "SYS_ftruncate" },
    { SYS_getdents, "SYS_getdents" },
    { SYS_getcwd, "SYS_getcwd" },
    { SYS_chdir, "SYS_chdir" },
    { SYS_fchdir, "SYS_fchdir" },
    { SYS_rename, "SYS_rename" },
    { SYS_mkdir, "SYS_mkdir" },
    { SYS_rmdir, "SYS_rmdir" },
    { SYS_creat, "SYS_creat" },
    { SYS_link, "SYS_link" },
    { SYS_unlink, "SYS_unlink" },
    { SYS_symlink, "SYS_symlink" },
    { SYS_readlink, "SYS_readlink" },
    { SYS_chmod, "SYS_chmod" },
    { SYS_fchmod, "SYS_fchmod" },
    { SYS_chown, "SYS_chown" },
    { SYS_fchown, "SYS_fchown" },
    { SYS_lchown, "SYS_lchown" },
    { SYS_umask, "SYS_umask" },
    { SYS_gettimeofday, "SYS_gettimeofday" },
    { SYS_getrlimit, "SYS_getrlimit" },
    { SYS_getrusage, "SYS_getrusage" },
    { SYS_sysinfo, "SYS_sysinfo" },
    { SYS_times, "SYS_times" },
    { SYS_ptrace, "SYS_ptrace" },
    { SYS_getuid, "SYS_getuid" },
    { SYS_syslog, "SYS_syslog" },
    { SYS_getgid, "SYS_getgid" },
    { SYS_setuid, "SYS_setuid" },
    { SYS_setgid, "SYS_setgid" },
    { SYS_geteuid, "SYS_geteuid" },
    { SYS_getegid, "SYS_getegid" },
    { SYS_setpgid, "SYS_setpgid" },
    { SYS_getppid, "SYS_getppid" },
    { SYS_getpgrp, "SYS_getpgrp" },
    { SYS_setsid, "SYS_setsid" },
    { SYS_setreuid, "SYS_setreuid" },
    { SYS_setregid, "SYS_setregid" },
    { SYS_getgroups, "SYS_getgroups" },
    { SYS_setgroups, "SYS_setgroups" },
    { SYS_setresuid, "SYS_setresuid" },
    { SYS_getresuid, "SYS_getresuid" },
    { SYS_setresgid, "SYS_setresgid" },
    { SYS_getresgid, "SYS_getresgid" },
    { SYS_getpgid, "SYS_getpgid" },
    { SYS_setfsuid, "SYS_setfsuid" },
    { SYS_setfsgid, "SYS_setfsgid" },
    { SYS_getsid, "SYS_getsid" },
    { SYS_capget, "SYS_capget" },
    { SYS_capset, "SYS_capset" },
    { SYS_rt_sigpending, "SYS_rt_sigpending" },
    { SYS_rt_sigtimedwait, "SYS_rt_sigtimedwait" },
    { SYS_rt_sigqueueinfo, "SYS_rt_sigqueueinfo" },
    { SYS_rt_sigsuspend, "SYS_rt_sigsuspend" },
    { SYS_sigaltstack, "SYS_sigaltstack" },
    { SYS_utime, "SYS_utime" },
    { SYS_mknod, "SYS_mknod" },
    { SYS_uselib, "SYS_uselib" },
    { SYS_personality, "SYS_personality" },
    { SYS_ustat, "SYS_ustat" },
    { SYS_statfs, "SYS_statfs" },
    { SYS_fstatfs, "SYS_fstatfs" },
    { SYS_sysfs, "SYS_sysfs" },
    { SYS_getpriority, "SYS_getpriority" },
    { SYS_setpriority, "SYS_setpriority" },
    { SYS_sched_setparam, "SYS_sched_setparam" },
    { SYS_sched_getparam, "SYS_sched_getparam" },
    { SYS_sched_setscheduler, "SYS_sched_setscheduler" },
    { SYS_sched_getscheduler, "SYS_sched_getscheduler" },
    { SYS_sched_get_priority_max, "SYS_sched_get_priority_max" },
    { SYS_sched_get_priority_min, "SYS_sched_get_priority_min" },
    { SYS_sched_rr_get_interval, "SYS_sched_rr_get_interval" },
    { SYS_mlock, "SYS_mlock" },
    { SYS_munlock, "SYS_munlock" },
    { SYS_mlockall, "SYS_mlockall" },
    { SYS_munlockall, "SYS_munlockall" },
    { SYS_vhangup, "SYS_vhangup" },
    { SYS_modify_ldt, "SYS_modify_ldt" },
    { SYS_pivot_root, "SYS_pivot_root" },
    { SYS__sysctl, "SYS__sysctl" },
    { SYS_prctl, "SYS_prctl" },
    { SYS_arch_prctl, "SYS_arch_prctl" },
    { SYS_adjtimex, "SYS_adjtimex" },
    { SYS_setrlimit, "SYS_setrlimit" },
    { SYS_chroot, "SYS_chroot" },
    { SYS_sync, "SYS_sync" },
    { SYS_acct, "SYS_acct" },
    { SYS_settimeofday, "SYS_settimeofday" },
    { SYS_mount, "SYS_mount" },
    { SYS_umount2, "SYS_umount2" },
    { SYS_swapon, "SYS_swapon" },
    { SYS_swapoff, "SYS_swapoff" },
    { SYS_reboot, "SYS_reboot" },
    { SYS_sethostname, "SYS_sethostname" },
    { SYS_setdomainname, "SYS_setdomainname" },
    { SYS_iopl, "SYS_iopl" },
    { SYS_ioperm, "SYS_ioperm" },
    { SYS_create_module, "SYS_create_module" },
    { SYS_init_module, "SYS_init_module" },
    { SYS_delete_module, "SYS_delete_module" },
    { SYS_get_kernel_syms, "SYS_get_kernel_syms" },
    { SYS_query_module, "SYS_query_module" },
    { SYS_quotactl, "SYS_quotactl" },
    { SYS_nfsservctl, "SYS_nfsservctl" },
    { SYS_getpmsg, "SYS_getpmsg" },
    { SYS_putpmsg, "SYS_putpmsg" },
    { SYS_afs_syscall, "SYS_afs_syscall" },
    { SYS_tuxcall, "SYS_tuxcall" },
    { SYS_security, "SYS_security" },
    { SYS_gettid, "SYS_gettid" },
    { SYS_readahead, "SYS_readahead" },
    { SYS_setxattr, "SYS_setxattr" },
    { SYS_lsetxattr, "SYS_lsetxattr" },
    { SYS_fsetxattr, "SYS_fsetxattr" },
    { SYS_getxattr, "SYS_getxattr" },
    { SYS_lgetxattr, "SYS_lgetxattr" },
    { SYS_fgetxattr, "SYS_fgetxattr" },
    { SYS_listxattr, "SYS_listxattr" },
    { SYS_llistxattr, "SYS_llistxattr" },
    { SYS_flistxattr, "SYS_flistxattr" },
    { SYS_removexattr, "SYS_removexattr" },
    { SYS_lremovexattr, "SYS_lremovexattr" },
    { SYS_fremovexattr, "SYS_fremovexattr" },
    { SYS_tkill, "SYS_tkill" },
    { SYS_time, "SYS_time" },
    { SYS_futex, "SYS_futex" },
    { SYS_sched_setaffinity, "SYS_sched_setaffinity" },
    { SYS_sched_getaffinity, "SYS_sched_getaffinity" },
    { SYS_set_thread_area, "SYS_set_thread_area" },
    { SYS_io_setup, "SYS_io_setup" },
    { SYS_io_destroy, "SYS_io_destroy" },
    { SYS_io_getevents, "SYS_io_getevents" },
    { SYS_io_submit, "SYS_io_submit" },
    { SYS_io_cancel, "SYS_io_cancel" },
    { SYS_get_thread_area, "SYS_get_thread_area" },
    { SYS_lookup_dcookie, "SYS_lookup_dcookie" },
    { SYS_epoll_create, "SYS_epoll_create" },
    { SYS_epoll_ctl_old, "SYS_epoll_ctl_old" },
    { SYS_epoll_wait_old, "SYS_epoll_wait_old" },
    { SYS_remap_file_pages, "SYS_remap_file_pages" },
    { SYS_getdents64, "SYS_getdents64" },
    { SYS_set_tid_address, "SYS_set_tid_address" },
    { SYS_restart_syscall, "SYS_restart_syscall" },
    { SYS_semtimedop, "SYS_semtimedop" },
    { SYS_fadvise64, "SYS_fadvise64" },
    { SYS_timer_create, "SYS_timer_create" },
    { SYS_timer_settime, "SYS_timer_settime" },
    { SYS_timer_gettime, "SYS_timer_gettime" },
    { SYS_timer_getoverrun, "SYS_timer_getoverrun" },
    { SYS_timer_delete, "SYS_timer_delete" },
    { SYS_clock_settime, "SYS_clock_settime" },
    { SYS_clock_gettime, "SYS_clock_gettime" },
    { SYS_clock_getres, "SYS_clock_getres" },
    { SYS_clock_nanosleep, "SYS_clock_nanosleep" },
    { SYS_exit_group, "SYS_exit_group" },
    { SYS_epoll_wait, "SYS_epoll_wait" },
    { SYS_epoll_ctl, "SYS_epoll_ctl" },
    { SYS_tgkill, "SYS_tgkill" },
    { SYS_utimes, "SYS_utimes" },
    { SYS_vserver, "SYS_vserver" },
    { SYS_mbind, "SYS_mbind" },
    { SYS_set_mempolicy, "SYS_set_mempolicy" },
    { SYS_get_mempolicy, "SYS_get_mempolicy" },
    { SYS_mq_open, "SYS_mq_open" },
    { SYS_mq_unlink, "SYS_mq_unlink" },
    { SYS_mq_timedsend, "SYS_mq_timedsend" },
    { SYS_mq_timedreceive, "SYS_mq_timedreceive" },
    { SYS_mq_notify, "SYS_mq_notify" },
    { SYS_mq_getsetattr, "SYS_mq_getsetattr" },
    { SYS_kexec_load, "SYS_kexec_load" },
    { SYS_waitid, "SYS_waitid" },
    { SYS_add_key, "SYS_add_key" },
    { SYS_request_key, "SYS_request_key" },
    { SYS_keyctl, "SYS_keyctl" },
    { SYS_ioprio_set, "SYS_ioprio_set" },
    { SYS_ioprio_get, "SYS_ioprio_get" },
    { SYS_inotify_init, "SYS_inotify_init" },
    { SYS_inotify_add_watch, "SYS_inotify_add_watch" },
    { SYS_inotify_rm_watch, "SYS_inotify_rm_watch" },
    { SYS_migrate_pages, "SYS_migrate_pages" },
    { SYS_openat, "SYS_openat" },
    { SYS_mkdirat, "SYS_mkdirat" },
    { SYS_mknodat, "SYS_mknodat" },
    { SYS_fchownat, "SYS_fchownat" },
    { SYS_futimesat, "SYS_futimesat" },
    { SYS_newfstatat, "SYS_newfstatat" },
    { SYS_unlinkat, "SYS_unlinkat" },
    { SYS_renameat, "SYS_renameat" },
    { SYS_linkat, "SYS_linkat" },
    { SYS_symlinkat, "SYS_symlinkat" },
    { SYS_readlinkat, "SYS_readlinkat" },
    { SYS_fchmodat, "SYS_fchmodat" },
    { SYS_faccessat, "SYS_faccessat" },
    { SYS_pselect6, "SYS_pselect6" },
    { SYS_ppoll, "SYS_ppoll" },
    { SYS_unshare, "SYS_unshare" },
    { SYS_set_robust_list, "SYS_set_robust_list" },
    { SYS_get_robust_list, "SYS_get_robust_list" },
    { SYS_splice, "SYS_splice" },
    { SYS_tee, "SYS_tee" },
    { SYS_sync_file_range, "SYS_sync_file_range" },
    { SYS_vmsplice, "SYS_vmsplice" },
    { SYS_move_pages, "SYS_move_pages" },
    { SYS_utimensat, "SYS_utimensat" },
    { SYS_epoll_pwait, "SYS_epoll_pwait" },
    { SYS_signalfd, "SYS_signalfd" },
    { SYS_timerfd_create, "SYS_timerfd_create" },
    { SYS_eventfd, "SYS_eventfd" },
    { SYS_fallocate, "SYS_fallocate" },
    { SYS_timerfd_settime, "SYS_timerfd_settime" },
    { SYS_timerfd_gettime, "SYS_timerfd_gettime" },
    { SYS_accept4, "SYS_accept4" },
    { SYS_signalfd4, "SYS_signalfd4" },
    { SYS_eventfd2, "SYS_eventfd2" },
    { SYS_epoll_create1, "SYS_epoll_create1" },
    { SYS_dup3, "SYS_dup3" },
    { SYS_pipe2, "SYS_pipe2" },
    { SYS_inotify_init1, "SYS_inotify_init1" },
    { SYS_preadv, "SYS_preadv" },
    { SYS_pwritev, "SYS_pwritev" },
    { SYS_rt_tgsigqueueinfo, "SYS_rt_tgsigqueueinfo" },
    { SYS_perf_event_open, "SYS_perf_event_open" },
    { SYS_recvmmsg, "SYS_recvmmsg" },
    { SYS_fanotify_init, "SYS_fanotify_init" },
    { SYS_fanotify_mark, "SYS_fanotify_mark" },
    { SYS_prlimit64, "SYS_prlimit64" },
    { SYS_name_to_handle_at, "SYS_name_to_handle_at" },
    { SYS_open_by_handle_at, "SYS_open_by_handle_at" },
    { SYS_clock_adjtime, "SYS_clock_adjtime" },
    { SYS_syncfs, "SYS_syncfs" },
    { SYS_sendmmsg, "SYS_sendmmsg" },
    { SYS_setns, "SYS_setns" },
    { SYS_getcpu, "SYS_getcpu" },
    { SYS_process_vm_readv, "SYS_process_vm_readv" },
    { SYS_process_vm_writev, "SYS_process_vm_writev" },
    { SYS_kcmp, "SYS_kcmp" },
    { SYS_finit_module, "SYS_finit_module" },
    { SYS_sched_setattr, "SYS_sched_setattr" },
    { SYS_sched_getattr, "SYS_sched_getattr" },
    { SYS_renameat2, "SYS_renameat2" },
    { SYS_seccomp, "SYS_seccomp" },
    { SYS_getrandom, "SYS_getrandom" },
    { SYS_memfd_create, "SYS_memfd_create" },
    { SYS_kexec_file_load, "SYS_kexec_file_load" },
    { SYS_bpf, "SYS_bpf" },
    { SYS_execveat, "SYS_execveat" },
    { SYS_userfaultfd, "SYS_userfaultfd" },
    { SYS_membarrier, "SYS_membarrier" },
    { SYS_mlock2, "SYS_mlock2" },
    { SYS_copy_file_range, "SYS_copy_file_range" },
    { SYS_preadv2, "SYS_preadv2" },
    { SYS_pwritev2, "SYS_pwritev2" },
    { SYS_pkey_mprotect, "SYS_pkey_mprotect" },
    { SYS_pkey_alloc, "SYS_pkey_alloc" },
    { SYS_pkey_free, "SYS_pkey_free" },
    { SYS_statx, "SYS_statx" },
    { SYS_io_pgetevents, "SYS_io_pgetevents" },
    { SYS_rseq, "SYS_rseq" },
};

static size_t _n_pairs = sizeof(_pairs) / sizeof(_pairs[0]);

const char* syscall_str(long n)
{
    for (size_t i = 0; i < _n_pairs; i++)
    {
        if (n == _pairs[i].num)
            return _pairs[i].str;
    }

    return "unknown";
}

__attribute__((format(printf, 2, 3)))
static void _strace(long n, const char* fmt, ...)
{
    if (_trace_syscalls)
    {
        fprintf(stderr, "=== %s(", syscall_str(n));

        if (fmt)
        {
            va_list ap;
            va_start(ap, fmt);
            vfprintf(stderr, fmt, ap);
            va_end(ap);
        }

        fprintf(stderr, ")\n");
    }
}

static int _exit_status;

int libos_get_exit_status(void)
{
    return _exit_status;
}

static const void* _original_fs_base;

static void _set_fs_base(const void* p)
{
    __asm__ volatile("wrfsbase %0" ::"r"(p));
}

static void* _get_fs_base(void)
{
    void* p;
    __asm__ volatile("mov %%fs:0, %0" : "=r"(p));
    return p;
}

static long _forward_syscall(long n, long params[6])
{
    extern long oe_syscall(long n, long x1, long x2, long x3, long x4,
        long x5, long x6);
    long x1 = params[0];
    long x2 = params[1];
    long x3 = params[2];
    long x4 = params[3];
    long x5 = params[4];
    long x6 = params[5];

    return oe_syscall(n, x1, x2, x3, x4, x5, x6);
}

typedef struct fd_entry
{
    int fd;
    char path[PATH_MAX];
}
fd_entry_t;

static long _return(long n, long ret)
{
    if (_trace_syscalls)
    {
        fprintf(stderr, "    %s(): return=%ld\n", syscall_str(n), ret);
    }

    return ret;
}

long libos_syscall_creat(const char* pathname, mode_t mode)
{
    long ret = 0;
    int fd;
    char suffix[PATH_MAX];
    libos_fs_t* fs;
    libos_file_t* file;

    ECHECK(libos_mount_resolve(pathname, suffix, &fs));

    ECHECK((*fs->fs_creat)(fs, suffix, mode, &file));

    if ((fd = libos_fdtable_add(LIBOS_FDTABLE_TYPE_FILE, fs, file)) < 0)
    {
        fprintf(stderr, "libos_fdtable_add() failed: %d\n", fd);
    }

    ret = fd;

done:

    return ret;
}

long libos_syscall_open(const char* pathname, int flags, mode_t mode)
{
    long ret = 0;
    int fd;
    char suffix[PATH_MAX];
    libos_fs_t* fs;
    libos_file_t* file;

    ECHECK(libos_mount_resolve(pathname, suffix, &fs));

    ECHECK((*fs->fs_open)(fs, suffix, flags, mode, &file));

    if ((fd = libos_fdtable_add(LIBOS_FDTABLE_TYPE_FILE, fs, file)) < 0)
    {
        fprintf(stderr, "libos_fdtable_add() failed: %d\n", fd);
    }

    ret = fd;

done:

    return ret;
}

long libos_syscall_lseek(int fd, off_t offset, int whence)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));

    ret = ((*fs->fs_lseek)(fs, file, offset, whence));

done:
    return ret;
}

long libos_syscall_close(int fd)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));

    ECHECK((*fs->fs_close)(fs, file));

    ECHECK(libos_fdtable_remove(fd));

done:
    return ret;
}

long libos_syscall_read(int fd, void* buf, size_t count)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));

    ret = (*fs->fs_read)(fs, file, buf, count);

done:
    return ret;
}

long libos_syscall_write(int fd, const void* buf, size_t count)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));

    ret = (*fs->fs_write)(fs, file, buf, count);

done:
    return ret;
}

long libos_syscall_readv(int fd, const struct iovec* iov, int iovcnt)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));

    ret = (*fs->fs_readv)(fs, file, iov, iovcnt);

done:
    return ret;
}

long libos_syscall_writev(int fd, const struct iovec* iov, int iovcnt)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));

    ret = (*fs->fs_writev)(fs, file, iov, iovcnt);

done:
    return ret;
}

long libos_syscall_stat(const char* pathname, struct stat* statbuf)
{
    long ret = 0;
    char suffix[PATH_MAX];
    libos_fs_t* fs;

    ECHECK(libos_mount_resolve(pathname, suffix, &fs));
    ECHECK((*fs->fs_stat)(fs, suffix, statbuf));

done:
    return ret;
}

long libos_syscall_fstat(int fd, struct stat* statbuf)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));

    ret = (*fs->fs_fstat)(fs, file, statbuf);

done:
    return ret;
}

long libos_syscall_mkdir(const char *pathname, mode_t mode)
{
    long ret = 0;
    char suffix[PATH_MAX];
    libos_fs_t* fs;

    ECHECK(libos_mount_resolve(pathname, suffix, &fs));
    ECHECK((*fs->fs_mkdir)(fs, suffix, mode));

done:
    return ret;
}

long libos_syscall_rmdir(const char* pathname)
{
    long ret = 0;
    char suffix[PATH_MAX];
    libos_fs_t* fs;

    ECHECK(libos_mount_resolve(pathname, suffix, &fs));
    ECHECK((*fs->fs_rmdir)(fs, suffix));

done:
    return ret;
}

long libos_syscall_getdents64(int fd, struct dirent* dirp, size_t count)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));

    ret = (*fs->fs_getdents64)(fs, file, dirp, count);

done:
    return ret;
}

long libos_syscall_link(const char* oldpath, const char* newpath)
{
    long ret = 0;
    char old_suffix[PATH_MAX];
    char new_suffix[PATH_MAX];
    libos_fs_t* old_fs;
    libos_fs_t* new_fs;

    ECHECK(libos_mount_resolve(oldpath, old_suffix, &old_fs));
    ECHECK(libos_mount_resolve(newpath, new_suffix, &new_fs));

    if (old_fs != new_fs)
    {
        /* oldpath and newpath are not on the same mounted file system */
        ERAISE(-EXDEV);
    }

    ECHECK((*old_fs->fs_link)(old_fs, old_suffix, new_suffix));

done:
    return ret;
}

long libos_syscall_unlink(const char* pathname)
{
    long ret = 0;
    char suffix[PATH_MAX];
    libos_fs_t* fs;

    ECHECK(libos_mount_resolve(pathname, suffix, &fs));
    ECHECK((*fs->fs_unlink)(fs, suffix));

done:
    return ret;
}

long libos_syscall_access(const char* pathname, int mode)
{
    long ret = 0;
    char suffix[PATH_MAX];
    libos_fs_t* fs;
    bool trace = libos_get_trace();

    ECHECK(libos_mount_resolve(pathname, suffix, &fs));
    libos_set_trace(false);
    ECHECK((*fs->fs_access)(fs, suffix, mode));
    libos_set_trace(trace);

done:
    return ret;
}

long libos_syscall_rename(const char* oldpath, const char* newpath)
{
    long ret = 0;
    char old_suffix[PATH_MAX];
    char new_suffix[PATH_MAX];
    libos_fs_t* old_fs;
    libos_fs_t* new_fs;

    ECHECK(libos_mount_resolve(oldpath, old_suffix, &old_fs));
    ECHECK(libos_mount_resolve(newpath, new_suffix, &new_fs));

    if (old_fs != new_fs)
    {
        /* oldpath and newpath are not on the same mounted file system */
        ERAISE(-EXDEV);
    }

    ECHECK((*old_fs->fs_rename)(old_fs, old_suffix, new_suffix));

done:
    return ret;
}

long libos_syscall_truncate(const char* path, off_t length)
{
    long ret = 0;
    char suffix[PATH_MAX];
    libos_fs_t* fs;

    ECHECK(libos_mount_resolve(path, suffix, &fs));
    ECHECK((*fs->fs_truncate)(fs, suffix, length));

done:
    return ret;
}

long libos_syscall_ftruncate(int fd, off_t length)
{
    long ret = 0;
    libos_fs_t* fs;
    libos_file_t* file;
    const libos_fdtable_type_t type = LIBOS_FDTABLE_TYPE_FILE;

    ECHECK(libos_fdtable_find(fd, type, (void**)&fs, (void**)&file));
    ret = (*fs->fs_ftruncate)(fs, file, length);

done:
    return ret;
}

long libos_syscall_readlink(const char* pathname, char* buf, size_t bufsiz)
{
    long ret = 0;
    char suffix[PATH_MAX];
    libos_fs_t* fs;

    ECHECK(libos_mount_resolve(pathname, suffix, &fs));
    ECHECK((*fs->fs_readlink)(fs, pathname, buf, bufsiz));

done:
    return ret;
}

long libos_syscall_symlink(const char* target, const char* linkpath)
{
    long ret = 0;
    char suffix[PATH_MAX];
    libos_fs_t* fs;

    ECHECK(libos_mount_resolve(linkpath, suffix, &fs));
    ECHECK((*fs->fs_symlink)(fs, target, suffix));

done:
    return ret;
}

static char _cwd[PATH_MAX] = "/";
static libos_spinlock_t _cwd_lock = LIBOS_SPINLOCK_INITIALIZER;

long libos_syscall_chdir(const char* path)
{
    long ret = 0;

    libos_spin_lock(&_cwd_lock);

    if (!path)
        ERAISE(-EINVAL);

    if (LIBOS_STRLCPY(_cwd, path) >= sizeof(_cwd))
        ERAISE(-ERANGE);

done:

    libos_spin_unlock(&_cwd_lock);

    return ret;
}

long libos_syscall_getcwd(char* buf, size_t size)
{
    long ret = 0;

    libos_spin_lock(&_cwd_lock);

    if (!buf)
        ERAISE(-EINVAL);

    if (libos_strlcpy(buf, _cwd, size) >= size)
        ERAISE(-ERANGE);

    ret = (long)buf;

done:

    libos_spin_unlock(&_cwd_lock);

    return ret;
}

long libos_syscall_ret(long ret)
{
    if ((unsigned long)ret > -4096UL)
    {
        errno = (int)-ret;
        return -1;
    }

    return ret;
}

static const char* _fcntl_cmdstr(int cmd)
{
    switch (cmd)
    {
        case F_DUPFD:
            return "F_DUPFD";
        case F_SETFD:
            return "F_SETFD";
        case F_GETFD:
            return "F_GETFD";
        case F_SETFL:
            return "F_SETFL";
        case F_GETFL:
            return "F_GETFL";
        case F_SETOWN:
            return "F_SETOWN";
        case F_GETOWN:
            return "F_GETOWN";
        case F_SETSIG:
            return "F_SETSIG";
        case F_GETSIG:
            return "F_GETSIG";
        case F_SETLK:
            return "F_SETLK";
        case F_GETLK:
            return "F_GETLK";
        case F_SETLKW:
            return "F_SETLKW";
        case F_SETOWN_EX:
            return "F_SETOWN_EX";
        case F_GETOWN_EX:
            return "F_GETOWN_EX";
        case F_GETOWNER_UIDS:
            return "F_GETOWNER_UIDS";
        default:
            return "unknown";
    }
}

long libos_syscall(long n, long params[6])
{
    long x1 = params[0];
    long x2 = params[1];
    long x3 = params[2];
    long x4 = params[3];
    long x5 = params[4];
    long x6 = params[5];

    switch (n)
    {
        case LIBOS_SYS_trace:
        {
            printf("trace: %s\n", (const char*)params[0]);
            return _return(n, 0);
        }
        case LIBOS_SYS_trace_ptr:
        {
            printf("trace: %s: %lX %ld\n",
                (const char*)params[0], params[1], params[1]);
            return _return(n, 0);
        }
        case LIBOS_SYS_dump_stack:
        {
            elf_dump_stack((void*)params[0]);
            return _return(n, 0);
        }
        case LIBOS_SYS_dump_ehdr:
        {
            elf_dump_ehdr((void*)params[0]);
            return _return(n, 0);
        }
        case SYS_read:
        {
            int fd = (int)x1;
            void* buf = (void*)x2;
            size_t count = (size_t)x3;

            _strace(n, "fd=%d buf=%p count=%zu", fd, buf, count);

            if (!libos_is_libos_fd(fd))
                return _return(n, _forward_syscall(n, params));

            return _return(n, libos_syscall_read(fd, buf, count));
        }
        case SYS_write:
        {
            int fd = (int)x1;
            const void* buf = (const void*)x2;
            size_t count = (size_t)x3;

            _strace(n, "fd=%d buf=%p count=%zu", fd, buf, count);

            if (!libos_is_libos_fd(fd))
                return _return(n, _forward_syscall(n, params));

            return _return(n, libos_syscall_write(fd, buf, count));
        }
        case SYS_open:
        {
            const char* path = (const char*)x1;
            int flags = (int)x2;
            mode_t mode = (mode_t)x3;

            _strace(n, "path=%s flags=%d mode=0%o", path, flags, mode);

            return _return(n, libos_syscall_open(path, flags, mode));
        }
        case SYS_close:
        {
            int fd = (int)x1;

            _strace(n, "fd=%d", fd);

            if (!libos_is_libos_fd(fd))
                return _return(n, _forward_syscall(n, params));

            return _return(n, libos_syscall_close(fd));
        }
        case SYS_stat:
        {
            const char* pathname = (const char*)x1;
            struct stat* statbuf = (struct stat*)x2;

            _strace(n, "pathname=%s statbuf=%p", pathname, statbuf);

            return _return(n, libos_syscall_stat(pathname, statbuf));
        }
        case SYS_fstat:
        {
            int fd = (int)x1;
            void* statbuf = (void*)x2;

            _strace(n, "fd=%d statbuf=%p", fd, statbuf);

            return _return(n, libos_syscall_fstat(fd, statbuf));
        }
        case SYS_lstat:
        {
            /* ATTN: remove this! */
            const char* pathname = (const char*)x1;
            struct stat* statbuf = (struct stat*)x2;

            _strace(n, "pathname=%s statbuf=%p", pathname, statbuf);

            return _return(n, libos_syscall_stat(pathname, statbuf));
        }
        case SYS_poll:
        {
            _strace(n, NULL);
            return _return(n, _forward_syscall(n, params));
        }
        case SYS_lseek:
        {
            int fd = (int)x1;
            off_t offset = (off_t)x2;
            int whence = (int)x3;

            _strace(n, "fd=%d offset=%ld whence=%d", fd, offset, whence);

            return _return(n, libos_syscall_lseek(fd, offset, whence));
        }
        case SYS_mmap:
        {
            void* addr = (void*)x1;
            size_t length = (size_t)x2;
            int prot = (int)x3;
            int flags = (int)x4;
            int fd = (int)x5;
            off_t offset = (off_t)x6;

            _strace(n, "addr=%p length=%lu prot=%d flags=%d fd=%d offset=%lu",
                addr, length, prot, flags, fd, offset);

            return _return(n, (long)libos_mmap(
                addr, length, prot, flags, fd, offset));
        }
        case SYS_mprotect:
        {
            const void* addr = (void*)x1;
            const size_t length = (size_t)x2;
            const int prot = (int)x3;

            _strace(n, "addr=%p length=%zu prot=%d", addr, length, prot);

            return _return(n, 0);
        }
        case SYS_munmap:
        {
            void* addr = (void*)x1;
            size_t length = (size_t)x2;

            _strace(n, "addr=%p length=%zu", addr, length);

            return _return(n, (long)libos_munmap(addr, length));
        }
        case SYS_brk:
        {
            _strace(n, NULL);
            return _return(n, _forward_syscall(n, params));
        }
        case SYS_rt_sigaction:
            break;
        case SYS_rt_sigprocmask:
        {
            /* ATTN: ignored for now */
            return _return(n, 0);
        }
        case SYS_rt_sigreturn:
            break;
        case SYS_ioctl:
        {
            int fd = (int)x1;
            /* Note: 0x5413 is TIOCGWINSZ  */
            unsigned long request = (unsigned long)x2;

            _strace(n, "fd=%d request=%lx", fd, request);

            return _return(n, _forward_syscall(n, params));
        }
        case SYS_pread64:
            break;
        case SYS_pwrite64:
            break;
        case SYS_readv:
        {
            int fd = (int)x1;
            const struct iovec* iov = (const struct iovec*)x2;
            int iovcnt = (int)x3;

            _strace(n, "fd=%d iov=%p iovcnt=%d", fd, iov, iovcnt);

            if (!libos_is_libos_fd(fd))
                return _return(n, _forward_syscall(n, params));

            return _return(n, libos_syscall_readv(fd, iov, iovcnt));
        }
        case SYS_writev:
        {
            int fd = (int)x1;
            const struct iovec* iov = (const struct iovec*)x2;
            int iovcnt = (int)x3;

            _strace(n, "fd=%d iov=%p iovcnt=%d", fd, iov, iovcnt);

            if (!libos_is_libos_fd(fd))
                return _return(n, _forward_syscall(n, params));

            return _return(n, libos_syscall_writev(fd, iov, iovcnt));
        }
        case SYS_access:
        {
            const char* pathname = (const char*)x1;
            int mode = (int)x2;

            _strace(n, "pathname=%s mode=%d", pathname, mode);

            return _return(n, libos_syscall_access(pathname, mode));
        }
        case SYS_pipe:
            break;
        case SYS_select:
        {
            int nfds = (int)x1;
            fd_set* rfds = (fd_set*)x2;
            fd_set* wfds = (fd_set*)x3;
            fd_set* efds = (fd_set*)x4;
            struct timeval* timeout = (struct timeval*)x5;

            _strace(n, "nfds=%d rfds=%p wfds=%p xfds=%p timeout=%p",
                nfds, rfds, wfds, efds, timeout);

            return _return(n, _forward_syscall(n, params));
        }
        case SYS_sched_yield:
            break;
        case SYS_mremap:
            /* ATTN: hook up implementation */
            break;
        case SYS_msync:
            /* ATTN: hook up implementation */
            break;
        case SYS_mincore:
            /* ATTN: hook up implementation */
            break;
        case SYS_madvise:
            /* ATTN: hook up implementation */
            break;
        case SYS_shmget:
            break;
        case SYS_shmat:
            break;
        case SYS_shmctl:
            break;
        case SYS_dup:
            break;
        case SYS_dup2:
            break;
        case SYS_pause:
            break;
        case SYS_nanosleep:
        {
            const struct timespec* req = (const struct timespec*)x1;
            struct timespec* rem = (struct timespec*)x2;

            _strace(n, "req=%p rem=%p", req, rem);

            return _return(n, _forward_syscall(n, params));
        }
        case SYS_getitimer:
            break;
        case SYS_alarm:
            break;
        case SYS_setitimer:
            break;
        case SYS_getpid:
        {
            _strace(n, NULL);
            return _return(n, DEFAULT_PID);
        }
        case SYS_clone:
            break;
        case SYS_fork:
            break;
        case SYS_vfork:
            break;
        case SYS_execve:
            break;
        case SYS_exit:
        {
            const int status = (int)x1;

            /* restore original fs base, else stack smashing will be detected */
            _set_fs_base(_original_fs_base);

            _strace(n, "status=%d", status);

            _exit_status = status;
            longjmp(_exit_jmp_buf, 1);

            /* Unreachable! */
            assert("unreachable" == NULL);
            break;
        }
        case SYS_wait4:
            break;
        case SYS_kill:
            break;
        case SYS_uname:
        {
            struct utsname* buf = (struct utsname*)x1;

            LIBOS_STRLCPY(buf->sysname, "OpenLibos");
            LIBOS_STRLCPY(buf->nodename, "libos");
            LIBOS_STRLCPY(buf->release, "1.0.0");
            LIBOS_STRLCPY(buf->version, "Libos 1.0.0");
            LIBOS_STRLCPY(buf->machine, "x86_64");

            return _return(n, 0);
        }
        case SYS_semget:
            break;
        case SYS_semop:
            break;
        case SYS_semctl:
            break;
        case SYS_shmdt:
            break;
        case SYS_msgget:
            break;
        case SYS_msgsnd:
            break;
        case SYS_msgrcv:
            break;
        case SYS_msgctl:
            break;
        case SYS_fcntl:
        {
            int fd = (int)x1;
            int cmd = (int)x2;
            long arg = (long)x3;

            const char* cmdstr = _fcntl_cmdstr(cmd);
            _strace(n, "fd=%d cmd=%d(%s) arg=%ld", fd, cmd, cmdstr, arg);

            if (cmd == F_SETFD && arg == FD_CLOEXEC)
            {
                /* Ignore FD_CLOEXEC since fork/exec not supported */
                return 0;
            }

            /* unhandled */
            break;
        }
        case SYS_flock:
            break;
        case SYS_fsync:
            break;
        case SYS_fdatasync:
            break;
        case SYS_truncate:
        {
            const char* path = (const char*)x1;
            off_t length = (off_t)x2;

            _strace(n, "path=%s length=%ld", path, length);

            return _return(n, libos_syscall_truncate(path, length));
        }
        case SYS_ftruncate:
        {
            int fd = (int)x1;
            off_t length = (off_t)x2;

            _strace(n, "fd=%d length=%ld", fd, length);

            return _return(n, libos_syscall_ftruncate(fd, length));
        }
        case SYS_getdents:
            break;
        case SYS_getcwd:
        {
            char* buf = (char*)x1;
            size_t size = (size_t)x2;

            _strace(n, "buf=%p size=%zu", buf, size);

            return _return(n, libos_syscall_getcwd(buf, size));
        }
        case SYS_chdir:
        {
            const char* path = (const char*)x1;

            _strace(n, "path=%s", path);

            return _return(n, libos_syscall_chdir(path));
        }
        case SYS_fchdir:
            break;
        case SYS_rename:
        {
            const char* oldpath = (const char*)x1;
            const char* newpath = (const char*)x2;

            _strace(n, "oldpath=%s newpath=%s", oldpath, newpath);

            return _return(n, libos_syscall_rename(oldpath, newpath));
        }
        case SYS_mkdir:
        {
            const char* pathname = (const char*)x1;
            mode_t mode = (mode_t)x1;

            _strace(n, "pathname=%s mode=%u", pathname, mode);

            return _return(n, libos_syscall_mkdir(pathname, mode));
        }
        case SYS_rmdir:
        {
            const char* pathname = (const char*)x1;

            _strace(n, "pathname=%s", pathname);

            return _return(n, libos_syscall_rmdir(pathname));
        }
        case SYS_creat:
        {
            const char* pathname = (const char*)x1;
            mode_t mode = (mode_t)x2;

            _strace(n, "pathname=%s mode=%x", pathname, mode);

            return _return(n, libos_syscall_creat(pathname, mode));
        }
        case SYS_link:
        {
            const char* oldpath = (const char*)x1;
            const char* newpath = (const char*)x2;

            _strace(n, "oldpath=%s newpath=%s", oldpath, newpath);

            return _return(n, libos_syscall_link(oldpath, newpath));
        }
        case SYS_unlink:
        {
            const char* pathname = (const char*)x1;

            _strace(n, "pathname=%s", pathname);

            return _return(n, libos_syscall_unlink(pathname));
        }
        case SYS_symlink:
        {
            const char* target = (const char*)x1;
            const char* linkpath = (const char*)x2;

            _strace(n, "target=%s linkpath=%s", target, linkpath);

            return _return(n, libos_syscall_symlink(target, linkpath));
        }
        case SYS_readlink:
        {
            const char* pathname = (const char*)x1;
            char* buf = (char*)x2;
            size_t bufsiz = (size_t)x3;

            _strace(n, "pathname=%s buf=%p bufsiz=%zu", pathname, buf, bufsiz);

            return _return(n, libos_syscall_readlink(pathname, buf, bufsiz));
        }
        case SYS_chmod:
            break;
        case SYS_fchmod:
            break;
        case SYS_chown:
            break;
        case SYS_fchown:
            break;
        case SYS_lchown:
            break;
        case SYS_umask:
            break;
        case SYS_gettimeofday:
        {
            struct timeval* tv = (struct timeval*)x1;
            struct timezone* tz = (void*)x2;

            _strace(n, "tv=%p tz=%p", tv, tz);

            return _return(n, _forward_syscall(n, params));
        }
        case SYS_getrlimit:
            break;
        case SYS_getrusage:
            break;
        case SYS_sysinfo:
            break;
        case SYS_times:
            break;
        case SYS_ptrace:
            break;
        case SYS_getuid:
        {
            _strace(n, NULL);
            return _return(n, DEFAULT_UID);
        }
        case SYS_syslog:
        {
            /* Ignore syslog for now */
            return _return(n, 0);
        }
        case SYS_getgid:
        {
            _strace(n, NULL);
            return _return(n, DEFAULT_GID);
        }
        case SYS_setuid:
            break;
        case SYS_setgid:
            break;
        case SYS_geteuid:
            break;
        case SYS_getegid:
            break;
        case SYS_setpgid:
            break;
        case SYS_getppid:
            break;
        case SYS_getpgrp:
            break;
        case SYS_setsid:
            break;
        case SYS_setreuid:
            break;
        case SYS_setregid:
            break;
        case SYS_getgroups:
            break;
        case SYS_setgroups:
            break;
        case SYS_setresuid:
            break;
        case SYS_getresuid:
            break;
        case SYS_setresgid:
            break;
        case SYS_getresgid:
            break;
        case SYS_getpgid:
            break;
        case SYS_setfsuid:
            break;
        case SYS_setfsgid:
            break;
        case SYS_getsid:
            break;
        case SYS_capget:
            break;
        case SYS_capset:
            break;
        case SYS_rt_sigpending:
            break;
        case SYS_rt_sigtimedwait:
            break;
        case SYS_rt_sigqueueinfo:
            break;
        case SYS_rt_sigsuspend:
            break;
        case SYS_sigaltstack:
            break;
        case SYS_utime:
            break;
        case SYS_mknod:
            break;
        case SYS_uselib:
            break;
        case SYS_personality:
            break;
        case SYS_ustat:
            break;
        case SYS_statfs:
            break;
        case SYS_fstatfs:
            break;
        case SYS_sysfs:
            break;
        case SYS_getpriority:
            break;
        case SYS_setpriority:
            break;
        case SYS_sched_setparam:
            break;
        case SYS_sched_getparam:
            break;
        case SYS_sched_setscheduler:
            break;
        case SYS_sched_getscheduler:
            break;
        case SYS_sched_get_priority_max:
            break;
        case SYS_sched_get_priority_min:
            break;
        case SYS_sched_rr_get_interval:
            break;
        case SYS_mlock:
            break;
        case SYS_munlock:
            break;
        case SYS_mlockall:
            break;
        case SYS_munlockall:
            break;
        case SYS_vhangup:
            break;
        case SYS_modify_ldt:
            break;
        case SYS_pivot_root:
            break;
        case SYS__sysctl:
            break;
        case SYS_prctl:
            break;
        case SYS_arch_prctl:
            break;
        case SYS_adjtimex:
            break;
        case SYS_setrlimit:
            break;
        case SYS_chroot:
            break;
        case SYS_sync:
            break;
        case SYS_acct:
            break;
        case SYS_settimeofday:
            break;
        case SYS_mount:
            break;
        case SYS_umount2:
            break;
        case SYS_swapon:
            break;
        case SYS_swapoff:
            break;
        case SYS_reboot:
            break;
        case SYS_sethostname:
        {
            const char* name= (const char*)x1;
            size_t len = (size_t)x2;

            _strace(n, "name=%s len=%zu", name, len);

            return 0;
            return _return(n, _forward_syscall(n, params));
        }
        case SYS_setdomainname:
            break;
        case SYS_iopl:
            break;
        case SYS_ioperm:
            break;
        case SYS_create_module:
            break;
        case SYS_init_module:
            break;
        case SYS_delete_module:
            break;
        case SYS_get_kernel_syms:
            break;
        case SYS_query_module:
            break;
        case SYS_quotactl:
            break;
        case SYS_nfsservctl:
            break;
        case SYS_getpmsg:
            break;
        case SYS_putpmsg:
            break;
        case SYS_afs_syscall:
            break;
        case SYS_tuxcall:
            break;
        case SYS_security:
            break;
        case SYS_gettid:
            break;
        case SYS_readahead:
            break;
        case SYS_setxattr:
            break;
        case SYS_lsetxattr:
            break;
        case SYS_fsetxattr:
            break;
        case SYS_getxattr:
            break;
        case SYS_lgetxattr:
            break;
        case SYS_fgetxattr:
            break;
        case SYS_listxattr:
            break;
        case SYS_llistxattr:
            break;
        case SYS_flistxattr:
            break;
        case SYS_removexattr:
            break;
        case SYS_lremovexattr:
            break;
        case SYS_fremovexattr:
            break;
        case SYS_tkill:
            break;
        case SYS_time:
            break;
        case SYS_futex:
            break;
        case SYS_sched_setaffinity:
            break;
        case SYS_sched_getaffinity:
            break;
        case SYS_set_thread_area:
        {
            const void* tp = (void*)params[0];

            _strace(n, "tp=%p", tp);

            if (!_original_fs_base)
                _original_fs_base = _get_fs_base();

            _set_fs_base(tp);

            return _return(n, 0);
        }
        case SYS_io_setup:
            break;
        case SYS_io_destroy:
            break;
        case SYS_io_getevents:
            break;
        case SYS_io_submit:
            break;
        case SYS_io_cancel:
            break;
        case SYS_get_thread_area:
            break;
        case SYS_lookup_dcookie:
            break;
        case SYS_epoll_create:
            break;
        case SYS_epoll_ctl_old:
            break;
        case SYS_epoll_wait_old:
            break;
        case SYS_remap_file_pages:
            break;
        case SYS_getdents64:
        {
            unsigned int fd = (unsigned int)x1;
            struct dirent* dirp = (struct dirent*)x2;
            unsigned int count = (unsigned int)x3;

            _strace(n, "fd=%d dirp=%p count=%u", fd, dirp, count);

            return _return(n, libos_syscall_getdents64((int)fd, dirp, count));
        }
        case SYS_set_tid_address:
        {
            const void* tidptr = (const void*)params[0];

            /* ATTN: unused */

            _strace(n, "tidptr=%p", tidptr);

            return _return(n, 0);
        }
        case SYS_restart_syscall:
            break;
        case SYS_semtimedop:
            break;
        case SYS_fadvise64:
            break;
        case SYS_timer_create:
            break;
        case SYS_timer_settime:
            break;
        case SYS_timer_gettime:
            break;
        case SYS_timer_getoverrun:
            break;
        case SYS_timer_delete:
            break;
        case SYS_clock_settime:
            break;
        case SYS_clock_gettime:
        {
            _strace(n, NULL);
            return _return(n, _forward_syscall(n, params));
        }
        case SYS_clock_getres:
            break;
        case SYS_clock_nanosleep:
            break;
        case SYS_exit_group:
        {
            int status = (int)x1;

            _strace(n, "status=%d", status);

            return 0;
        }
        case SYS_epoll_wait:
            break;
        case SYS_epoll_ctl:
            break;
        case SYS_tgkill:
            break;
        case SYS_utimes:
            break;
        case SYS_vserver:
            break;
        case SYS_mbind:
            break;
        case SYS_set_mempolicy:
            break;
        case SYS_get_mempolicy:
            break;
        case SYS_mq_open:
            break;
        case SYS_mq_unlink:
            break;
        case SYS_mq_timedsend:
            break;
        case SYS_mq_timedreceive:
            break;
        case SYS_mq_notify:
            break;
        case SYS_mq_getsetattr:
            break;
        case SYS_kexec_load:
            break;
        case SYS_waitid:
            break;
        case SYS_add_key:
            break;
        case SYS_request_key:
            break;
        case SYS_keyctl:
            break;
        case SYS_ioprio_set:
            break;
        case SYS_ioprio_get:
            break;
        case SYS_inotify_init:
            break;
        case SYS_inotify_add_watch:
            break;
        case SYS_inotify_rm_watch:
            break;
        case SYS_migrate_pages:
            break;
        case SYS_openat:
            break;
        case SYS_mkdirat:
            break;
        case SYS_mknodat:
            break;
        case SYS_fchownat:
            break;
        case SYS_futimesat:
            break;
        case SYS_newfstatat:
            break;
        case SYS_unlinkat:
            break;
        case SYS_renameat:
            break;
        case SYS_linkat:
            break;
        case SYS_symlinkat:
            break;
        case SYS_readlinkat:
            break;
        case SYS_fchmodat:
            break;
        case SYS_faccessat:
            break;
        case SYS_pselect6:
            break;
        case SYS_ppoll:
            break;
        case SYS_unshare:
            break;
        case SYS_set_robust_list:
            break;
        case SYS_get_robust_list:
            break;
        case SYS_splice:
            break;
        case SYS_tee:
            break;
        case SYS_sync_file_range:
            break;
        case SYS_vmsplice:
            break;
        case SYS_move_pages:
            break;
        case SYS_utimensat:
            break;
        case SYS_epoll_pwait:
            break;
        case SYS_signalfd:
            break;
        case SYS_timerfd_create:
            break;
        case SYS_eventfd:
            break;
        case SYS_fallocate:
            break;
        case SYS_timerfd_settime:
            break;
        case SYS_timerfd_gettime:
            break;
        case SYS_accept4:
            break;
        case SYS_signalfd4:
            break;
        case SYS_eventfd2:
            break;
        case SYS_epoll_create1:
            break;
        case SYS_dup3:
            break;
        case SYS_pipe2:
            break;
        case SYS_inotify_init1:
            break;
        case SYS_preadv:
            break;
        case SYS_pwritev:
            break;
        case SYS_rt_tgsigqueueinfo:
            break;
        case SYS_perf_event_open:
            break;
        case SYS_recvmmsg:
            break;
        case SYS_fanotify_init:
            break;
        case SYS_fanotify_mark:
            break;
        case SYS_prlimit64:
            break;
        case SYS_name_to_handle_at:
            break;
        case SYS_open_by_handle_at:
            break;
        case SYS_clock_adjtime:
            break;
        case SYS_syncfs:
            break;
        case SYS_sendmmsg:
            break;
        case SYS_setns:
            break;
        case SYS_getcpu:
            break;
        case SYS_process_vm_readv:
            break;
        case SYS_process_vm_writev:
            break;
        case SYS_kcmp:
            break;
        case SYS_finit_module:
            break;
        case SYS_sched_setattr:
            break;
        case SYS_sched_getattr:
            break;
        case SYS_renameat2:
            break;
        case SYS_seccomp:
            break;
        case SYS_getrandom:
            break;
        case SYS_memfd_create:
            break;
        case SYS_kexec_file_load:
            break;
        case SYS_bpf:
            break;
        case SYS_execveat:
            break;
        case SYS_userfaultfd:
            break;
        case SYS_membarrier:
            break;
        case SYS_mlock2:
            break;
        case SYS_copy_file_range:
            break;
        case SYS_preadv2:
            break;
        case SYS_pwritev2:
            break;
        case SYS_pkey_mprotect:
            break;
        case SYS_pkey_alloc:
            break;
        case SYS_pkey_free:
            break;
        case SYS_statx:
            break;
        case SYS_io_pgetevents:
            break;
        case SYS_rseq:
            break;
        /* forward network syscdalls to OE */
        case SYS_sendfile:
        case SYS_socket:
        case SYS_connect:
        case SYS_accept:
        case SYS_sendto:
        case SYS_recvfrom:
        case SYS_sendmsg:
        case SYS_recvmsg:
        case SYS_shutdown:
        case SYS_bind:
        case SYS_listen:
        case SYS_getsockname:
        case SYS_getpeername:
        case SYS_socketpair:
        case SYS_setsockopt:
        case SYS_getsockopt:
        {
            _strace(n, "forwarded");
            return _return(n, _forward_syscall(n, params));
        }
        default:
        {
            fprintf(stderr, "********** unknown: %s()\n", syscall_str(n));
            abort();
        }
    }

    fprintf(stderr, "********** unhandled: %s()\n", syscall_str(n));
    abort();

    return 0;
}

int libos_set_exit_jump(void)
{
    return setjmp(_exit_jmp_buf);
}

void libos_trace_syscalls(bool flag)
{
    _trace_syscalls = flag;
}
