#include <hook.h>
#include <asm/current.h>
#include <taskext.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/fs.h>
#include <linux/cred.h>
#include "redirect.h"
#include "shared_data.h"

static hook_err_t hook_err = HOOK_NOT_HOOK;
static do_filp_open_func_t backup_do_filp_open;

static inline void set_priv_selinx_allow(struct task_struct* task, int val) {
    struct task_ext* ext = get_task_ext(task);
    if (likely(task_ext_valid(ext))) {
        ext->priv_sel_allow = val;
        dsb(ish);
    }
}

static inline uid_t get_current_uid(void) {
    struct cred* cred = *(struct cred**)((uintptr_t)current + task_struct_offset.cred_offset);
    uid_t current_uid = *(uid_t*)((uintptr_t)cred + cred_offset.uid_offset);
    return current_uid;
}

// 使用cat命令读取文件，安卓会有content provider 的代理机制 或 Android 系统的
// “媒体读取委托”机制，需要通过过滤com.android.providers.media.module的uid实现重定向，而非app进程自身的uid
static struct file *replace_do_filp_open(int dfd, struct filename *pathname, const struct open_flags *op) {
    struct file *filp;
    char buf[PATH_MAX];
    char *curr_path;
    struct shared_data_entry entry;
    unsigned int map_count;
    unsigned int i;
    uid_t current_uid;

    filp = backup_do_filp_open(dfd, pathname, op);

    if (unlikely(IS_ERR(filp))) {
        return filp;
    }

    memset(buf, 0, PATH_MAX);
    curr_path = d_path(&filp->f_path, buf, PATH_MAX);


    if (IS_ERR(curr_path)) {
        return filp;
    }

    current_uid = get_current_uid();

    map_count = shared_data_get_count_by_type(SHARED_DATA_TYPE_PATH_MAPPING);

    for (i = 0; i < map_count; i++) {
        unsigned int global_index;
        int ret;

        ret = shared_data_get_type_entry_info(SHARED_DATA_TYPE_PATH_MAPPING, i,
                                              &global_index, &entry);

        if (ret != SHARED_DATA_SUCCESS) {
            continue;
        }

        if (strncmp(curr_path, entry.data.mapping.src_path,
                    strlen(entry.data.mapping.src_path)) == 0) {

            if (entry.data.mapping.uid == 0 || entry.data.mapping.uid == current_uid) {
                struct filename *new_pathname;
                struct file *redirect_filp;

                pr_info("[yuuki] Redirecting path from %s to %s (uid: %u, current: %u)\n",
                        entry.data.mapping.src_path,
                        entry.data.mapping.dst_path,
                        entry.data.mapping.uid,
                        current_uid);

                fput(filp);

                new_pathname = kf_vmalloc(sizeof(struct filename));
                if (!new_pathname) {
                    pr_err("[yuuki] Failed to allocate memory for new pathname\n");
                    return ERR_PTR(-ENOMEM);
                }

                new_pathname->name = entry.data.mapping.dst_path;
                set_priv_selinx_allow(current, true);
                redirect_filp = backup_do_filp_open(dfd, new_pathname, op);
                set_priv_selinx_allow(current, false);
                kf_vfree(new_pathname);

                return redirect_filp;
            }
        }
    }

    return filp;
}

static inline bool hook_do_filp_open(void) {
    if (!original_do_filp_open) {
        hook_err = HOOK_BAD_ADDRESS;
        pr_err("[yuuki] no symbol: do_filp_open\n");
        return false;
    }

    hook_err = hook((void *)original_do_filp_open,
                    (void *)replace_do_filp_open,
                    (void **)&backup_do_filp_open);

    if (hook_err != HOOK_NO_ERR) {
        pr_err("[yuuki] hook do_filp_open failed, address: %llx, error: %d\n",
               (unsigned long long)original_do_filp_open, hook_err);
        return false;
    }

    return true;
}

static inline bool install_hook(void) {
    if (hook_err == HOOK_NO_ERR) {
        pr_info("[yuuki] redirect hook already installed, skipping...\n");
        return true;
    }

    if (hook_do_filp_open()) {
        pr_info("[yuuki] redirect hook installed successfully\n");
        return true;
    }

    pr_err("[yuuki] redirect hook installation failed\n");
    return false;
}

static inline bool uninstall_hook(void) {
    if (hook_err != HOOK_NO_ERR) {
        pr_info("[yuuki] redirect hook not installed, skipping uninstall...\n");
        return true;
    }

    unhook((void *)original_do_filp_open);
    hook_err = HOOK_NOT_HOOK;
    pr_info("[yuuki] redirect hook uninstalled successfully\n");
    return true;
}

static inline bool control_hook(bool enable) {
    return enable ? install_hook() : uninstall_hook();
}

long redirect_init(void) {
    return control_hook(true) ? 0 : -1;
}

long redirect_exit(void) {
    return control_hook(false) ? 0 : -1;
}