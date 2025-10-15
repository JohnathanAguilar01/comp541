#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/version.h>

#define MAX_PIDS 128
#define PROC_NAME "pidinfo"

static int stored_pids[MAX_PIDS];
static int num_pids;
static struct proc_dir_entry *proc_entry;

/* ---- seq_file show ---- */
static int pidinfo_show(struct seq_file *m, void *v)
{
    int i;

    if (num_pids == 0) {
        seq_puts(m, "No PIDs written yet.\n");
        return 0;
    }

    for (i = 0; i < num_pids; i++) {
        int pid = stored_pids[i];
        struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);

        if (!task) {
            seq_printf(m, "PID %d: <not found>\n", pid);
            continue;
        }

        /* Print a few useful fields */
        seq_printf(m,
                   "command = [%s]\tpid = [%d]\tstate = [%u]\n",
                   task->comm,
                   task->pid,
                   READ_ONCE(task->__state));
    }

    return 0;
}

/* ---- /proc open ---- */
static int proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, pidinfo_show, NULL);
}

/* ---- /proc write ---- */
static ssize_t proc_write(struct file *file, const char __user *usr_buf,
                          size_t count, loff_t *pos)
{
	char k_mem[32];
	size_t n = min(count, sizeof(k_mem) - 1);
	int ret, pid;

	/* copy and NUL-terminate */
	if (copy_from_user(k_mem, usr_buf, n))
		return -EFAULT;
	k_mem[n] = '\0';

	/* optional: trim trailing newline */
	if (n > 0 && k_mem[n - 1] == '\n')
		k_mem[n - 1] = '\0';

	ret = kstrtoint(k_mem, 10, &pid);
	if (ret)
		return ret;

	if (num_pids >= MAX_PIDS)
		return -ENOSPC;

	stored_pids[num_pids++] = pid;
	return count;
}

static const struct proc_ops proc_ops = {
	.proc_open    = proc_open,
	.proc_read    = seq_read,
	.proc_write   = proc_write,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

static int __init pidinfo_init(void)
{
	num_pids = 0;
	proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_ops);
	if (!proc_entry)
		return -ENOMEM;
	pr_info("/proc/%s created\n", PROC_NAME);
	return 0;
}

static void __exit pidinfo_exit(void)
{
	if (proc_entry)
		proc_remove(proc_entry);
	pr_info("/proc/%s removed\n", PROC_NAME);
}

module_init(pidinfo_init);
module_exit(pidinfo_exit);

MODULE_AUTHOR("Johnathan Aguilar");
MODULE_DESCRIPTION("pidinfo module");
MODULE_LICENSE("GPL");
