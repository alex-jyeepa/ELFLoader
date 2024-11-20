#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "exec_parser.h"

static so_exec_t *exec;
static struct sigaction og_handler;
static int page_size;
static int descriptor;
static int *mapped;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	int position, *visited;
	void *index;
	if (signum == SIGSEGV && info->si_code != SEGV_ACCERR)
		for (int i = 0; i < exec->segments_no; i++)
			if (((void *)exec->segments[i].vaddr <= info->si_addr) &&
				(info->si_addr < ((void *)(exec->segments[i].vaddr + exec->segments[i].mem_size)))) {
				if (mapped[i] == 0) {
					exec->segments[i].data = calloc(exec->segments[i].mem_size / page_size + 1, sizeof(int));
					mapped[i] = 1;
				}
				visited = exec->segments[i].data;
				position = (int)(info->si_addr - exec->segments[i].vaddr) / page_size;
				if (visited[position] == 0) {
					visited[position] = 1;
					index = (void *)(exec->segments[i].vaddr + position * page_size);
					mmap(index, page_size, PROT_WRITE, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1, 0);
					lseek(descriptor, exec->segments[i].offset + page_size * position, SEEK_SET);
					if (position == exec->segments[i].file_size / page_size){
						read(descriptor, index, page_size - ((position + 1) * page_size - exec->segments[i].file_size));
						mprotect(index, page_size, exec->segments[i].perm);
						return;
					}
					if (info->si_addr < (exec->segments[i].vaddr + (void*)exec->segments[i].mem_size) &&
					 info->si_addr >= (exec->segments[i].vaddr +(void*)exec->segments[i].file_size)) {
						mprotect(index, page_size, exec->segments[i].perm);
						return;
					}
					read(descriptor, index, page_size);
					mprotect(index, page_size, exec->segments[i].perm);
					return;
				}
				og_handler.sa_sigaction(signum, info, context);
			}
	og_handler.sa_sigaction(signum, info, context);
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	og_handler.sa_sigaction = sa.sa_sigaction;
	page_size = getpagesize();

	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0)
	{
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	mapped = calloc(exec->segments_no, sizeof(int));

	descriptor = open(path, O_RDONLY);
	so_start_exec(exec, argv);

	return -1;
}