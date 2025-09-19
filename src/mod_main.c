#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <linux/err.h>
#include "./komm/komm.h"
#include "./control/control.h"
#include "./redirect/redirect.h"
#include "./feature/feature.h"
#include "./ptrace/ptrace_hide.h"
#include "symbols.h"

KPM_NAME("mem_crc");
KPM_VERSION("0.0.1");
KPM_AUTHOR("yuuki");
KPM_DESCRIPTION("original intention is to bypass the memory crc check");

static long mod_init(const char *args, const char *event, void *__user reserved){
    pr_info("[yuuki] Initializing...\n");
    if(init_symbols()) {
        pr_info("[yuuki] init_symbols failed\n");
        goto exit;
    }

    ioctl_init();
    feature_init();
    redirect_init();
    syscall_init();
    ptrace_hide_init();

    exit:
    return 0;
}

static long mod_control0(const char *args, char *__user out_msg, int outlen) {
    pr_info("[yuuki] kpm hello control0, args: %s\n", args);

    return 0;
}

static long mod_exit(void *__user reserved) {
    pr_info("[yuuki] mod_exit, uninstalled hook.\n");
    ioctl_exit();
    feature_exit();
    redirect_exit();
    syscall_exit();
    ptrace_hide_exit();
    return 0;
}

KPM_INIT(mod_init);
KPM_CTL0(mod_control0);
KPM_EXIT(mod_exit);
