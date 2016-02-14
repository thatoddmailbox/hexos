#include <stdint.h>

#include <kernel/shell.h>

char shell_buffer[128];
uint8_t shell_buffer_len = 0;

void shell_init() {

}

void shell_prompt() {
	printf("> ");
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
		/*char buf[3];
		itoa(buf, shell_buffer_len, 10);
		dbgprint(buf);
		dbgprint("\n");*/
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
	} else if(!strcmp(shell_buffer, "uname")) {
		printf("HexOS\n");
		return;
	} else {
		printf("Unknown or invalid command!\n");
		return;
	}
}
