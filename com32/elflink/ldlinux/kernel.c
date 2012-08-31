#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <console.h>
#include <dprintf.h>
#include <syslinux/loadfile.h>
#include <syslinux/linux.h>
#include <syslinux/pxe.h>
#include "core.h"

const char *globaldefault = NULL;
const char *append = NULL;

/* Will be called from readconfig.c */
int new_linux_kernel(char *okernel, char *ocmdline)
{
	const char *kernel_name = NULL;
	struct initramfs *initramfs = NULL;
	char *temp;
	void *kernel_data;
	size_t kernel_len;
	bool opt_quiet = false;
	bool accept_comma = false; /* multidisk syntax workaround for initramfs */
	char initrd_name[256];
	char cmdline_buf[256], *cmdline;

	dprintf("okernel = %s, ocmdline = %s", okernel, ocmdline);

	cmdline = cmdline_buf;

	temp = cmdline;
	/*
	strcpy(temp, "BOOT_IMAGE=");
	temp += 11;
	*/

	if (okernel)
		kernel_name = okernel;
	else if (globaldefault)
		kernel_name = globaldefault;

	strcpy(temp, kernel_name);
	temp += strlen(kernel_name);

	/* in elflink branch, KernelCName no more exist */	
	/*
	else {
		strcpy(temp, KernelCName);
		temp += strlen(KernelCName);
		kernel_name = KernelCName;
	}
	*/

	*temp = ' ';
	temp++;
	if (ocmdline)
		strcpy(temp, ocmdline);
	else if (append)
		strcpy(temp, append);
	/*	
	else if (*(char *)CmdOptPtr)
		strcpy(temp, (char *)CmdOptPtr);
	else if (AppendLen) {
		for (i = 0; i < AppendLen; i++)
			*temp++ = AppendBuf[i];
		*temp = '\0';
	}
	*/

	printf("cmdline = %s\n", cmdline);
	/*
	printf("VkernelEnd = %x\n", VKernelEnd);
	printf("HighMemSize = %x\n", __com32.cs_memsize);
	*/

	/* "keeppxe" handling */
#if IS_PXELINUX
	extern char KeepPXE;

	if (strstr(cmdline, "keeppxe"))
		KeepPXE |= 1;
#endif

	if (strstr(cmdline, "quiet"))
		opt_quiet = true;

	if (!opt_quiet)
		printf("Loading %s... ", kernel_name);

	if (loadfile(kernel_name, &kernel_data, &kernel_len)) {
		if (opt_quiet)
			printf("Loading %s ", kernel_name);
		printf("failed!\n");
		goto bail;
	}

	if (!opt_quiet)
		printf("ok\n");

	/* Find and load initramfs */
	temp = strstr(cmdline, "initrd=");
	if (temp) {
		/* Initialize the initramfs chain */
		initramfs = initramfs_init();
		if (!initramfs)
			goto bail;

		temp += 6; /* strlen("initrd") */
		do {
		    accept_comma = false;
		    char *p = initrd_name;

		    temp++;	/* Skip = or , */

		    if (*temp == '(') /* multidisk path */
			    accept_comma = true;

		    /* should not stop at any comma before ')' if it's a multidisk path */
		    while (*temp != ' ' && (accept_comma || *temp != ',') && *temp) {
			    if (*temp == ')')
				accept_comma = false;
			    *p++ = *temp++;
		    }
		    *p = '\0';

		    if (!opt_quiet)
			printf("Loading %s...", initrd_name);

		    if (initramfs_load_archive(initramfs, initrd_name)) {
			if (opt_quiet)
			    printf("Loading %s ", initrd_name);
			printf("failed!\n");
			goto bail;
		    }

		    if (!opt_quiet)
			printf("ok\n");
		} while (*temp == ',');
	}

	/* This should not return... */
	syslinux_boot_linux(kernel_data, kernel_len, initramfs, NULL, cmdline);

bail:
	printf("Kernel load failure (insufficient memory?)\n");
	return 1;
}
