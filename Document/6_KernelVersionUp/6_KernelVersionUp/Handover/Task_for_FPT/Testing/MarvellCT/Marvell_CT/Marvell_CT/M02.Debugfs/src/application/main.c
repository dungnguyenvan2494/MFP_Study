#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1ULL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE - 1))

/* ioctl command */

#define MVEBU_DBG_VA2PA 	_IOWR('q', 1, struct va2pa_args *)

#define VA2PA_TYPE_USER		0
#define VA2PA_TYPE_KERNEL	1
#define GET_VA_KERNEL_EXAMPLE	2

#define DEBUG_FS_FILE_PATH "/sys/kernel/debug/mvebu/debug"

struct va2pa_args {
	void *va;
	void *pa;
	int type;
	uint8_t  esr_fault;
	bool non_secure;
	uint8_t   sharability;
	uint8_t   mair;
};

uint64_t virtual_to_physical(void *vaddr) {
    int fd;
    uint64_t paddr = 0;
    uint64_t pagemap_entry;
    off_t offset;
    ssize_t read_size;

    // Open /proc/self/pagemap to read page information
    fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 0;
    }

    // Calculate offset in pagemap based on the virtual address
    offset = ((uintptr_t)vaddr >> PAGE_SHIFT) * sizeof(pagemap_entry);

    // Read the entry from pagemap
    read_size = pread(fd, &pagemap_entry, sizeof(pagemap_entry), offset);
    if (read_size != sizeof(pagemap_entry)) {
        perror("pread");
        close(fd);
        return 0;
    }

    // Check the "page present" bit
    if (pagemap_entry & (1ULL << 63)) {
        // Extract physical address from the entry
        paddr = (pagemap_entry & ((1ULL << 55) - 1)) << PAGE_SHIFT;
        // Add the offset within the page to get the full physical address
        paddr |= ((uintptr_t)vaddr & (PAGE_SIZE - 1));
    } else {
        printf("Page not present in memory\n");
    }

    close(fd);
    return paddr;
}

int main() {
	
	int fd = -1;
	va2pa_args value_arg;
	
	fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
        perror("Failed to open the device file");
        return EXIT_FAILURE;
    }
	
	value_arg->type = VA2PA_TYPE_USER;
	// Perform ioctl to get the value
    if (ioctl(fd, MVEBU_DBG_VA2PA, &value_arg) == -1) {
        perror("Failed to perform ioctl");
        close(fd);
        return EXIT_FAILURE;
    }
	
	
    // Allocate a memory region in user space
    void *buffer = malloc(PAGE_SIZE);
	memset(buffer, 0, PAGE_SIZE);
    if (!buffer) {
        perror("malloc");
        return 1;
    }

    // Get the corresponding physical address
    uint64_t phys_addr = virtual_to_physical(buffer);
    if (phys_addr == value_arg->pa) {
        printf("TC009 OK\n");
    }else {
		printf("TC009 NG\n");
	}
	
	value_arg->type = GET_VA_KERNEL_EXAMPLE;
	if (ioctl(fd, MVEBU_DBG_VA2PA, &value_arg) == -1) {
        perror("Failed to perform ioctl");
        close(fd);
        return EXIT_FAILURE;
    }
	
	phys_addr = value_arg->pa;
	
	value_arg->pa = NULL;
	value_arg->type = VA2PA_TYPE_KERNEL;
	
	if (ioctl(fd, MVEBU_DBG_VA2PA, &value_arg) == -1) {
        perror("Failed to perform ioctl");
        close(fd);
        return EXIT_FAILURE;
    }
	
	if(phys_addr = value_arg->pa) {
		printf("TC0010 OK\n");
	}else {
		printf("TC0010 NG\n");
	}
	
    free(buffer);
    return 0;
}
