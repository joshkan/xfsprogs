// SPDX-License-Identifier: GPL-2.0
/*
 * Get or set the group a file's allocations start in.
 */
#include "platform_defs.h"
#include "command.h"
#include "init.h"
#include "io.h"

static cmdinfo_t allocgroup_cmd;

static void
allocgroup_help(void)
{
	printf(_(
"\n"
" report or set the allocation group of the current file\n"
"\n"
" With no argument, print the group, or \"none\".  With a number, start\n"
" the file's allocations in that AG, or rtgroup for a realtime file.\n"
" The setting is held in memory only and is lost when the inode is evicted.\n"
"\n"
" -c -- clear the allocation group\n"
"\n"));
}

static int
allocgroup_f(
	int			argc,
	char			**argv)
{
	struct xfs_alloc_group	ag = { };
	bool			clear = false;
	char			*end;
	int			c;

	while ((c = getopt(argc, argv, "c")) != EOF) {
		switch (c) {
		case 'c':
			clear = true;
			break;
		default:
			return command_usage(&allocgroup_cmd);
		}
	}

	if (clear && optind < argc)
		return command_usage(&allocgroup_cmd);

	if (!clear && optind == argc) {
		if (ioctl(file->fd, XFS_IOC_GET_ALLOC_GROUP, &ag) < 0) {
			perror("XFS_IOC_GET_ALLOC_GROUP");
			exitcode = 1;
			return 0;
		}
		if (ag.group == XFS_ALLOC_GROUP_NONE)
			printf(_("[none] %s\n"), file->name);
		else
			printf("[%u] %s\n", ag.group, file->name);
		return 0;
	}

	if (clear) {
		ag.group = XFS_ALLOC_GROUP_NONE;
	} else {
		errno = 0;
		ag.group = strtoul(argv[optind], &end, 0);
		if (errno || *end || ag.group == XFS_ALLOC_GROUP_NONE) {
			printf(_("invalid group -- %s\n"), argv[optind]);
			exitcode = 1;
			return 0;
		}
	}

	if (ioctl(file->fd, XFS_IOC_SET_ALLOC_GROUP, &ag) < 0) {
		perror("XFS_IOC_SET_ALLOC_GROUP");
		exitcode = 1;
	}
	return 0;
}

void
allocgroup_init(void)
{
	allocgroup_cmd.name = "allocgroup";
	allocgroup_cmd.cfunc = allocgroup_f;
	allocgroup_cmd.args = _("[-c | group]");
	allocgroup_cmd.argmin = 0;
	allocgroup_cmd.argmax = 1;
	allocgroup_cmd.flags = CMD_NOMAP_OK;
	allocgroup_cmd.oneline =
		_("get/set the allocation group of the open file");
	allocgroup_cmd.help = allocgroup_help;

	add_command(&allocgroup_cmd);
}
