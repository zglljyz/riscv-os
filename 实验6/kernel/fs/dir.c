#include "types.h"
#include "riscv.h"
#include "fs.h"
#include "proc.h"
#include "string.h"

// 外部函数声明
void printf(const char *fmt, ...);

//===========================================
// 实验七：目录操作实现 (dir.c)
//===========================================

// 在目录中查找文件名，返回inode号
int dirlookup(struct inode *dp, char *name, uint32_t *poff) {
    uint32_t off;
    struct dirent de;

    if (dp->type != T_DIR) {
        printf("dirlookup: not a directory\n");
        return -1;
    }

    for (off = 0; off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
            printf("dirlookup: read failed\n");
            return -1;
        }

        if (de.inum == 0) continue;  // 空闲目录项

        if (strncmp(name, de.name, MAX_NAME_LEN) == 0) {
            // 找到匹配的文件名
            if (poff) *poff = off;
            printf("dirlookup: found %s at inum %d\n", name, de.inum);
            return de.inum;
        }
    }

    return -1;  // 未找到
}

// 在目录中创建新的目录项
int dirlink(struct inode *dp, char *name, uint32_t inum) {
    int off;
    struct dirent de;

    // 检查文件名是否已存在
    if (dirlookup(dp, name, 0) >= 0) {
        printf("dirlink: %s already exists\n", name);
        return -1;
    }

    // 查找空闲的目录项
    for (off = 0; off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
            printf("dirlink: read failed\n");
            return -1;
        }

        if (de.inum == 0) break;  // 找到空闲目录项
    }

    // 创建新目录项
    strncpy(de.name, name, MAX_NAME_LEN);
    de.inum = inum;

    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
        printf("dirlink: write failed\n");
        return -1;
    }

    printf("dirlink: linked %s to inum %d at offset %d\n", name, inum, off);
    return 0;
}

// 路径名解析：返回路径对应的inode
struct inode* namei(char *path) {
    char name[MAX_NAME_LEN];
    return namex(path, 0, name);
}

// 路径名解析：返回父目录的inode，并将最后一部分文件名存储在name中
struct inode* nameiparent(char *path, char *name) {
    return namex(path, 1, name);
}

// 路径名解析的核心函数
// nameiparent: 如果为1，返回父目录；如果为0，返回完整路径对应的inode
struct inode* namex(char *path, int nameiparent, char *name) {
    struct inode *ip, *next;

    if (*path == '/') {
        ip = iget(ROOTDEV, ROOTINO);  // 从根目录开始
    } else {
        ip = iget(ROOTDEV, ROOTINO);  // 简化：总是从根目录开始
    }

    while ((path = skipelem(path, name)) != 0) {
        ilock(ip);
        if (ip->type != T_DIR) {
            iunlock(ip);
            iput(ip);
            return 0;
        }

        if (nameiparent && *path == '\0') {
            // 这是最后一个元素且我们要返回父目录
            iunlock(ip);
            return ip;
        }

        int inum = dirlookup(ip, name, 0);
        if (inum < 0) {
            iunlock(ip);
            iput(ip);
            return 0;
        }

        next = iget(ip->dev, inum);
        iunlock(ip);
        iput(ip);
        ip = next;
    }

    if (nameiparent) {
        iput(ip);
        return 0;
    }

    return ip;
}

// 跳过路径分隔符，提取下一个路径元素
char* skipelem(char *path, char *name) {
    char *s;
    int len;

    while (*path == '/') path++;  // 跳过前导斜杠

    if (*path == 0) return 0;     // 路径结束

    s = path;
    while (*path != '/' && *path != 0) path++;  // 找到下一个斜杠或字符串结束

    len = path - s;
    if (len >= MAX_NAME_LEN) {
        memcpy(name, s, MAX_NAME_LEN - 1);
        name[MAX_NAME_LEN - 1] = 0;
    } else {
        memcpy(name, s, len);
        name[len] = 0;
    }

    while (*path == '/') path++;  // 跳过尾随斜杠

    return path;
}

// 创建新文件/目录的通用函数
struct inode* create(char *path, short type, short major, short minor) {
    struct inode *ip, *dp;
    char name[MAX_NAME_LEN];

    begin_op();

    if ((dp = nameiparent(path, name)) == 0) {
        end_op();
        return 0;
    }

    ilock(dp);

    if (dirlookup(dp, name, 0) >= 0) {
        // 文件已存在
        iunlock(dp);
        iput(dp);
        end_op();
        return 0;
    }

    if ((ip = ialloc(dp->dev, type)) == 0) {
        iunlock(dp);
        iput(dp);
        end_op();
        return 0;
    }

    ilock(ip);
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    iwrite(ip);

    if (type == T_DIR) {
        // 为目录创建 "." 和 ".." 目录项
        dp->nlink++;  // 父目录链接数增加（因为子目录的".."指向它）
        iwrite(dp);

        // "."
        if (dirlink(ip, ".", ip->inum) < 0) goto fail;
        // ".."
        if (dirlink(ip, "..", dp->inum) < 0) goto fail;
    }

    if (dirlink(dp, name, ip->inum) < 0) goto fail;

    iunlock(dp);
    iput(dp);
    iunlock(ip);

    end_op();

    printf("create: created %s (type=%d, inum=%d)\n", path, type, ip->inum);
    return ip;

fail:
    iunlock(dp);
    iput(dp);
    ip->nlink = 0;
    iwrite(ip);
    iunlock(ip);
    iput(ip);
    end_op();
    return 0;
}

// 系统调用包装函数

// 创建普通文件
int sys_create(char *path) {
    struct inode *ip = create(path, T_FILE, 0, 0);
    if (ip == 0) return -1;

    iput(ip);
    return 0;
}

// 创建目录
int sys_mkdir(char *path) {
    struct inode *ip = create(path, T_DIR, 0, 0);
    if (ip == 0) return -1;

    iput(ip);
    return 0;
}

// 删除文件/目录
int sys_unlink(char *path) {
    struct inode *ip, *dp;
    struct dirent de;
    char name[MAX_NAME_LEN];
    uint32_t off;

    begin_op();

    if ((dp = nameiparent(path, name)) == 0) {
        end_op();
        return -1;
    }

    ilock(dp);

    // 不能删除 "." 和 ".."
    if (strncmp(name, ".", MAX_NAME_LEN) == 0 || strncmp(name, "..", MAX_NAME_LEN) == 0) {
        iunlock(dp);
        iput(dp);
        end_op();
        return -1;
    }

    int inum = dirlookup(dp, name, &off);
    if (inum < 0) {
        iunlock(dp);
        iput(dp);
        end_op();
        return -1;
    }

    ip = iget(dp->dev, inum);
    ilock(ip);

    if (ip->nlink < 1) {
        printf("unlink: nlink < 1\n");
        goto fail;
    }

    if (ip->type == T_DIR && !isdirempty(ip)) {
        printf("unlink: directory not empty\n");
        goto fail;
    }

    // 清除目录项
    memset(&de, 0, sizeof(de));
    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
        printf("unlink: writei failed\n");
        goto fail;
    }

    if (ip->type == T_DIR) {
        dp->nlink--;
        iwrite(dp);
    }

    iunlock(dp);
    iput(dp);

    ip->nlink--;
    iwrite(ip);
    iunlock(ip);
    iput(ip);

    end_op();

    printf("sys_unlink: deleted %s\n", path);
    return 0;

fail:
    iunlock(dp);
    iput(dp);
    iunlock(ip);
    iput(ip);
    end_op();
    return -1;
}

// 检查目录是否为空（仅包含"."和".."）
int isdirempty(struct inode *dp) {
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
            printf("isdirempty: read failed\n");
            return 0;
        }

        if (de.inum != 0) return 0;  // 发现非空目录项
    }

    return 1;  // 目录为空
}