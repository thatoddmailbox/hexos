#include <stdint.h>

#include <kernel/heap.h>
#include <kernel/shell.h>
#include <kernel/vfs.h>

char shell_buffer[128];
uint8_t shell_buffer_len = 0;

fs_node_t * current_dir;

void shell_init() {
	current_dir = &fs_root;
}

void shell_prompt() {
	printf("> ");
}

char * shell_pwd() {
	return "hex";
}

void shell_putchar(char c) {
	if (c == '\n') {
		printf("\n");
		shell_runcmd();
		shell_buffer[0] = '\0';
		shell_buffer_len = 0;
		shell_prompt();
		return;
	} else if (c == '\b') {
		if (shell_buffer_len <= 0) {
			return;
		}
		shell_buffer_len--;
		shell_buffer[shell_buffer_len] = '\0';
		terminal_deletechar();
		return;
	}
	if (shell_buffer_len > 254) {
		return;
	}
	shell_buffer[shell_buffer_len] = c;
	shell_buffer[shell_buffer_len + 1] = '\0';
	shell_buffer_len++;
	terminal_putchar(c);
}

void shell_runcmd() {
	if (shell_buffer_len == 0) {
		return;
	} else if (!strcmp(shell_buffer, "uname")) {
		printf("HexOS\n");
		return;
	} else if (!strcmp(shell_buffer, "ls")) {
		uint32_t i = 0;
		dirent * entry;
		entry = readdir_fs(current_dir, i);
		while (entry != 0) {
			printf("%s\n", entry->d_name);
			i++;
			entry = readdir_fs(current_dir, i);
		}
		return;
	} else if (shell_buffer[0] == 'c' && shell_buffer[1] == 'd' && shell_buffer[2] == ' ') {
		char * dest = shell_buffer + 3;
		if (!strcmp(dest, "..")) {
			if (current_dir->parent.recreate == 0) {
				printf("cd: cannot go up further\n");
				return;
			}
			fs_node_t * old_dir = current_dir;
			current_dir = current_dir->parent.recreate(&(current_dir->parent));
			old_dir->free_node(old_dir);
			return;
		}
		fs_node_t * dest_node = finddir_fs(current_dir, dest);
		if (dest_node == 0) {
			printf("cd: %s: no such file or directory\n", dest);
			return;
		}
		if ((dest_node->flags&0x7) != FS_DIRECTORY) {
			printf("cd: %s: is a file\n", dest);
			return;
		}
		current_dir = dest_node;
	} else if (!strcmp(shell_buffer, "pwd")) {
		printf("%s\n", shell_pwd());
	} else if (!strcmp(shell_buffer, "mem")) {
		printf("TODO: this\n");
	} else {
		printf("Unknown or invalid command!\n");
		return;
	}
}
