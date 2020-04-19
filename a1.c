#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <string.h>
#ifdef __DEBUG
void debug_info(const char *file, const char *function, const int line)
{
	fprintf(stderr, "DEBUG. ERROR PLACE: File=\"%s\", Function=\"%s\", Line=\"%d\"\n", file, function, line);
}

#define ERR_MSG(DBG_MSG) { \
        perror(DBG_MSG); \
        debug_info(__FILE__, __FUNCTION__, __LINE__); \
}

#else                   // with no __DEBUG just displays the error message

#define ERR_MSG(DBG_MSG) { \
        perror(DBG_MSG); \
}

#endif
#define MAX_PATH_LEN 1024

typedef struct section_header {

	char aux;            //placeholder to make sure all the other fields are read correctly
	char sect_name[14];
	u_int8_t sect_type;
	u_int32_t sect_offset;
	u_int32_t sect_size;
}section_header;

typedef struct header {
	u_int32_t magic;
	u_int16_t headerSize;
	u_int16_t version;
	u_int8_t noOfSections;
}header;


int read_header(char *file_path) {

	header* aux = (header*)malloc(sizeof(header));
	if (aux == NULL) {
		printf("Couldn't allocate memory\n");
		return -1;
	}
	int fd;
	if ((fd = open(file_path, O_RDONLY)) < 0) {
		ERR_MSG("Error opening the source file");
		exit(2);
	}
	int nr;

	if ((nr = read(fd, aux, sizeof(header))) == 0) {
		printf("Couldn't read from file\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	if (aux->magic != 1630816885) {         //1630816885 is just "uJ4a"
		printf("ERROR\nwrong magic\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	if (aux->version < 76 || aux->version > 103) {
		printf("ERROR\nwrong version\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	if (aux->noOfSections < 5 || aux->noOfSections > 13) {
		printf("ERROR\nwrong sect_nr\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	int sect_types[] = { 48, 89, 64, 70, 60, 18, 21 };
	section_header **headers = (section_header**)malloc(aux->noOfSections * sizeof(section_header));
	if (headers == NULL) {
		printf("Couldn't allocate memory\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	lseek(fd, -4, SEEK_CUR);
	for (int i = 0; i < aux->noOfSections; i++) {
		headers[i] = (section_header*)malloc(sizeof(section_header));
		if ((nr = read(fd, headers[i], sizeof(section_header))) == 0) {
			printf("Couldn't read from file\n");
			for (int j = 0; j < aux->noOfSections; j++) {
				if (headers[j] != NULL) {
					free(headers[j]);
					headers[j] = NULL;
				}
			}
			free(headers);
			headers = NULL;
			free(aux);
			aux = NULL;
			return -1;
		}
		lseek(fd, -1, SEEK_CUR);
	}
	for (int i = 0; i < aux->noOfSections; i++) {
		int OK = 0;
		for (int j = 0; j < 7; j++) {
			if (headers[i]->sect_type == sect_types[j]) {
				OK = 1;
				break;
			}
		}
		if (!OK) {
			printf("ERROR\nwrong sect_types\n");
			for (int j = 0; j < aux->noOfSections; j++) {
				free(headers[j]);
				headers[j] = NULL;
			}
			free(headers);
			headers = NULL;
			free(aux);
			aux = NULL;
			return -1;
		}
	}
	printf("SUCCESS\n");
	printf("version=%d\n", aux->version);
	printf("nr_sections=%d\n", aux->noOfSections);
	for (int i = 0; i < aux->noOfSections; i++) {
		printf("section%d: %.14s %d %d\n", i + 1, headers[i]->sect_name, headers[i]->sect_type,
			headers[i]->sect_size);
	}

	for (int i = 0; i < aux->noOfSections; i++) {
		free(headers[i]);
		headers[i] = NULL;
	}
	free(headers);
	headers = NULL;
	free(aux);
	aux = NULL;
	return 0;
}

int extract_header(char *file_path, int section, int line) {

	header* aux = (header*)malloc(sizeof(header));
	if (aux == NULL) {
		printf("Couldn't allocate memory\n");
		return -1;
	}
	int fd;
	if ((fd = open(file_path, O_RDONLY)) < 0) {
		ERR_MSG("Error opening the source file");
		free(aux);
		aux = NULL;
		exit(2);
	}
	int nr;

	if ((nr = read(fd, aux, sizeof(header))) == 0) {
		printf("Couldn't read from file\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	if (aux->magic != 1630816885) {
		printf("ERROR\nwrong magic\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	if (aux->version < 76 || aux->version > 103) {
		printf("ERROR\nwrong version\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	if (aux->noOfSections < 5 || aux->noOfSections > 13) {
		printf("ERROR\nwrong sect_nr\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	int sect_types[] = { 48, 89, 64, 70, 60, 18, 21 };
	if (section > aux->noOfSections || section < 0) {
		printf("ERROR\ninvalid section\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	section_header **headers = (section_header**)malloc(aux->noOfSections * sizeof(section_header));
	if (headers == NULL) {
		printf("Couldn't allocate memory\n");
		free(aux);
		aux = NULL;
		return -1;
	}
	lseek(fd, -4, SEEK_CUR);
	for (int i = 0; i < section + 1; i++) {
		headers[i] = (section_header*)malloc(sizeof(section_header));
		if (headers[i] == NULL) {
			printf("Couldn't allocate memory\n");
			free(aux);
			aux = NULL;
			free(headers);
			headers = NULL;
			return -1;
		}
		if ((nr = read(fd, headers[i], sizeof(section_header))) == 0) {
			printf("Couldn't read from file\n");
			for (int j = 0; j < aux->noOfSections; j++) {
				free(headers[j]);
				headers[j] = NULL;
			}
			free(headers);
			headers = NULL;
			free(aux);
			aux = NULL;
			return -1;
		}
		lseek(fd, -1, SEEK_CUR);
	}

	lseek(fd, headers[section]->sect_offset, SEEK_SET);
	char lines[headers[section]->sect_size + 1];
	int lineNo = 0;
	for (int i = 0; i < section + 1; i++) {
		int OK = 0;
		for (int j = 0; j < 7; j++) {
			if (headers[i]->sect_type == sect_types[j]) {
				OK = 1;
				break;
			}
		}
		if (!OK) {
			printf("ERROR\nwrong sect_types\n");
			for (int j = 0; j < section + 1; j++) {
				free(headers[j]);
				headers[j] = NULL;
			}
			free(headers);
			headers = NULL;
			free(aux);
			aux = NULL;
			return -1;
		}
	}
	lseek(fd, headers[section]->sect_offset, SEEK_SET);
	if ((nr = read(fd, lines, headers[section]->sect_size)) == 0) {
		printf("Couldn't read from file\n");
		free(headers);
		headers = NULL;
		free(aux);
		aux = NULL;
		return -1;
	}
	printf("SUCCESS\n");
	for (int i = headers[section]->sect_size - 1; i >= 0; i--) {
		if (lines[i] == '\n') {
			lineNo++;
		}
		if (lineNo == line - 1)
			printf("%c", lines[i - 1]);
	}
	if (line > lineNo || line < 0) {
		printf("ERROR\ninvalid line\n");
		free(headers);
		headers = NULL;
		free(aux);
		aux = NULL;
		return -1;
	}
	for (int i = 0; i < section + 1; i++) {
		free(headers[i]);
		headers[i] = NULL;
	}
	free(headers);
	headers = NULL;
	free(aux);
	aux = NULL;
	return 0;
}

char *end = NULL;
char *permit = NULL;

int checkSection(char *dir_path) {

	header* aux = (header*)malloc(sizeof(header));
	if (aux == NULL) {
		printf("Couldn't allocate memory\n");
		return 0;
	}
	int fd;
	if ((fd = open(dir_path, O_RDONLY)) < 0) {
		ERR_MSG("Error opening the source file");
		free(aux);
		aux = NULL;
		return 0;
	}
	int nr;

	if ((nr = read(fd, aux, sizeof(header))) == 0) {
		printf("Couldn't read from file\n");
		free(aux);
		aux = NULL;
		return 0;
	}
	if (aux->magic != 1630816885) {
		free(aux);
		aux = NULL;
		return 0;
	}
	if (aux->version < 76 || aux->version > 103) {
		free(aux);
		aux = NULL;
		return 0;
	}
	if (aux->noOfSections < 5 || aux->noOfSections > 13) {
		free(aux);
		aux = NULL;
		return 0;
	}
	int sect_types[] = { 48, 89, 64, 70, 60, 18, 21 };
	section_header **headers = (section_header**)malloc(aux->noOfSections * sizeof(section_header));
	if (headers == NULL) {
		printf("Couldn't allocate memory\n");
		return 0;
	}
	lseek(fd, -4, SEEK_CUR);
	for (int i = 0; i < aux->noOfSections; i++) {
		headers[i] = (section_header*)malloc(sizeof(section_header));
		if ((nr = read(fd, headers[i], sizeof(section_header))) == 0) {
			printf("Couldn't read from file\n");
			return -1;
		}
		lseek(fd, -1, SEEK_CUR);
	}
	for (int i = 0; i < aux->noOfSections; i++) {
		int OK = 0;
		for (int j = 0; j < 7; j++) {
			if (headers[i]->sect_type == sect_types[j]) {
				OK = 1;
				break;
			}
		}
		if (!OK) {
			for (int i = 0; i < aux->noOfSections; i++) {
				free(headers[i]);
				headers[i] = NULL;
			}
			free(headers);
			headers = NULL;
			free(aux);
			aux = NULL;
			return 0;
		}
	}
	int hasLines = 0;
	for (int i = 0; i < aux->noOfSections; i++) {
		char lines[headers[i]->sect_size + 1];
		int lineNo = 0;
		lseek(fd, headers[i]->sect_offset, SEEK_SET);
		if ((nr = read(fd, lines, headers[i]->sect_size)) == 0) {
			printf("Couldn't read from file\n");
			return -1;
		}
		for (int j = headers[i]->sect_size - 1; j >= 0; j--) {
			if (lines[j] == '\n') {
				lineNo++;
			}
		}
		if (lineNo == 15) {
			hasLines = 1;
			break;
		}
	}
	for (int i = 0; i < aux->noOfSections; i++) {
		free(headers[i]);
		headers[i] = NULL;
	}
	free(headers);
	headers = NULL;
	free(aux);
	aux = NULL;
	return hasLines;
}
int checkNameEnd(char *dir_path) {
	if (end == NULL) {
		return 0;
	}
	int j = strlen(dir_path) - strlen(end);
	for (int i = 0; i < strlen(end); i++) {
		if (dir_path[j] != end[i]) {
			return -1;
		}
		j++;
	}
	return 0;
}

int checkPermissions(char *dir_path) {
	if (permit == NULL) {
		return 0;
	}
	char aux[9];
	memset(aux, 0, 9);
	struct stat fileMetadata;
	if (stat(dir_path, &fileMetadata) < 0) {
		return -1;
	}

	if (fileMetadata.st_mode & S_IRUSR)
		aux[0] = 'r';
	else
		aux[0] = '-';

	if (fileMetadata.st_mode & S_IWUSR)
		aux[1] = 'w';
	else
		aux[1] = '-';

	if (fileMetadata.st_mode & S_IXUSR)
		aux[2] = 'x';
	else
		aux[2] = '-';

	if (fileMetadata.st_mode & S_IRGRP)
		aux[3] = 'r';
	else
		aux[3] = '-';

	if (fileMetadata.st_mode & S_IWGRP)
		aux[4] = 'w';
	else
		aux[4] = '-';

	if (fileMetadata.st_mode & S_IXGRP)
		aux[5] = 'x';
	else
		aux[5] = '-';

	if (fileMetadata.st_mode & S_IROTH)
		aux[6] = 'r';
	else
		aux[6] = '-';

	if (fileMetadata.st_mode & S_IWOTH)
		aux[7] = 'w';
	else
		aux[7] = '-';

	if (fileMetadata.st_mode & S_IXOTH)
		aux[8] = 'x';
	else
		aux[8] = '-';

	if (strcmp(aux, permit) == 0) {
		return 0;
	}
	return -1;
}
void listDir(char *dirName) {

	DIR* dir;
	struct dirent *dirEntry;
	struct stat inode;
	char name[MAX_PATH_LEN];

	dir = opendir(dirName);
	if (dir == 0) {
		ERR_MSG("Error opening directory");
		exit(4);
	}

	while ((dirEntry = readdir(dir)) != 0) {
		if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)
			continue;
		snprintf(name, MAX_PATH_LEN, "%s/%s", dirName, dirEntry->d_name);
		lstat(name, &inode);

		char aux[MAX_PATH_LEN];
		realpath(dirName, aux);
		strcat(aux, "/");
		strcat(aux, dirEntry->d_name);
		if (!checkNameEnd(dirEntry->d_name) && !checkPermissions(aux))
			printf("%s/%s\n", dirName, dirEntry->d_name);
	}

	closedir(dir);
}

void listSF(char *dirName) {
	DIR* dir;
	struct dirent *dirEntry;
	struct stat inode;
	char name[MAX_PATH_LEN];

	dir = opendir(dirName);
	if (dir == 0) {
		ERR_MSG("Error opening directory");
		exit(4);
	}

	while ((dirEntry = readdir(dir)) != 0) {
		if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)
			continue;
		snprintf(name, MAX_PATH_LEN, "%s/%s", dirName, dirEntry->d_name);
		lstat(name, &inode);

		char aux[MAX_PATH_LEN];
		realpath(dirName, aux);
		strcat(aux, "/");
		strcat(aux, dirEntry->d_name);
		if (checkSection(aux)) {
			printf("%s/%s\n", dirName, dirEntry->d_name);
		}

	}

	closedir(dir);
}

void listDirRec(char *dirName) {
	DIR *dir;
	struct dirent *dirEntry;
	char name[MAX_PATH_LEN];

	dir = opendir(dirName);
	if (dir == 0) {
		printf("Error opening directory\n");
		return;
	}
	listDir(dirName);

	while ((dirEntry = readdir(dir)) != 0) {
		if (dirEntry->d_type == DT_DIR && strcmp(dirEntry->d_name, ".") != 0 &&
			strcmp(dirEntry->d_name, "..") != 0)
		{
			sprintf(name, "%s/%s", dirName, dirEntry->d_name);
			listDirRec(name);
		}
	}
	closedir(dir);
	return;
}

void listSFRec(char *dirName) {
	DIR *dir;
	struct dirent *dirEntry;
	char name[MAX_PATH_LEN];

	dir = opendir(dirName);
	if (dir == 0) {
		printf("Error opening directory\n");
		return;
	}
	listSF(dirName);

	while ((dirEntry = readdir(dir)) != 0) {
		if (dirEntry->d_type == DT_DIR && strcmp(dirEntry->d_name, ".") != 0 &&
			strcmp(dirEntry->d_name, "..") != 0)
		{
			sprintf(name, "%s/%s", dirName, dirEntry->d_name);

			listSFRec(name);
		}
	}
	closedir(dir);
	return;
}

void printVariant() {
	printf("22771\n");
}


int main(int argc, char **argv) {

	char *dir_path;
	if (argc >= 2) {
		int toList = 0, isRecursive = 0;
		int toParse = 0;
		int toExtract = 0;
		int section = 0;
		int line = 0;
		int toFindAll = 0;
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "variant") == 0) {
				printVariant();
				return 0;
			}
			if (strcmp(argv[i], "list") == 0) {

				toList = 1;
			}
			if (strcmp(argv[i], "recursive") == 0) {

				isRecursive = 1;
			}
			if (strcmp(argv[i], "parse") == 0) {

				toParse = 1;
			}
			if (strcmp(argv[i], "findall") == 0) {

				toFindAll = 1;
			}
			if (strcmp(argv[i], "extract") == 0) {

				toExtract = 1;
			}
			if (strncmp(argv[i], "section", 7) == 0) {

				section = atoi(argv[i] + 8);
			}
			if (strncmp(argv[i], "line", 4) == 0) {

				line = atoi(argv[i] + 5);
			}
			if (strncmp(argv[i], "path=", 5) == 0) {

				dir_path = argv[i] + 5;
			}
			if (strncmp(argv[i], "name_ends_with", 14) == 0) {

				end = argv[i] + 15;
			}
			if (strncmp(argv[i], "permissions", 11) == 0) {

				permit = argv[i] + 12;
			}

		}
		if (toList == 1) {
			struct stat fileMetadata;
			if (stat(dir_path, &fileMetadata) < 0) {
				ERR_MSG("ERROR (getting info about the file)");
				exit(2);
			}
			if (!S_ISDIR(fileMetadata.st_mode)) {
				printf("ERROR\ninvalid directory path\n");
				exit(2);
			}
			printf("SUCCESS\n");
			if (isRecursive == 0) {
				listDir(dir_path);
				return 0;
			}
			listDirRec(dir_path);
			return 0;
		}
		if (toFindAll == 1) {
			struct stat fileMetadata;
			if (stat(dir_path, &fileMetadata) < 0) {
				ERR_MSG("ERROR (getting info about the file)");
				exit(2);
			}
			if (!S_ISDIR(fileMetadata.st_mode)) {
				printf("ERROR\ninvalid directory path\n");
				exit(2);
			}
			printf("SUCCESS\n");
			listSFRec(dir_path);
			return 0;
		}
		if (toParse == 1) {
			read_header(dir_path);
			return 0;
		}
		if (toExtract == 1) {
			extract_header(dir_path, section - 1, line);
			return 0;
		}
	}
	else
	{
		printf("USAGE: %s dir_name\n", argv[0]);
		exit(1);
	}
	return 0;
}