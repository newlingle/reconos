/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Architecture specific code - Linux
 *
 *   project:      ReconOS
 *   author:       Christoph Rüthing, University of Paderborn
 *   description:  Functions needed for ReconOS which are architecure
 *                 specific and are implemented here.
 *
 * ======================================================================
 */

#if defined(RECONOS_OS_linux)

#include "arch.h"
#include "../utils.h"

#include "arch_linux_kernel.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "pthread.h"

#define PROC_CONTROL_DEV "/dev/reconos-proc-control"
#define OSIF_DEV "/dev/reconos-osif"

unsigned int NUM_CLKMGS = 1;


/* == OSIF related functions ============================================ */

int reconos_osif_open(uint32_t ctrl_mask, uint32_t ctrl_bits) {
	int fd = open(OSIF_DEV, O_RDWR);
	if (fd < 0) {
		panic("[reconos-osif] "
		      "cannot open osif (0x%x, 0x%x)\n", ctrl_mask, ctrl_bits);
		return 0;
	}

	ioctl(fd, RECONOS_OSIF_SET_MASK, &ctrl_mask);
	ioctl(fd, RECONOS_OSIF_SET_BITS, &ctrl_bits);

	return fd;
}

ssize_t reconos_osif_read(int fd, uint32_t *buf, size_t count) {
	return read(fd, buf, count);
}

ssize_t reconos_osif_write(int fd, uint32_t *buf, size_t count) {
	return write(fd, buf, count);
}

void reconos_osif_break(int fd) {
#if 0
	struct osif_fifo_dev *dev = &osif_fifo_dev[fd];

	ioctl(osif_intc_fd, RECONOS_OSIF_INTC_BREAK, &dev->index);
#endif
}

void reconos_osif_close(int fd) {
	close(fd);
}


/* == Proc control related functions ==================================== */

int proc_control_fd;

int reconos_proc_control_open() {
	return proc_control_fd;
}

int reconos_proc_control_get_num_hwts(int fd) {
	int data;

	ioctl(fd, RECONOS_PROC_CONTROL_GET_NUM_HWTS, &data);

	return data;
}

int reconos_proc_control_get_tlb_hits(int fd) {
	int data;

	ioctl(fd, RECONOS_PROC_CONTROL_GET_TLB_HITS, &data);

	return data;
}

int reconos_proc_control_get_tlb_misses(int fd) {
	int data;

	ioctl(fd, RECONOS_PROC_CONTROL_GET_TLB_MISSES, &data);

	return data;
}

uint32_t reconos_proc_control_get_fault_addr(int fd) {
	uint32_t data;

	ioctl(fd, RECONOS_PROC_CONTROL_GET_FAULT_ADDR, &data);

	return data;
}

void reconos_proc_control_clear_page_fault(int fd) {
	ioctl(fd, RECONOS_PROC_CONTROL_CLEAR_PAGE_FAULT, NULL);
}

void reconos_proc_control_set_pgd(int fd) {
	ioctl(fd, RECONOS_PROC_CONTROL_SET_PGD_ADDR, NULL);
}

void reconos_proc_control_sys_reset(int fd) {
	ioctl(fd, RECONOS_PROC_CONTROL_SYS_RESET, NULL);
}

void reconos_proc_control_hwt_reset(int fd, int num, int reset) {
	if (reset)
		ioctl(fd, RECONOS_PROC_CONTROL_SET_HWT_RESET, &num);
	else
		ioctl(fd, RECONOS_PROC_CONTROL_CLEAR_HWT_RESET, &num);
}

void reconos_proc_control_hwt_signal(int fd, int num, int sig) {
	if (sig)
		ioctl(fd, RECONOS_PROC_CONTROL_SET_HWT_SIGNAL, &num);
	else
		ioctl(fd, RECONOS_PROC_CONTROL_CLEAR_HWT_SIGNAL, &num);
}

void reconos_proc_control_cache_flush(int fd) {
#if	defined(RECONOS_BOARD_ml605)
	ioctl(fd, RECONOS_PROC_CONTROL_CACHE_FLUSH, NULL);
#endif
}

void reconos_proc_control_set_ic_sig(int fd, int sig) {
	ioctl(fd, RECONOS_PROC_CONTROL_SET_IC_SIG, &sig);
}

int reconos_proc_control_get_ic_rdy(int fd) {
	int data;

	ioctl(fd, RECONOS_PROC_CONTROL_GET_IC_RDY, &data);

	return data;
}

void reconos_proc_control_set_ic_rst(int fd, int rst) {
	ioctl(fd, RECONOS_PROC_CONTROL_SET_IC_RST, &rst);
}

void reconos_proc_control_close(int fd) {
	close(fd);
}


/* == Clock related functions =========================================== */

#define CLOCK_BASE_ADDR    0x69E00000
#define CLOCK_BASE_SIZE    0x10000
#define CLOCK_MEM_SIZE     0x20

#define CLOCK_REG_HIGH_BIT(t) (((t) & 0x0000003F) << 6)
#define CLOCK_REG_LOW_BIT(t)  (((t) & 0x0000003F) << 0)
#define CLOCK_REG_EDGE_BIT    0x00800000
#define CLOCK_REG_COUNT_BIT   0x00400000

struct clock_dev {
	unsigned int index;

	volatile uint32_t *ptr;
};

struct clock_dev *clock_dev;

static inline void clock_write(struct clock_dev *dev, int clk, uint32_t reg) {
	dev->ptr[clk] = reg;
}

int reconos_clock_open(int num) {
	debug("[reconos-clock-%d] "
	      "opening ...\n", num);

	if (num < 0 || num >= NUM_CLKMGS)
		return -1;
	else
		return num;
}

void reconos_clock_set_divider(int fd, int clk, int divd) {
	struct clock_dev *dev = &clock_dev[fd];
	uint32_t reg = 0;

	debug("[reconos-clock-%d] "
	      "writing divider %d of clock %d ...\n", fd, divd, clk);

	if (divd < 1 || divd > 126) {
		whine("[reconos-clock-%d] "
		      "divider out of range %d\n", fd, divd);
		return;
	}

	if (divd == 1) {
		reg |= CLOCK_REG_EDGE_BIT;
		reg |= CLOCK_REG_COUNT_BIT;
		reg |= CLOCK_REG_LOW_BIT(1);

	} else if (divd % 2 == 0) {
		reg |= CLOCK_REG_HIGH_BIT(divd / 2) | CLOCK_REG_LOW_BIT(divd / 2);
	} else {
		reg |= CLOCK_REG_EDGE_BIT;
		reg |= CLOCK_REG_HIGH_BIT(divd / 2) | CLOCK_REG_LOW_BIT(divd / 2 + 1);
	}

	clock_write(dev, clk, reg);
}

void reconos_clock_close(int fd) {
	debug("[reconos-clock-%d] "
	      "closing ...\n", fd);
}


/* == Reconfiguration related functions ================================= */

#if defined(RECONOS_BOARD_zedboard_c) || defined(RECONOS_BOARD_zedboard_d)

int is_configured = 0;
pthread_mutex_t mutex;

inline void init_xdevcfg() {
	if (!is_configured) {
		pthread_mutex_init(&mutex, NULL);
		is_configured = 1;
	}
}

int load_bitstream(uint32_t *bitstream, size_t bitstream_length, int partial) {
	int fd;
	char d;

	init_xdevcfg();

	pthread_mutex_lock(&mutex);

	fd = open("/sys/class/xdevcfg/xdevcfg/device/is_partial_bitstream", O_WRONLY);
	if (!fd) {
		printf("[xdevcfg lib] failed to open\n");
		exit(EXIT_FAILURE);
	}
	if (partial)
		d = '1';
	else
		d = '0';
	write(fd, &d, 1);
	close(fd);

	fd = open("/dev/xdevcfg", O_WRONLY);
	if (!fd) {
		printf("[xdevcfg lib] failed to open\n");
		exit(EXIT_FAILURE);
	}
	write(fd, bitstream, bitstream_length);
	close(fd);

	fd = open("/sys/class/xdevcfg/xdevcfg/device/prog_done", O_RDONLY);
	if (!fd) {
		printf("[xdevcfg lib] failed to open\n");
		exit(EXIT_FAILURE);
	}
	do {
		read(fd, &d, 1);
		debug("... Waiting for programming to finish, currently reading %c\n", d);
	} while(d != '1');
	close(fd);

	pthread_mutex_unlock(&mutex);

	return 0;
}

#elif defined(RECONOS_BOARD_ml605)

int load_partial_bitstream(uint32_t *bitstream, unsigned int bitstream_length, int partial) {
	panic("NOT IMPLEMENTED YET\n");

	return 0;
}

#endif


/* == Initialization function =========================================== */

void reconos_drv_init() {
	int i;
	int fd;
	char *mem;


	// opening proc control device
	fd = open(PROC_CONTROL_DEV, O_RDWR);
	if (fd < 0)
		panic("[reconos-core] "
		      "error while opening proc control\n");
	else
		proc_control_fd = fd;

	// reset entire system
	reconos_proc_control_sys_reset(proc_control_fd);

	// create mapping for clock
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0)
		panic("[reconos-clock] "
		      "failed to open /dev/mem\n");

	mem = (char *)mmap(0, CLOCK_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CLOCK_BASE_ADDR);
	if (mem == MAP_FAILED)
		panic("[reconos-clock] "
		      "failed to mmap clock memory\n");

	close(fd);


	// allocate and initialize clock devices
	clock_dev = (struct clock_dev*)malloc(NUM_CLKMGS * sizeof(struct clock_dev));
	if (!clock_dev)
		panic("[reconos-clock] "
		      "failed to allocate memory\n");

	for (i = 0; i < NUM_CLKMGS; i++) {
		clock_dev[i].index = i;
		clock_dev[i].ptr = (uint32_t *)(mem + i * CLOCK_MEM_SIZE);
	}
}

#endif
