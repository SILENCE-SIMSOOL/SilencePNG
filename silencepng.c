#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_FILES 100000
#define PATH_BUFFER 4096

static char *files[MAX_FILES];
static size_t file_count = 0;

static volatile LONG cancelled = 0;
static HANDLE current_process = NULL;

static BOOL WINAPI console_handler(DWORD type) {
	if (type != CTRL_C_EVENT)
		return FALSE;

	InterlockedExchange(&cancelled, 1);

	printf("\n\n[!] Cancelling...\n");

	if (current_process != NULL)
		TerminateProcess(current_process, 1);

	return TRUE;
}

static int has_png_extension(const char *name) {
	const char *dot = strrchr(name, '.');

	if (dot == NULL)
		return 0;

	return _stricmp(dot, ".png") == 0;
}

static void add_file(const char *path) {
	if (file_count >= MAX_FILES)
		return;

	files[file_count] = _strdup(path);

	if (files[file_count] != NULL)
		file_count++;
}

static unsigned long long get_file_size(const char *path) {
	WIN32_FILE_ATTRIBUTE_DATA data;

	if (!GetFileAttributesExA(
		path,
		GetFileExInfoStandard,
		&data
	))
		return 0;

	ULARGE_INTEGER size;

	size.HighPart = data.nFileSizeHigh;
	size.LowPart = data.nFileSizeLow;

	return size.QuadPart;
}

static void scan_directory(const char *path) {
	char search[PATH_BUFFER];
	char full[PATH_BUFFER];

	WIN32_FIND_DATAA data;

	snprintf(
		search,
		sizeof(search),
		"%s\\*",
		path
	);

	HANDLE find = FindFirstFileA(search, &data);

	if (find == INVALID_HANDLE_VALUE)
		return;

	do {
		if (cancelled)
			break;

		if (
			strcmp(data.cFileName, ".") == 0 ||
			strcmp(data.cFileName, "..") == 0
		)
			continue;

		snprintf(
			full,
			sizeof(full),
			"%s\\%s",
			path,
			data.cFileName
		);

		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			scan_directory(full);
			continue;
		}

		if (has_png_extension(data.cFileName))
			add_file(full);

	} while (FindNextFileA(find, &data));

	FindClose(find);
}

static int run_process(char *command, int hide_output) {
	STARTUPINFOA startup;
	PROCESS_INFORMATION process;

	ZeroMemory(&startup, sizeof(startup));
	ZeroMemory(&process, sizeof(process));

	startup.cb = sizeof(startup);

	HANDLE null_file = NULL;

	if (hide_output) {
		null_file = CreateFileA(
			"NUL",
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		startup.dwFlags |= STARTF_USESTDHANDLES;
		startup.hStdOutput = null_file;
		startup.hStdError = null_file;
		startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	}

	if (!CreateProcessA(
		NULL,
		command,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&startup,
		&process
	)) {
		if (null_file != NULL)
			CloseHandle(null_file);

		return -1;
	}

	current_process = process.hProcess;

	while (!cancelled) {
		DWORD result = WaitForSingleObject(
			process.hProcess,
			100
		);

		if (result == WAIT_OBJECT_0)
			break;
	}

	if (cancelled)
		TerminateProcess(process.hProcess, 1);

	WaitForSingleObject(
		process.hProcess,
		INFINITE
	);

	DWORD exit_code = 1;

	GetExitCodeProcess(
		process.hProcess,
		&exit_code
	);

	current_process = NULL;

	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);

	if (null_file != NULL)
		CloseHandle(null_file);

	return (int) exit_code;
}

static int command_exists(const char *command) {
	char cmd[512];

	snprintf(
		cmd,
		sizeof(cmd),
		"where %s >nul 2>&1",
		command
	);

	return system(cmd) == 0;
}

static int install_oxipng(void) {
	printf("[1/3] oxipng not found.\n");
	printf("      Installing oxipng...\n\n");

	char command[] =
		"winget.exe install oxipng "
		"--accept-package-agreements "
		"--accept-source-agreements";

	int result = run_process(
		command,
		0
	);

	if (cancelled)
		return 0;

	if (result != 0) {
		printf("\n[!] Failed to install oxipng.\n");
		return 0;
	}

	return 1;
}

static void free_files(void) {
	for (size_t i = 0; i < file_count; i++)
		free(files[i]);
}

static void format_number(
	unsigned long long value,
	char *output,
	size_t output_size
) {
	char temp[64];

	snprintf(
		temp,
		sizeof(temp),
		"%llu",
		value
	);

	size_t length = strlen(temp);
	size_t commas = length > 0 ? (length - 1) / 3 : 0;
	size_t final_length = length + commas;

	if (final_length + 1 > output_size) {
		snprintf(
			output,
			output_size,
			"%llu",
			value
		);

		return;
	}

	output[final_length] = '\0';

	size_t src = length;
	size_t dst = final_length;
	int group = 0;

	while (src > 0) {
		output[--dst] = temp[--src];

		group++;

		if (
			group == 3 &&
			src > 0
		) {
			output[--dst] = ',';
			group = 0;
		}
	}
}

static void wait_for_exit(void) {
	printf("\nPress any key to exit...");
	_getch();
}

int main(int argc, char **argv) {
	SetConsoleCtrlHandler(
		console_handler,
		TRUE
	);

	printf(
		"\n"
		" SILENCE PNG\n"
		" PNG Optimizer\n"
		" ----------------------------------------\n\n"
	);

	if (argc != 2) {
		printf("Usage:\n");
		printf("  silencepng <path>\n");

		wait_for_exit();

		return 1;
	}

	if (!command_exists("oxipng")) {
		if (!install_oxipng()) {
			wait_for_exit();
			return 1;
		}

		if (!command_exists("oxipng")) {
			printf(
				"\n"
				"[!] oxipng was installed, but it is not "
				"available in PATH yet.\n"
				"    Restart the terminal and try again.\n"
			);

			wait_for_exit();

			return 1;
		}
	} else {
		printf("[1/3] oxipng found.\n");
	}

	if (cancelled) {
		wait_for_exit();
		return 1;
	}

	printf("[2/3] Scanning PNG files...\n");

	DWORD attributes = GetFileAttributesA(
		argv[1]
	);

	if (attributes == INVALID_FILE_ATTRIBUTES) {
		printf(
			"\n[!] Path not found:\n%s\n",
			argv[1]
		);

		wait_for_exit();

		return 1;
	}

	if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
		scan_directory(argv[1]);
	} else {
		if (has_png_extension(argv[1]))
			add_file(argv[1]);
	}

	if (cancelled) {
		free_files();
		wait_for_exit();

		return 1;
	}

	printf(
		"      Found %zu PNG files.\n\n",
		file_count
	);

	if (file_count == 0) {
		printf("No PNG files found.\n");

		free_files();
		wait_for_exit();

		return 0;
	}

	unsigned long long original_size = 0;

	for (size_t i = 0; i < file_count; i++)
		original_size += get_file_size(files[i]);

	printf("[3/3] Optimizing...\n");
	printf("      Press Ctrl+C to cancel.\n\n");

	size_t success = 0;
	size_t failed = 0;
	size_t modified = 0;

	for (size_t i = 0; i < file_count; i++) {
		if (cancelled)
			break;

		unsigned long long before =
			get_file_size(files[i]);

		printf(
			"[%zu/%zu] %s\n",
			i + 1,
			file_count,
			files[i]
		);

		char command[PATH_BUFFER + 128];

		snprintf(
			command,
			sizeof(command),
			"oxipng.exe -o max \"%s\"",
			files[i]
		);

		int result = run_process(
			command,
			1
		);

		if (cancelled)
			break;

		if (result == 0) {
			success++;

			unsigned long long after =
				get_file_size(files[i]);

			if (after != before)
				modified++;
		} else {
			failed++;
		}
	}

	unsigned long long optimized_size = 0;

	for (size_t i = 0; i < file_count; i++)
		optimized_size += get_file_size(files[i]);

	unsigned long long saved_size = 0;

	if (original_size > optimized_size)
		saved_size = original_size - optimized_size;

	char original_text[64];
	char optimized_text[64];
	char saved_text[64];

	format_number(
		original_size,
		original_text,
		sizeof(original_text)
	);

	format_number(
		optimized_size,
		optimized_text,
		sizeof(optimized_text)
	);

	format_number(
		saved_size,
		saved_text,
		sizeof(saved_text)
	);

	printf("\n----------------------------------------\n");

	if (cancelled) {
		printf("Optimization cancelled.\n\n");

		printf(
			"Processed          : %zu / %zu\n",
			success + failed,
			file_count
		);
	} else {
		printf("Optimization complete.\n\n");
	}

	printf(
		"PNG files modified : %zu\n",
		modified
	);

	printf(
		"Original size      : %s bytes\n",
		original_text
	);

	printf(
		"Optimized size     : %s bytes\n",
		optimized_text
	);

	printf(
		"Space saved        : %s bytes\n",
		saved_text
	);

	if (failed > 0)
		printf("Failed             : %zu\n", failed);

	free_files();

	wait_for_exit();

	return cancelled ? 1 : 0;
}