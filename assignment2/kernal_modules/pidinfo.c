#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/version.h>

#define MAX_PIDS 128
#define PROC_NAME "pidinfo"

int stored_pids[MAX_PIDS];
int num_pids;


ssize_t proc_write(struct file *file, char __user *usr_buf, size_t count, loff_t *pos);

static const struct proc_ops proc_ops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_write = proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release
};

static int __init pidinfo_init(void){
  proc_create(PROC_NAME, 0, NULL, &proc_ops);
  printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
  return 0;
}

static void __exit pidinfo_exit(void){
  remove_proc_entry(PROC_NAME, NULL);
  printk( KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

ssize_t proc_write(struct file *file, char __user *usr_buf, size_t count, loff_t *pos){
  char k_mem[32];
  size_t n= min(count, sizeof(k_mem)-1);
  int pid;

  copy_from_user(k_mem, usr_buf, n);
  k_mem[n] = '/0';
  kstrtoint(k_mem, 10, &pid);
  stored_pids[num_pids++] = pid;
  return count;
}

module_init(pidinfo_init);
module_exit(pidinfo_exit);

MODULE_AUTHOR("Johnathan Aguilar");
MODULE_DESCRIPTION("pidinfo module");
MODULE_LICENSE("GPL"); // important: avoid taint warnings
