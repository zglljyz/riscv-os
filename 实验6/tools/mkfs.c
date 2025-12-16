#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define BLOCK_SIZE    4096
#define MAX_NAME_LEN  14
#define DIRECT_BLOCKS 12
#define ROOTINO 1
#define FS_MAGIC      0x12345678

struct superblock {
    uint32_t magic;
    uint32_t size;
    uint32_t nblocks;
    uint32_t ninodes;
    uint32_t nlog;
    uint32_t logstart;
    uint32_t inodestart;
    uint32_t bmapstart;
    uint32_t reserved[24];
};

// File types
#define T_DIR     1
#define T_FILE    2
#define T_DEVICE  3

// inode structure
struct dinode {
    uint16_t type;
    uint16_t major;
    uint16_t minor;
    uint16_t nlink;
    uint32_t size;
    uint32_t addrs[DIRECT_BLOCKS + 1];
};

// Directory entry
struct dirent {
    uint16_t inum;
    char name[MAX_NAME_LEN];
};

// Inodes per block
#define IPB (BLOCK_SIZE / sizeof(struct dinode))

// Bitmap bits per block
#define BPB (BLOCK_SIZE * 8)

// Block of inode i
#define IBLOCK(i, sb) ((i) / IPB + (sb).inodestart)

// Block of bitmap for block b
#define BBLOCK(b, sb) ((b) / BPB + (sb).bmapstart)

// implementation
int fsfd;
struct superblock sb;
unsigned char zeroes[BLOCK_SIZE];
uint32_t freeinode = 1; // inode 0 is unused, root is 1
uint32_t freeblock;

void balloc(int used);
void wsect(uint32_t sec, void *buf);
void winode(uint32_t inum, struct dinode *ip);
void rinode(uint32_t inum, struct dinode *dip);
uint32_t ialloc(uint16_t type);
void iappend(uint32_t inum, void *p, int n);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: mkfs fs.img [files...]\n");
        exit(1);
    }

    fsfd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fsfd < 0) {
        perror(argv[1]);
        exit(1);
    }

    // 1. Setup superblock parameters
    sb.magic = FS_MAGIC;
    sb.size = 1000;
    sb.nblocks = 900;
    sb.ninodes = 200;
    sb.nlog = 30;
    sb.logstart = 2;
    sb.inodestart = sb.logstart + sb.nlog;
    sb.bmapstart = sb.inodestart + (sb.ninodes + IPB - 1) / IPB;

    freeblock = sb.bmapstart + (sb.size + BPB - 1) / BPB;
    
    printf("fs layout:\n");
    printf("  superblock: 1 block\n");
    printf("  log: %d blocks (starts at %d)\n", sb.nlog, sb.logstart);
    printf("  inodes: %d blocks (starts at %d)\n", (sb.ninodes + IPB - 1) / IPB, sb.inodestart);
    printf("  bitmap: %d blocks (starts at %d)\n", (sb.size + BPB - 1) / BPB, sb.bmapstart);
    printf("  data blocks start at: %d\n", freeblock);

    // 2. Zero out the entire file system image
    for (int i = 0; i < sb.size; i++) {
        wsect(i, zeroes);
    }

    // 3. Write the superblock
    char buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);
    memcpy(buf, &sb, sizeof(sb));
    wsect(1, buf); // Superblock is at block 1

    // 4. Create the root directory
    uint32_t rootino = ialloc(T_DIR);
    if (rootino != ROOTINO) {
        fprintf(stderr, "root inode is not 1!\n");
        exit(1);
    }

    struct dirent de;
    de.inum = rootino;
    strncpy(de.name, ".", MAX_NAME_LEN);
    iappend(rootino, &de, sizeof(de));

    de.inum = rootino;
    strncpy(de.name, "..", MAX_NAME_LEN);
    iappend(rootino, &de, sizeof(de));

    // 5. Add files passed as arguments
    for (int i = 2; i < argc; i++) {
        char *path = argv[i];
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            perror(path);
            exit(1);
        }

        // Get filename from path
        char *name = strrchr(path, '/');
        if (name) name++;
        else name = path;
        
        uint32_t inum = ialloc(T_FILE);

        // Add directory entry to root directory
        de.inum = inum;
        strncpy(de.name, name, MAX_NAME_LEN);
        iappend(rootino, &de, sizeof(de));

        // Read file content and append to inode
        while (read(fd, buf, sizeof(buf)) > 0) {
            iappend(inum, buf, sizeof(buf));
        }
        close(fd);
    }
    
    // Fix size of root directory
    struct dinode din;
    rinode(rootino, &din);
    uint off = din.size;
    off = ((off/BLOCK_SIZE) + 1) * BLOCK_SIZE;
    din.size = off;
    winode(rootino, &din);

    // 6. Mark used blocks in bitmap
    balloc(freeblock);

    close(fsfd);
    printf("File system image '%s' created successfully.\n", argv[1]);
    exit(0);
}

void wsect(uint32_t sec, void *buf) {
    if (lseek(fsfd, sec * BLOCK_SIZE, 0) != sec * BLOCK_SIZE) {
        perror("lseek");
        exit(1);
    }
    if (write(fsfd, buf, BLOCK_SIZE) != BLOCK_SIZE) {
        perror("write");
        exit(1);
    }
}

void winode(uint32_t inum, struct dinode *ip) {
    char buf[BLOCK_SIZE];
    uint32_t bn = IBLOCK(inum, sb);
    rsect(bn, buf);
    struct dinode *dip = (struct dinode*)buf + (inum % IPB);
    *dip = *ip;
    wsect(bn, buf);
}

void rinode(uint32_t inum, struct dinode *dip) {
    char buf[BLOCK_SIZE];
    uint32_t bn = IBLOCK(inum, sb);
    rsect(bn, buf);
    struct dinode *ip = (struct dinode*)buf + (inum % IPB);
    *dip = *ip;
}

void rsect(uint32_t sec, void *buf) {
    if(lseek(fsfd, sec * BLOCK_SIZE, 0) != sec * BLOCK_SIZE){
        perror("lseek");
        exit(1);
    }
    if(read(fsfd, buf, BLOCK_SIZE) != BLOCK_SIZE){
        perror("read");
        exit(1);
    }
}

uint32_t ialloc(uint16_t type) {
    uint32_t inum = freeinode++;
    struct dinode din;
    memset(&din, 0, sizeof(din));
    din.type = type;
    din.nlink = 1;
    din.size = 0;
    winode(inum, &din);
    return inum;
}

void balloc(int used) {
    unsigned char buf[BLOCK_SIZE];
    printf("balloc: first %d blocks have been allocated\n", used);
    
    memset(buf, 0, BLOCK_SIZE);
    for (int i = 0; i < used; i++) {
        buf[i/8] = buf[i/8] | (1 << (i%8));
    }
    printf("balloc: writing bitmap block at sector %d\n", sb.bmapstart);
    wsect(sb.bmapstart, buf);
}

void iappend(uint32_t inum, void *xp, int n) {
    char *p = (char*)xp;
    struct dinode din;
    uint32_t fbn, off, n1;
    struct buf *bp;
    char buf[BLOCK_SIZE];
    uint32_t indirect[BLOCK_SIZE/sizeof(uint32_t)];

    rinode(inum, &din);
    off = din.size;

    while(n > 0){
        fbn = off / BLOCK_SIZE;
        if(fbn < DIRECT_BLOCKS){
            if(din.addrs[fbn] == 0){
                din.addrs[fbn] = freeblock++;
            }
            n1 = BLOCK_SIZE - (off % BLOCK_SIZE);
            if(n1 > n) n1 = n;
            wsect(din.addrs[fbn] + (off % BLOCK_SIZE) / BLOCK_SIZE, p);
            n -= n1;
            off += n1;
            p += n1;
        } else {
            if(din.addrs[DIRECT_BLOCKS] == 0){
                din.addrs[DIRECT_BLOCKS] = freeblock++;
            }
            rsect(din.addrs[DIRECT_BLOCKS], (char*)indirect);
            if(indirect[fbn - DIRECT_BLOCKS] == 0){
                indirect[fbn - DIRECT_BLOCKS] = freeblock++;
                wsect(din.addrs[DIRECT_BLOCKS], (char*)indirect);
            }
            n1 = BLOCK_SIZE - (off % BLOCK_SIZE);
            if(n1 > n) n1 = n;
            wsect(indirect[fbn-DIRECT_BLOCKS] + (off % BLOCK_SIZE) / BLOCK_SIZE, p);
            n -= n1;
            off += n1;
            p += n1;
        }
    }
    din.size = off;
    winode(inum, &din);
}
