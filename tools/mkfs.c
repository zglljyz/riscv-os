#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>


#define BLOCK_SIZE 512
#define MAX_FILES 64
#define FILE_NAME_LEN 28

struct file_entry {
    char name[FILE_NAME_LEN];
    uint32_t offset;  // 文件在镜像中的偏移(块号)
    uint32_t size;    // 文件大小(字节)
};

struct fs_header {
    uint32_t magic;       // 魔数: 0x12345678
    uint32_t nfiles;      // 文件数量
    struct file_entry files[MAX_FILES];
};

int main(int argc, char *argv[]) {
    struct fs_header header;
    int fd_img, fd_file;
    char buf[BLOCK_SIZE];
    int current_block;
    struct stat st;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <fs.img> <file1> <file2> ...\n", argv[0]);
        return 1;
    }

    // 创建文件系统镜像
    fd_img = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd_img < 0) {
        perror("open fs.img");
        return 1;
    }

    // 初始化header
    memset(&header, 0, sizeof(header));
    header.magic = 0x12345678;
    header.nfiles = argc - 2;

    if (header.nfiles > MAX_FILES) {
        fprintf(stderr, "Too many files (max %d)\n", MAX_FILES);
        return 1;
    }

    // 文件数据从第一个块之后开始
    current_block = 1;

    // 处理每个文件
    for (int i = 0; i < header.nfiles; i++) {
        const char *filename = argv[i + 2];

        // 打开文件
        fd_file = open(filename, O_RDONLY);
        if (fd_file < 0) {
            perror(filename);
            return 1;
        }

        // 获取文件大小
        if (fstat(fd_file, &st) < 0) {
            perror("fstat");
            return 1;
        }

        // 提取文件名（去掉路径）
        const char *basename = strrchr(filename, '/');
        basename = basename ? basename + 1 : filename;

        // 填充文件条目
        strncpy(header.files[i].name, basename, FILE_NAME_LEN - 1);
        header.files[i].offset = current_block;
        header.files[i].size = st.st_size;

        printf("Adding %s: offset=%d, size=%d bytes\n",
               header.files[i].name,header.files[i].offset,
               header.files[i].size);

        // 计算文件占用的块数
        int nblocks = (st.st_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        current_block += nblocks;

        close(fd_file);
    }

    // 写入header (第0块)
    lseek(fd_img, 0, SEEK_SET);
    write(fd_img, &header, sizeof(header));

    // 写入文件数据
    for (int i = 0; i < header.nfiles; i++) {
        const char *filename = argv[i + 2];

        fd_file = open(filename, O_RDONLY);
        if (fd_file < 0) {
            perror(filename);
            return 1;
        }

        // 定位到文件数据位置
        lseek(fd_img, header.files[i].offset * BLOCK_SIZE, SEEK_SET);

        // 复制文件内容
        ssize_t n;
        while ((n = read(fd_file, buf, BLOCK_SIZE)) > 0) {
            if (write(fd_img, buf, BLOCK_SIZE) != BLOCK_SIZE) {
                perror("write");
                return 1;
            }
        }

        close(fd_file);
    }

    close(fd_img);

    printf("\nFile system image '%s' created successfully\n", argv[1]);
    printf("Total files: %d\n", header.nfiles);
    printf("Total blocks: %d\n", current_block);

    return 0;
}
