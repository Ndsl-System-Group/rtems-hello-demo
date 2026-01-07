#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>

#include <rtems/libio.h>
#include <rtems/media.h>
#include <rtems/dosfs.h>
#include <rtems/fsmount.h>
#include <rtems/shell.h>


#define MOUNT_POINT "/mnt"


const rtems_fstab_entry fs_table[] = {
    {.source = "/dev/mmcsd0",
     .target = "/mnt",
     .type = "dosfs",
     .options = RTEMS_FILESYSTEM_READ_WRITE}};


// login_check 回调示例，允许所有登录
bool no_login_check(const char *user, const char *pass)
{
    (void)user;
    (void)pass;
    return true; // 总是通过验证
}

rtems_task Init(rtems_task_argument arg)
{
    printf("Starting RTEMS disk mount test...\n");

    // 挂载文件系统
    int rc = rtems_fsmount(fs_table, sizeof(fs_table) / sizeof(fs_table[0]), NULL);
    if (rc != 0)
    {
        perror("Mount failed");
        exit(1);
    }
    printf("Disk mounted successfully at %s\n", MOUNT_POINT);

    // 打开挂载目录读取文件
    DIR *dir = opendir(MOUNT_POINT);
    if (!dir)
    {
        perror("opendir failed");
        exit(1);
    }
    printf("Listing files on SD card:\n");

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        printf(" %s\n", ent->d_name);
    }

    closedir(dir);


    // 启动 Shell
    rtems_status_code status;

    status = rtems_shell_init(
        "SHLL",         // shell 任务名
        4096,           // 任务堆栈大小（字节，适当调整大小）
        100,            // shell 任务优先级 (根据应用调整)
        "/dev/console", // 绑定的设备名，一般用控制台设备
        true,           // forever: shell 是否永久运行，true 通常是
        true,           // wait: shell init 是否阻塞等待完成
        no_login_check  // 登录验证函数指针，NULL 表示不验证
    );
    if (status != RTEMS_SUCCESSFUL)
    {
        printf("Failed to initialize shell: %d\n", status);
        // 处理失败，比如退出或重试
    }

    // 如果 forever 为 true，这里 shell 会阻塞，下面代码通常不会执行
    while (1)
    {
        rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
    }

    // 结束任务
    // exit(0);
}
