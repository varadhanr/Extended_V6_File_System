//============================================================================
// Filename : fsaccess.c
// Team Members : Varadhan Ramamoorthy and Humayoon Akthar Qaimkhani
// UTD ID : 2021480952 and 2021505334
// NET ID: vrr180003 and hxq190001
// Class: OS 5348.001
// Project : Project 3
//============================================================================



#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#define LAST(k,n) ((k) & ((1<<(n))-1))
#define MID(k,m,n) LAST((k)>>(m),((n)-(m)))
#define FREE_SIZE 152
#define I_SIZE 200
#define BLOCK_SIZE 1024
#define ADDR_SIZE 11
#define I_SIZE 200
const unsigned short INODE_SIZE = 64;
/**
 * Structure for super block
 */
typedef struct {
	char flock;
	char ilock;
	unsigned short isize;
	unsigned short fsize;
	unsigned short nfree;
	unsigned short ninode;
	unsigned char fmode;
	unsigned short time[2];
	unsigned short inode[I_SIZE];
	unsigned int free[FREE_SIZE];

} superblock_type;

/*
 * Structure for free block
 */
typedef struct {
	unsigned short nfree;
	unsigned int free[FREE_SIZE];
} free_blocks_struct;

typedef struct {
	unsigned int allocated :1;
	unsigned int file_type :2;
	unsigned int large_file :1;
	unsigned int not_in_use :12;
} inode_flags;

/*
 * Structure for inode
 */
typedef struct {
	unsigned short flags;
	unsigned short nlinks;
	unsigned short uid;
	unsigned short gid;
	unsigned int size;
	unsigned int addr[ADDR_SIZE];
	unsigned short actime[2];
	unsigned short modtime[2];
} inode_type;

/*
 * Structure for directory entry
 */
typedef struct {
	unsigned short inode_offset;
	char file_name[14];
} dir_type;

//Funtion declarations

int initfs(int num_blocks, int num_inodes, FILE* file_system);
int cpin(const char* from_filename, char* to_filename, FILE* file_system,
		int inode_num);
int cpout(char* from_filename, const char* to_filename, FILE* file_system,
		int dir_inode);
int remove_file(char* filename, FILE* file_system, int dir_inode);
int traverse_file_path(FILE * file_system, int dir_inode, char *filePath);
int make_directory(char* filename, FILE* file_system, int dir_node);
void remove_directory(FILE* file_system, char * file_name, int dir_inode_num);
char * print_working_directory(FILE* file_system, int dir_inode_num, char * p);
int add_file_to_dir(const char* to_filename, FILE* file_system, int dir_inode);
int get_inode_by_file_name(const char* to_filename, FILE* file_system);
int get_inode_by_file_name_and_inode(const char* filename, FILE* file_system,
		int inode_num);
void list_directory(FILE* file_system, int dir_inode);
inode_type read_inode(int to_file_inode_num, FILE* file_system);
inode_type init_file_inode(int to_file_inode_num, unsigned int file_size,
		FILE* file_system);
int write_inode(int inode_num, inode_type inode, FILE* file_system);
void add_free_block(int block_num, FILE* file_system);
int get_free_block(FILE* file_system);
void copy_array(unsigned int *from_array, unsigned int *to_array,
		int buf_len);
void add_block_to_inode(int block_order_num, int block_num,
		int to_file_inode_num, FILE* file_system);
unsigned int lookup_blocks_for_large_file(int file_node_num,
		int block_number_order, FILE* file_system);
unsigned int get_inode_file_size(int to_file_inode_num, FILE* file_system);
void add_block_to_free_list(int next_block_number, FILE* file_system);
void remove_file_from_directory(int file_node_num, FILE* file_system,
		int dir_inode);
void add_block_to_inode_small_file(int block_order_num, int block_num,
		int to_file_inode_num, FILE* file_system);
unsigned int lookup_blocks_for_small_file(int file_node_num,
		int block_number_order, FILE* file_system);
int change_directory(FILE* file_system, char* filePath, int inode_number);
int traverse_file_path_for_make_directory(FILE * file_system, int inode_number,
		char *filePath);

// MAIN FUNCTIONS
//----------------
int main(int argc, char *argv[]) {
	int status;
	unsigned long long int file_size;
	FILE* file_system = NULL;
	const char *filename;
	int max = 200;
	char* cmmd = (char*) malloc(max); /* allocate buffer for user cmmd */
	char c;
	int cmd_counter = 0;
	printf("Extended V6 file system\n");
	int inode_number_for_dir_traversal = 1;
	while (1) {
		//While loop, wait for input
		cmd_counter++;
		printf(">>");
		int i = 0;
		while (1) {
			int c = getchar();
			if (c == '\n') {
				cmmd[i] = 0;
				break;
			}
			cmmd[i] = c;
			if (i == max - 1) {
				printf("cmmd is too long\n");
				exit(1);
			}
			i++;
		}
		char * cmd;
		char * arg;
		cmd = strtok(cmmd, " ,\n");
		if (cmd != NULL) {
			arg = strtok(NULL, "\n");
		}

		if (strcmp(cmmd, "q") == 0) {
			printf("Number of executed cmmds is..%i\n", cmd_counter);
			inode_number_for_dir_traversal = 1;
			fclose(file_system);
			exit(0);
		}
		// initfs command call
		if (strcmp(cmmd, "initfs") == 0) {
			char * p = strtok(arg, " ");
			filename = p;
			p = strtok(NULL, " ");
			long int num_blocks = atoi(p);
			p = strtok(NULL, " ");
			int num_inodes = atoi(p);
			printf("Init file_system was requested: %s\n", filename);
			file_system = fopen(filename, "w+");
			{
				status = initfs(num_blocks, num_inodes, file_system);
				if (status == 0)
					printf("\nFile system successfully initialized\n");
				else
					printf("\nFile system initialization failed\n");
			}
		}//open command call
		else if (strcmp(cmmd, "open") == 0) {
			char * p = strtok(arg, " ");
			if (access(p, F_OK) != -1) {
				printf("\nFile system %s exists. Opening...\n", p);
				file_system = fopen(p, "r+");
				fseek(file_system, 0L, SEEK_END);
				file_size = ftell(file_system);
				if (file_size == 0) {
					printf(
							"\nFile system %s doesn't exists. You need to run initfs cmmd\n",
							p);
				} else {
					printf("\nFile size is %llu\n", file_size);
					rewind(file_system);
				}
			} else {
				printf(
						"File doest not exist, use initfs to initialize file system\n");
			}

		}
		//cpin command call
		else if (strcmp(cmmd, "cpin") == 0) {
			if (file_system == NULL) {
				printf("No file system initialized or opened\n");
				continue;
			}
			printf("\ndir node for traversal : %i\n",
					inode_number_for_dir_traversal);
			printf("cpin was requested\n");
			char * p = strtok(arg, " ");
			const char *from_filename = p;
			p = strtok(NULL, " ");
			char *to_filename = p;
			status = cpin(from_filename, to_filename, file_system,
					inode_number_for_dir_traversal);
			if (status == 0)
				printf("\nFile  successfully copied\n");
			else
				printf("\nFile copy failed\n");

		}
		//cpout command call
		else if (strcmp(cmmd, "cpout") == 0) {
			if (file_system == NULL) {
				printf("No file system initialized or opened\n");
				continue;
			}
			char * p = strtok(arg, " ");
			char *from_filename = p;
			p = strtok(NULL, " ");
			char *to_filename = p;
			status = cpout(from_filename, to_filename, file_system,
					inode_number_for_dir_traversal);
			if (status == 0)
				printf("\nFile %s successfully copied\n", from_filename);
			else
				printf("\nFile copy failed\n");
		}
		//rm command call
		else if (strcmp(cmmd, "rm") == 0) {
			if (file_system == NULL) {
				printf("No file system initialized or opened\n");
				continue;
			}
			printf("rm was requested\n");
			char * p = strtok(arg, " ");
			char *filename = p;
			status = remove_file(filename, file_system,
					inode_number_for_dir_traversal);
			if (status == 0)
				printf("\nFile  successfully removed\n");
			else
				printf("\nFile removal failed\n");
		}
		//mkdir command call
		else if (strcmp(cmmd, "mkdir") == 0) {
			if (file_system == NULL) {
				printf("No file system initialized or opened\n");
				continue;
			}
			printf("mkdir was requested\n");
			char * p = strtok(arg, " ");
			char *filename = p;
			status = make_directory(filename, file_system,
					inode_number_for_dir_traversal);
			if (status == 0)
				printf("\nDirectory %s successfully created\n", filename);
			else
				printf("\nDirectory creation failed\n");
		}
		//rmdir command call
		else if (strcmp(cmmd, "rmdir") == 0) {
			if (file_system == NULL) {
				printf("No file system initialized or opened\n");
				continue;
			}
			printf("rmdir was requested\n");
			char * p = strtok(arg, " ");
			remove_directory(file_system, p, inode_number_for_dir_traversal);

		}
		//pwd command call
		else if (strcmp(cmmd, "pwd") == 0) {
			if (file_system == NULL) {
				printf("No file system initialized or opened\n");
				continue;
			}
			char *p = " ";
			printf(
					"\nCurrent Directory : %s\n",
					print_working_directory(file_system,
							inode_number_for_dir_traversal, p));

		}
		//cd command call
		else if (strcmp(cmmd, "cd") == 0) {
			if (file_system == NULL) {
				printf("No file system initialized or opened\n");
				continue;
			}
			char * p = strtok(arg, " ");
			if (p == NULL)
				printf("\nInvalid cd argument\n");
			else
				inode_number_for_dir_traversal = change_directory(file_system,
						p, inode_number_for_dir_traversal);

		}
		//ls command call
		else if (strcmp(cmmd, "ls") == 0) {
			if (file_system == NULL) {
				printf("No file system initialized or opened\n");
				continue;
			}
			list_directory(file_system, inode_number_for_dir_traversal);

		} else
			printf(
					"Not valid commands. Available cmmds: initfs, pwd, cd, ls, mkdir, rm, cpin, cpout, rmdir\n");

	}
}

/**
 * Function to print the list of files in the given directory inode
 */
char* print_working_directory(FILE* file_system, int dir_inode_num, char * p) {
	static char * front_slash = "/";
	char * fileS;
	inode_type dir_inode;
	dir_type dir_entry;
	dir_inode = read_inode(dir_inode_num, file_system);
	fseek(file_system, (BLOCK_SIZE * dir_inode.addr[0]), SEEK_SET);
	//read the first entry
	fread(&dir_entry, sizeof(dir_entry), 1, file_system);
	//sprintf(fileString,"%s%s",dir_entry.file_name,p);
	// read second entry
	fread(&dir_entry, sizeof(dir_entry), 1, file_system);
	if (dir_entry.inode_offset == dir_inode_num) {
		int l1 = strlen(front_slash);
		int l2 = strlen(p);
		fileS = (char *)calloc(l1 + l2 + 1, sizeof(char));
		strcat(fileS, front_slash);
		strcat(fileS, p);
		return fileS;
	}
	//read previous directory
	inode_type prev_dir;
	dir_type prev_dir_entry;
	prev_dir = read_inode(dir_entry.inode_offset, file_system);

	fseek(file_system, (BLOCK_SIZE * prev_dir.addr[0]), SEEK_SET);
	int records = (BLOCK_SIZE - 2) / sizeof(prev_dir_entry);
	int i;
	for (i = 0; i < records; i++) {
		fread(&prev_dir_entry, sizeof(prev_dir_entry), 1, file_system);
		if (prev_dir_entry.inode_offset == dir_inode_num) {
			//sprintf(fileString,"%s%s",prev_dir_entry.file_name,p);
			fileS = (char *)calloc(
					strlen(front_slash) + strlen(prev_dir_entry.file_name)
							+ strlen(p) + 1, sizeof(char));
			strcat(fileS, front_slash);
			strcat(fileS, prev_dir_entry.file_name);
			strcat(fileS, p);
			break;
		}

	}
	return print_working_directory(file_system, dir_entry.inode_offset, fileS);

}

/**
 * Function to remove a directory and files within it recursively for a given directory inode
 */
void remove_directory(FILE* file_system, char * file_name, int dir_inode_num) {

	dir_inode_num = traverse_file_path_for_make_directory(file_system,
			dir_inode_num, file_name);
	if (dir_inode_num == 0) {
		printf("Remove directory failed \n");
		return;
	}
	printf("\nDir inode : %d\n", dir_inode_num);
	char * filenamecpy;
	filenamecpy = strrchr(file_name, '/');
	if (filenamecpy != NULL) {
		file_name = filenamecpy + 1;
	}

	inode_type dir_inode;
	int inode_num_dir_to_be_deleted = 0;
	dir_type dir_entry;
	dir_inode = read_inode(dir_inode_num, file_system);
	fseek(file_system, (BLOCK_SIZE * dir_inode.addr[0]), SEEK_SET);
	int records = (BLOCK_SIZE - 2) / sizeof(dir_entry);
	int i;
	for (i = 0; i < records; i++) {
		fread(&dir_entry, sizeof(dir_entry), 1, file_system);
		if (strcmp(dir_entry.file_name, file_name) == 0) {
			inode_num_dir_to_be_deleted = dir_entry.inode_offset;
			break;
		}
	}

	if (inode_num_dir_to_be_deleted == 0) {
		//dir not found
		printf("\nDirectory %s not found\n", file_name);
		return;
	}

	unsigned char bit_14;
	inode_type dir_inode_to_be_deleted;
	dir_type directory_entry;
	dir_inode_to_be_deleted = read_inode(inode_num_dir_to_be_deleted,
			file_system);
	bit_14 = MID(dir_inode_to_be_deleted.flags,14,15);
	if (bit_14 != 1) {
		printf("\nNot a direcotry\n");
		return; //Not a directory
	}
	fseek(file_system, (BLOCK_SIZE * dir_inode_to_be_deleted.addr[0]), SEEK_SET);
	records = (BLOCK_SIZE - 2) / sizeof(directory_entry);
	int j;
	for (j = 0; j < records; j++) {
		fread(&directory_entry, sizeof(directory_entry), 1, file_system);
		inode_type file_inode;
		if (directory_entry.inode_offset != 0) {
			//printf("\nfile name : %s\n",directory_entry.file_name);
			file_inode = read_inode(directory_entry.inode_offset, file_system);
			if (MID(file_inode.flags,14,15) == 0) {
				remove_file(directory_entry.file_name, file_system,
						inode_num_dir_to_be_deleted);
			} else if (MID(file_inode.flags,14,15) == 1 && strcmp(
					directory_entry.file_name, ".") != 0 && strcmp(
					directory_entry.file_name, "..") != 0) {
				remove_directory(file_system, directory_entry.file_name,
						inode_num_dir_to_be_deleted);
			}
		}
	}

	remove_file_from_directory(inode_num_dir_to_be_deleted, file_system,
			dir_inode_num);
	fflush(file_system);

}

/**
 * Function that returns the inode for a given path in the file system
 */
int change_directory(FILE* file_system, char* filePath, int inode_number) {

	//handle absolute path from root
	if (filePath[0] == '/')
		inode_number = 1;

	char *p;
	p = strtok(filePath, "/");
	inode_type dir_inode;
	int curr_inode_number = inode_number;
	while (p) {
		printf("%s\n", p);
		int found = 0;
		inode_type directory_inode;
		dir_type directory_entry;
		directory_inode = read_inode(inode_number, file_system);
		fseek(file_system, (BLOCK_SIZE * directory_inode.addr[0]), SEEK_SET);
		int records = (BLOCK_SIZE - 2) / sizeof(directory_entry);
		int i;
		for (i = 0; i < records; i++) {
			fread(&directory_entry, sizeof(directory_entry), 1, file_system);
			if (strcmp(directory_entry.file_name, p) == 0) {
				inode_number = directory_entry.inode_offset;
				dir_inode = read_inode(inode_number, file_system);
				if (MID(dir_inode.flags,14,15) == 1) {
					found = 1;
					break;
				}
			}
		}
		if (found == 0) {
			printf("\nDirectory not found\n");
			return curr_inode_number;
		}
		p = strtok(NULL, "/");
	}

	//printf("\n Directory inode number :%d\n",inode_number);
	return inode_number;
}

/**
 * Funtion to initialize v6 file system
 *
 */
int initfs(int num_blocks, int num_inodes, FILE* file_system) {

//	printf("Value %i\n",d);
	inode_type i_node;
	inode_flags flags;
	char buff[BLOCK_SIZE];
	superblock_type block1;
	dir_type directory_entry;
	memset(buff, 0, BLOCK_SIZE);
	printf(
			"\nInitialize file_system with %i number of blocks and %i number of i-nodes,size of i-node %lu %lu %lu\n",
			num_blocks, num_inodes, sizeof(i_node), sizeof(flags),
			sizeof(block1));
	//FILE *file_system = NULL;
	int i;
	rewind(file_system);
	for (i = 0; i < num_blocks; i++) {
		fwrite(buff, 1, BLOCK_SIZE, file_system);
	}

	//Initialize Super block struct
	block1.isize = 0;
	block1.fsize = 0;
	block1.nfree = 1;
	block1.free[0] = 0;
	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fwrite(&block1, BLOCK_SIZE, 1, file_system);
	int number_inode_blocks = num_inodes / 16;
	int start_free_block = 2 + number_inode_blocks + 1;
	int next_free_block;
	printf("\nTotal number of data blocks : %d\n",
			num_blocks - start_free_block);

	for (next_free_block = start_free_block; next_free_block < num_blocks; next_free_block++) {

		add_free_block(next_free_block, file_system);
	}

	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fread(&block1, sizeof(block1), 1, file_system);
	block1.ninode = I_SIZE;
	int next_free_inode = 1;
	for (i = 0; i < I_SIZE; i++) {
		block1.inode[i] = next_free_inode;
		next_free_inode++;
	}

	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fwrite(&block1, sizeof(block1), 1, file_system);

	//initialize inode
	inode_type first_inode;
	first_inode.flags = 0;
	first_inode.flags |= 1 << 15;
	first_inode.flags |= 1 << 14;
	first_inode.flags |= 0 << 13;
	first_inode.nlinks = 0;
	first_inode.uid = 0;
	first_inode.gid = 0;
	first_inode.size = 16 * 2;
	first_inode.addr[0] = 0;
	first_inode.addr[1] = 0;
	first_inode.addr[2] = 0;
	first_inode.addr[3] = 0;
	first_inode.addr[4] = 0;
	first_inode.addr[5] = 0;
	first_inode.addr[6] = 0;
	first_inode.addr[7] = 0;
	first_inode.addr[8] = 0;
	first_inode.addr[9] = 0;
	first_inode.addr[10] = 0;
	first_inode.actime[0] = 0;
	first_inode.actime[1] = 0;
	first_inode.modtime[0] = 0;
	first_inode.modtime[1] = 0;

	directory_entry.inode_offset = 1;
	strcpy(directory_entry.file_name, ".");
	int dir_file_block = start_free_block - 1;
	fseek(file_system, dir_file_block * BLOCK_SIZE, SEEK_SET);
	fwrite(&directory_entry, 16, 1, file_system);
	strcpy(directory_entry.file_name, "..");
	fwrite(&directory_entry, 16, 1, file_system);

	printf("\nDirectory in the block %i", dir_file_block);
	first_inode.addr[0] = dir_file_block;
	write_inode(1, first_inode, file_system);

	return 0;
}

/**
 * Helper function that adds free blocks to the free list
 */
void add_free_block(int block_num, FILE* file_system) {
	superblock_type block1;
	free_blocks_struct copy_to_block;
	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fread(&block1, sizeof(block1), 1, file_system);
	if (block1.nfree == FREE_SIZE) {
		copy_to_block.nfree = FREE_SIZE;
		copy_array(block1.free, copy_to_block.free, FREE_SIZE);
		fseek(file_system, block_num * BLOCK_SIZE, SEEK_SET);
		fwrite(&copy_to_block, sizeof(copy_to_block), 1, file_system);
		block1.nfree = 1;
		block1.free[0] = block_num;
	} else {
		block1.free[block1.nfree] = block_num;
		block1.nfree++;
	}
	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fwrite(&block1, sizeof(block1), 1, file_system);
}

void copy_array(unsigned int *from_array, unsigned int *to_array,
		int buf_len) {
	int i;
	for (i = 0; i < buf_len; i++) {
		to_array[i] = from_array[i];
	}
}

/**
 * Funtion to copy in the files to the v6 file system
 */
int cpin(const char* from_filename, char* to_filename, FILE* file_system,
		int inode_num) {
	printf("\nInside cpin, copy from %s to %s \n", from_filename, to_filename);

	inode_num = traverse_file_path_for_make_directory(file_system, inode_num,
			to_filename);
	if (inode_num == 0)
		return -1;
	printf("\nDir inode : %d\n", inode_num);
	char * filenamecpy;
	filenamecpy = strrchr(to_filename, '/');
	if (filenamecpy != NULL) {
		to_filename = filenamecpy + 1;
	}

	inode_type to_file_inode;
	int to_file_inode_num, status_ind, num_bytes_read, free_block_num,
			file_node_num;
	unsigned long file_size;
	FILE* from_file_fd;
	unsigned char read_buffer[BLOCK_SIZE];

	file_node_num = get_inode_by_file_name_and_inode(to_filename, file_system,
			inode_num);
	if (file_node_num != -1) {
		printf("\nFile %s already exists. Choose different name", to_filename);
		return -1;
	}

	if (access(from_filename, F_OK) != -1) {
		printf("\nCopy From File %s exists. Trying to open...\n", from_filename);
		from_file_fd = fopen(from_filename, "rb");
		fseek(from_file_fd, 0L, SEEK_END); //Move to the end of the file
		file_size = ftell(from_file_fd);
		if (file_size == 0) {
			printf(
					"\nCopy from File %s doesn't exists. Type correct file name\n",
					from_filename);
			return -1;
		} else {
			printf("\nCopy from File size is %lu\n", file_size);
			rewind(file_system);
		}
	} else {
		printf("\nCopy from File %s doesn't exists. Type correct file name\n",
				from_filename);
		return -1;
	}

	if (file_size > pow(2, 32)) {
		printf("\nThe file is too big %lu, maximum supported size is 4GB",
				file_size);
		return 0;
	}
	to_file_inode_num = add_file_to_dir(to_filename, file_system, inode_num);
	printf("\nto_file_inode_num : %d\n", to_file_inode_num);
	to_file_inode = init_file_inode(to_file_inode_num, file_size, file_system);
	status_ind = write_inode(to_file_inode_num, to_file_inode, file_system);
	int num_blocks_read = 1;
	int total_num_blocks = 0;
	fseek(from_file_fd, 0L, SEEK_SET);
	int block_order = 0;
	FILE* debug_cpin = fopen("cpin_debug.txt", "w");
	printf("\nDebug point\n");
	while (num_blocks_read == 1) {
		// Read one block at a time from src file
		num_blocks_read = fread(&read_buffer, BLOCK_SIZE, 1, from_file_fd);
//		if (num_blocks_read != 1) {
//			break;
//		}
		total_num_blocks += num_blocks_read;
		free_block_num = get_free_block(file_system);
		fflush(debug_cpin);
		if (free_block_num == -1) {
			printf("\nNo free blocks left. Total blocks read so far:%i",
					total_num_blocks);
			return -1;
		}
		char buff[BLOCK_SIZE];
		memset(buff, 0, BLOCK_SIZE);
		fseek(file_system, free_block_num * BLOCK_SIZE, SEEK_SET);
		fwrite(buff, 1, BLOCK_SIZE, file_system);
//		fprintf(debug_cpin, "\n Block allocated %i", free_block_num);
		if (file_size > BLOCK_SIZE * 10)
			add_block_to_inode(block_order, free_block_num, to_file_inode_num,
					file_system);
		else
			add_block_to_inode_small_file(block_order, free_block_num,
					to_file_inode_num, file_system);
		fseek(file_system, free_block_num * BLOCK_SIZE, SEEK_SET);
		fwrite(&read_buffer, sizeof(read_buffer), 1, file_system);
		block_order++;
		fprintf(debug_cpin,
						"\nBlock allocated %i, Order num %i",
						free_block_num, block_order);

	}
	fclose(debug_cpin);
	return 0;
}

/*
 * Funtion that deletes a file  with given filename
 *
 */
int remove_file(char *filename, FILE* file_system, int dir_inode) {

	printf("\nInside rm, remove file %s", filename);
	printf("\nFilename : %s\n", filename);

	dir_inode = traverse_file_path(file_system, dir_inode, filename);

	if (dir_inode == 0)
		return -1;
	char * filenamecpy;
	filenamecpy = strrchr(filename, '/');
	if (filenamecpy != NULL) {
		filename = filenamecpy + 1;
	}
	int file_node_num, file_size, block_number_order, next_block_number;
	inode_type file_inode;
	unsigned char bit_14; //Plain file or Directory bit
	file_node_num = get_inode_by_file_name_and_inode(filename, file_system,
			dir_inode);
	if (file_node_num == -1) {
		printf("\nFile %s not found", filename);
		return -1;
	}
	file_inode = read_inode(file_node_num, file_system);
	bit_14 = MID(file_inode.flags,14,15);
	if (bit_14 == 0) { //Plain file
		printf("\nRemove Plain file");
		file_size = get_inode_file_size(file_node_num, file_system);
		printf("\nFile size:%d\n", file_size);
		block_number_order = file_size / BLOCK_SIZE;
		if (file_size % BLOCK_SIZE != 0)
			block_number_order++;

		block_number_order--;
		while (block_number_order > 0) {
			if (file_size > BLOCK_SIZE * 11)
				next_block_number = lookup_blocks_for_large_file(file_node_num,
						block_number_order, file_system);
			else
				next_block_number = file_inode.addr[block_number_order];

			add_block_to_free_list(next_block_number, file_system);
			block_number_order--;
		}
	}
	file_inode.flags = 0;
	write_inode(file_node_num, file_inode, file_system);
	remove_file_from_directory(file_node_num, file_system, dir_inode);
	fflush(file_system);
	return 0;
}

/**
 * Helper function for traversing a path in file system
 */
int traverse_file_path_for_make_directory(FILE * file_system, int inode_number,
		char *filePath) {

	char fileName[200];
	strcpy(fileName, filePath);
	char * ptr = strrchr(fileName, '/');
	if (ptr)
		*ptr = '\0';
	else
		return inode_number;

	if (fileName[0] == '/')
		inode_number = 1;

	char *p;
	p = strtok(fileName, "/");
	inode_type dir_inode;
	int curr_inode_number = inode_number;
	inode_type directory_inode;
	dir_type directory_entry;
	while (p) {
		printf("\n%s\n", p);
		int found = 0;
		directory_inode = read_inode(inode_number, file_system);
		fseek(file_system, (BLOCK_SIZE * directory_inode.addr[0]), SEEK_SET);
		int records = (BLOCK_SIZE - 2) / sizeof(directory_entry);
		int i;
		for (i = 0; i < records; i++) {
			fread(&directory_entry, sizeof(directory_entry), 1, file_system);
			if (strcmp(directory_entry.file_name, p) == 0) {

				dir_inode = read_inode(directory_entry.inode_offset,
						file_system);
				if (MID(dir_inode.flags,14,15) == 1) {
					inode_number = directory_entry.inode_offset;
					found = 1;
					break;
				}
			}
		}
		if (found == 0 && strtok(NULL, "/") != NULL) {
			printf("\nPath not found\n");
			return 0;
		}
		p = strtok(NULL, "/");
	}

	//printf("\n Directory inode number :%d\n",inode_number);
	return inode_number;

}
/*
 * Helper function for traversing a path in file system
 */
int traverse_file_path(FILE * file_system, int inode_number, char *filePath) {

	char fileName[200];
	strcpy(fileName, filePath);
	if (fileName[0] == '/')
		inode_number = 1;

	char *p;
	p = strtok(fileName, "/");
	inode_type dir_inode;
	int curr_inode_number = inode_number;
	inode_type directory_inode;
	dir_type directory_entry;
	while (p) {
		printf("\n%s\n", p);
		int found = 0;
		directory_inode = read_inode(inode_number, file_system);
		fseek(file_system, (BLOCK_SIZE * directory_inode.addr[0]), SEEK_SET);
		int records = (BLOCK_SIZE - 2) / sizeof(directory_entry);
		int i;
		for (i = 0; i < records; i++) {
			fread(&directory_entry, sizeof(directory_entry), 1, file_system);
			if (strcmp(directory_entry.file_name, p) == 0) {

				dir_inode = read_inode(directory_entry.inode_offset,
						file_system);
				if (MID(dir_inode.flags,14,15) == 1) {
					inode_number = directory_entry.inode_offset;
					found = 1;
					break;
				}
			}
		}
		if (found == 0 && strtok(NULL, "/") != NULL) {
			printf("\nPath not found\n");
			return 0;
		}
		p = strtok(NULL, "/");
	}

	//printf("\n Directory inode number :%d\n",inode_number);
	return inode_number;

}

/*
 * Funtion which allocates the addr array of a particular inode for large files
 */
void add_block_to_inode(int block_order_num, int block_num,
		int to_file_inode_num, FILE* file_system) {

	inode_type file_inode;
	int logical_block, prev_logical_block, word_in_block, free_block_num,
			word_in_sec, second_block_num, word_in_third;
	int block_num_tow1 = block_num;
	unsigned int sec_ind_block;
	file_inode = read_inode(to_file_inode_num, file_system);
	logical_block = block_order_num / 256;
	prev_logical_block = (block_order_num - 1) / 256;
	word_in_block = block_order_num % 256;
	if (word_in_block == 0 && logical_block <= 10) {
		free_block_num = get_free_block(file_system);
		file_inode.addr[logical_block] = free_block_num;
	}

	if (logical_block < 10) {
		fseek(
				file_system,
				file_inode.addr[logical_block] * BLOCK_SIZE + word_in_block * 4,
				SEEK_SET);
		fwrite(&block_num, sizeof(block_num), 1, file_system);
	}

	//	// Working code for double indirect block
	//
	//	if (logical_block >= 10) {
	//
	//		if (logical_block > prev_logical_block) {
	//			sec_ind_block = get_free_block(file_system);
	//			fseek(
	//					file_system,
	//					file_inode.addr[10] * BLOCK_SIZE + (logical_block - 10) * 4,
	//					SEEK_SET);
	//			fwrite(&sec_ind_block, sizeof(sec_ind_block), 1, file_system);
	//			fflush(file_system);
	//		} else {
	//
	//			fseek(
	//					file_system,
	//					file_inode.addr[10] * BLOCK_SIZE + (logical_block - 10) * 4,
	//					SEEK_SET);
	//			fread(&sec_ind_block, sizeof(sec_ind_block), 1, file_system);
	//		}
	//
	//		// Write target block number into second indirect block
	//		fseek(file_system, sec_ind_block * BLOCK_SIZE + word_in_block * 4,
	//				SEEK_SET);
	//		fwrite(&block_num_tow, sizeof(block_num_tow), 1, file_system);
	//	}


	/*
	 *
	 *
	 * TRIPLE INDIRECT BLOCK
	 *
	 *
	 *
	 *
	 */
	int a = floor((block_order_num - 256 * 10) / (256 * 256));
	int prev_a = floor(((block_order_num - 1) - 256 * 10) / (256 * 256));
	int b = floor( ((double)(block_order_num - 256 * 10 - (a * 256 * 256))) / (256));
	int prev_b = floor( ((double)(block_order_num - 1 - 256 * 10 - (a * 256 * 256))) / (256));
	int thrd_ind_block;
	if (logical_block >= 10) {

		if (block_order_num == 256 * 10) {
			thrd_ind_block = get_free_block(file_system);
			fseek(file_system, file_inode.addr[10] * BLOCK_SIZE, SEEK_SET);
			fwrite(&thrd_ind_block, sizeof(thrd_ind_block), 1, file_system);
		} else if (a > prev_a) {
			thrd_ind_block = get_free_block(file_system);
			fseek(file_system, file_inode.addr[10] * BLOCK_SIZE + a * 4,
					SEEK_SET);
			fwrite(&thrd_ind_block, sizeof(thrd_ind_block), 1, file_system);
		} else {
			fseek(file_system, file_inode.addr[10] * BLOCK_SIZE + a * 4,
					SEEK_SET);
			fread(&thrd_ind_block, sizeof(thrd_ind_block), 1, file_system);
		}

		if (block_order_num == 256 * 10) {
			sec_ind_block = get_free_block(file_system);
			fseek(file_system, thrd_ind_block * BLOCK_SIZE, SEEK_SET);
			fwrite(&sec_ind_block, sizeof(sec_ind_block), 1, file_system);
		} else if (b > prev_b) {
			sec_ind_block = get_free_block(file_system);
			fseek(file_system, thrd_ind_block * BLOCK_SIZE + b * 4, SEEK_SET);
			fwrite(&sec_ind_block, sizeof(sec_ind_block), 1, file_system);
		} else {
			fseek(file_system, thrd_ind_block * BLOCK_SIZE + b * 4, SEEK_SET);
			fread(&sec_ind_block, sizeof(&sec_ind_block), 1, file_system);
		}


		fseek(file_system, sec_ind_block * BLOCK_SIZE + word_in_block * 4,
				SEEK_SET);
		fwrite(&block_num, sizeof(block_num), 1, file_system);

	}
	write_inode(to_file_inode_num, file_inode, file_system);

}

/*
 * Funtion which allocates the addr array of a particular inode for small files
 */
void add_block_to_inode_small_file(int block_order_num, int block_num,
		int to_file_inode_num, FILE* file_system) {
	printf("\nSmall file\n");
	inode_type file_inode;
	unsigned int block_num_tow = block_num;
	file_inode = read_inode(to_file_inode_num, file_system);
	file_inode.addr[block_order_num] = block_num_tow;
	write_inode(to_file_inode_num, file_inode, file_system);
	return;
}
/*
 * Helper funtions that retrieves free blocks from nfree array
 */
int get_free_block(FILE* file_system) {
	superblock_type block1;
	free_blocks_struct copy_from_block;
	int free_block;

	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fread(&block1, sizeof(block1), 1, file_system);
	block1.nfree--;
	free_block = block1.free[block1.nfree];
	if (free_block == 0) {
		printf("(\nNo free blocks left");
		fseek(file_system, BLOCK_SIZE, SEEK_SET);
		fwrite(&block1, sizeof(block1), 1, file_system);
		fflush(file_system);
		return -1;
	}

	if (block1.nfree == 0) {
		fseek(file_system, BLOCK_SIZE * block1.free[block1.nfree], SEEK_SET);
		fread(&copy_from_block, sizeof(copy_from_block), 1, file_system);
		block1.nfree = copy_from_block.nfree;
		copy_array(copy_from_block.free, block1.free, FREE_SIZE);
		block1.nfree--;
		free_block = block1.free[block1.nfree];
	}

	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fwrite(&block1, sizeof(block1), 1, file_system);
	fflush(file_system);
//	free(&block1);
	return free_block;
}

/*
 * Helper funtions that add a free block to the nfree array
 *
 */
void add_block_to_free_list(int freed_block_number, FILE* file_system) {
	superblock_type block1;
	free_blocks_struct copy_to_block;

	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fread(&block1, sizeof(block1), 1, file_system);
	if (block1.nfree < FREE_SIZE) {
		block1.free[block1.nfree] = freed_block_number;
		block1.nfree++;
	} else {
		copy_array(block1.free, copy_to_block.free, FREE_SIZE);
		copy_to_block.nfree = FREE_SIZE;
		fseek(file_system, BLOCK_SIZE * freed_block_number, SEEK_SET);
		fwrite(&copy_to_block, sizeof(copy_to_block), 1, file_system);
		block1.nfree = 1;
		block1.free[0] = freed_block_number;
	}
	fseek(file_system, BLOCK_SIZE, SEEK_SET);
	fwrite(&block1, sizeof(block1), 1, file_system);
	fflush(file_system);

	return;
}

/*
 * Funtion to create a directory with given name as filename
 *
 */
int make_directory(char* filename, FILE* file_system, int dir_node) {

	printf("\nInside mk :\n");
	dir_node = traverse_file_path_for_make_directory(file_system, dir_node,
			filename);
	if (dir_node == 0)
		return -1;
	printf("\nDir inode : %d\n", dir_node);
	char * filenamecpy;
	filenamecpy = strrchr(filename, '/');
	if (filenamecpy != NULL) {
		filename = filenamecpy + 1;
	}
	printf("\nFilename : %s\n", filename);
	inode_type directory_inode, free_node;
	dir_type directory_entry;
	int to_file_inode_num, to_file_first_block, flag, status_ind, file_node_num;

	file_node_num = get_inode_by_file_name_and_inode(filename, file_system,
			dir_node);
	if (file_node_num != -1) {
		printf("\nDirectory %s already exists. Choose different name", filename);
		return -1;
	}

	int found = 0;
	to_file_inode_num = 1;
	while (found == 0) {
		to_file_inode_num++;
		free_node = read_inode(to_file_inode_num, file_system);
		flag = MID(free_node.flags,15,16);
		if (flag == 0) {
			//printf("\nFree inode found:%i",to_file_inode_num );
			found = 1;
		}
	}

	directory_inode = read_inode(dir_node, file_system);

	// Move to the end of directory file
	printf("\nDirectory node block number is %i", directory_inode.addr[0]);
	fseek(file_system,
			(BLOCK_SIZE * directory_inode.addr[0] + directory_inode.size),
			SEEK_SET);
	// Add record to file directory
	directory_entry.inode_offset = to_file_inode_num;
	strcpy(directory_entry.file_name, filename);
	fwrite(&directory_entry, 16, 1, file_system);

	directory_inode.size += 16;
	write_inode(dir_node, directory_inode, file_system);

	//Initialize new directory file-node
	free_node.flags = 0;
	free_node.flags |= 1 << 15;
	free_node.flags |= 1 << 14;
	free_node.flags |= 0 << 13;
	free_node.size = 16 * 2;

	int free_block = get_free_block(file_system);
	char buff[BLOCK_SIZE];
	memset(buff, 0, BLOCK_SIZE);
	fseek(file_system, free_block * BLOCK_SIZE, SEEK_SET);
	fwrite(buff, 1, BLOCK_SIZE, file_system);

	dir_type new_dir;
	strcpy(new_dir.file_name, ".");
	new_dir.inode_offset = to_file_inode_num;
	fseek(file_system, free_block * BLOCK_SIZE, SEEK_SET);
	fwrite(&new_dir, 16, 1, file_system);
	strcpy(new_dir.file_name, "..");
	new_dir.inode_offset = dir_node;
	fwrite(&new_dir, 16, 1, file_system);

	free_node.addr[0] = free_block;

	status_ind = write_inode(to_file_inode_num, free_node, file_system);

	return 0;

}

/*
 * Helper funtion that add a file entry to its corresponding directory
 *
 */
int add_file_to_dir(const char* to_filename, FILE* file_system,
		int dir_inode_num) {

	inode_type directory_inode, free_node;
	dir_type directory_entry;
	int to_file_inode_num, to_file_first_block, flag;

	int found = 0;
	to_file_inode_num = 1;
	while (found == 0) {
		to_file_inode_num++;
		free_node = read_inode(to_file_inode_num, file_system);
		flag = MID(free_node.flags,15,16);
		if (flag == 0) {
			//printf("\nFree inode found:%i",to_file_inode_num );
			found = 1;
		}
	}

	directory_inode = read_inode(dir_inode_num, file_system);

	// Move to the end of directory file
	fseek(file_system,
			(BLOCK_SIZE * directory_inode.addr[0] + directory_inode.size),
			SEEK_SET);
	// Add record to file directory
	directory_entry.inode_offset = to_file_inode_num;
	strcpy(directory_entry.file_name, to_filename);
	fwrite(&directory_entry, 16, 1, file_system);

	directory_inode.size += 16;
	write_inode(dir_inode_num, directory_inode, file_system);

	return to_file_inode_num;

}

/*
 * Helper method to initialize an inode
 */
inode_type init_file_inode(int to_file_inode_num, unsigned int file_size,
		FILE* file_system) {
	inode_type to_file_inode;
	unsigned short bit0_15;
	unsigned char bit16_23;
	unsigned short bit24;

	bit0_15 = LAST(file_size,16);
	bit16_23 = MID(file_size,16,24);
	bit24 = MID(file_size,24,25);

	to_file_inode.flags = 0; //Initialize
	to_file_inode.flags |= 1 << 15; //Set first bit to 1 - i-node is allocated
	to_file_inode.flags |= 0 << 14; // Set 2-3 bits to 10 - i-node is plain file
	to_file_inode.flags |= 0 << 13;
	if (bit24 == 1) { // Set most significant bit of file size
		to_file_inode.flags |= 1 << 0;
	} else {
		to_file_inode.flags |= 0 << 0;
	}
	if (file_size <= 10 * BLOCK_SIZE) {
		to_file_inode.flags |= 0 << 12; //Set 4th bit to 0 - small file
	} else {
		to_file_inode.flags |= 1 << 12; //Set 4th bit to 0 - large file
	}
	to_file_inode.nlinks = 0;
	to_file_inode.uid = 0;
	to_file_inode.gid = 0;
	to_file_inode.size = file_size;
	to_file_inode.addr[0] = 0;
	to_file_inode.addr[1] = 0;
	to_file_inode.addr[2] = 0;
	to_file_inode.addr[3] = 0;
	to_file_inode.addr[4] = 0;
	to_file_inode.addr[5] = 0;
	to_file_inode.addr[6] = 0;
	to_file_inode.addr[7] = 0;
	to_file_inode.addr[8] = 0;
	to_file_inode.addr[9] = 0;
	to_file_inode.addr[10] = 0;
	to_file_inode.actime[0] = 0;
	to_file_inode.actime[1] = 0;
	to_file_inode.modtime[0] = 0;
	to_file_inode.modtime[1] = 0;
	return to_file_inode;
}

/*
 * Helper method to read an inode given its inode number
 */
inode_type read_inode(int to_file_inode_num, FILE* file_system) {
	inode_type to_file_inode;
	fseek(file_system, (BLOCK_SIZE * 2 + INODE_SIZE * (to_file_inode_num - 1)),
			SEEK_SET);
	fread(&to_file_inode, INODE_SIZE, 1, file_system);
	return to_file_inode;
}

/*
 * Helper method to write to an inode given inode number and inode_type
 */
int write_inode(int inode_num, inode_type inode, FILE* file_system) {
	fseek(file_system, (BLOCK_SIZE * 2 + INODE_SIZE * (inode_num - 1)),
			SEEK_SET); //move to the beginning of inode with inode_num
	fwrite(&inode, INODE_SIZE, 1, file_system);
	return 0;
}

/*
 * Function that copy out a file from file system
 */
int cpout(char* from_filename, const char* to_filename, FILE* file_system,
		int dir_inode) {

	dir_inode = traverse_file_path_for_make_directory(file_system, dir_inode,
			from_filename);
	if (dir_inode == 0)
		return -1;
	printf("\nDir inode : %d\n", dir_inode);
	char * filenamecpy;
	filenamecpy = strrchr(from_filename, '/');
	if (filenamecpy != NULL) {
		from_filename = filenamecpy + 1;
	}

	//printf("\nInside cpout, copy from %s to %s \n",from_filename, to_filename);
	int file_node_num;
	file_node_num = get_inode_by_file_name_and_inode(from_filename,
			file_system, dir_inode);
	if (file_node_num == -1) {
		printf("\nFile %s not found", from_filename);
		return -1;
	}

	FILE* write_to_file;
	unsigned char buffer[BLOCK_SIZE];
	int next_block_number, block_number_order, number_of_blocks, file_size;
	write_to_file = fopen(to_filename, "w");
	block_number_order = 0;
	file_size = get_inode_file_size(file_node_num, file_system);
	printf("\nFile size %i", file_size);
	int number_of_bytes_last_block = file_size % BLOCK_SIZE;
	unsigned char last_buffer[number_of_bytes_last_block];
	if (number_of_bytes_last_block == 0) {
		number_of_blocks = file_size / BLOCK_SIZE;
	} else
		number_of_blocks = file_size / BLOCK_SIZE + 1; //The last block is not full

	FILE * debug_cpout = fopen("cpout_debug.txt", "w");
	while (block_number_order < number_of_blocks) {
		if (file_size > BLOCK_SIZE * 10)
			next_block_number = lookup_blocks_for_large_file(file_node_num,
					block_number_order, file_system);
		else
			next_block_number = lookup_blocks_for_small_file(file_node_num,
					block_number_order, file_system);

//		if(next_block_number == 0)
//			return 0;
		fseek(file_system, next_block_number * BLOCK_SIZE, SEEK_SET);
		if ((block_number_order < (number_of_blocks - 1))
				|| (number_of_bytes_last_block == 0)) {
			fread(buffer, sizeof(buffer), 1, file_system);
			fwrite(buffer, sizeof(buffer), 1, write_to_file);
		} else {
			fread(last_buffer, sizeof(last_buffer), 1, file_system);
			fwrite(last_buffer, sizeof(last_buffer), 1, write_to_file);
		}

		block_number_order++;
		fprintf(debug_cpout,
				"\nBlock allocated %i, Order num %i",
				next_block_number, block_number_order);

	}

	fclose(write_to_file);
	return 0;
}

/*
 * Funtions that retrieves data block for large files from addr array
 */
unsigned int lookup_blocks_for_large_file(int file_node_num,
		int block_number_order, FILE* file_system) {
	inode_type file_inode;
	unsigned int block_num_tow;
	unsigned int sec_ind_block, triple_ind_block;

	file_inode = read_inode(file_node_num, file_system);
	int second_logical_block = block_number_order / (256 * 256);
	int logical_block = block_number_order / 256;
	int word_in_block = block_number_order % 256;
	//printf("\nIndirect address:%i, num in array %i",file_inode.addr[logical_block], logical_block);
	if (logical_block < 10) {

		fseek(
				file_system,
				file_inode.addr[logical_block] * BLOCK_SIZE + word_in_block * 4,
				SEEK_SET);
		fread(&block_num_tow, sizeof(block_num_tow), 1, file_system);

	} else {

		/***
		 *
		 * Triple Indirect block retrieval
		 *
		 *
		 */
		unsigned int a = (block_number_order - 256 * 10) / (256 * 256);
		unsigned int b = (block_number_order - 2560 - (a * 256 * 256)) / (256);
		unsigned int thrd_ind_block;
		fseek(file_system, file_inode.addr[10] * BLOCK_SIZE + a * 4, SEEK_SET);
		fread(&thrd_ind_block, sizeof(thrd_ind_block), 1, file_system);

		fseek(file_system, thrd_ind_block * BLOCK_SIZE + b * 4, SEEK_SET);
		fread(&sec_ind_block, sizeof(sec_ind_block), 1, file_system);

		fseek(file_system, sec_ind_block * BLOCK_SIZE + word_in_block * 4,
				SEEK_SET);
		fread(&block_num_tow, sizeof(block_num_tow), 1, file_system);

		//		// Working code for double indirect block
		//
		//		// Read block number of second indirect block
		//		fseek(file_system,
		//				file_inode.addr[10] * BLOCK_SIZE + (logical_block - 10) * 4,
		//				SEEK_SET);
		//		fread(&sec_ind_block, sizeof(sec_ind_block), 1, file_system);
		//
		//		//printf("\nFirst Block %i, Second block %i",file_inode.addr[7],sec_ind_block);
		//		// Read target block number from second indirect block
		//		fseek(file_system, sec_ind_block * BLOCK_SIZE + word_in_block * 4,
		//				SEEK_SET);
		//		fread(&block_num_tow, sizeof(block_num_tow), 1, file_system);
	}

	return block_num_tow;

}

/*
 * Funtions that retrieves data block for small files from addr array
 */
unsigned int lookup_blocks_for_small_file(int file_node_num,
		int block_number_order, FILE* file_system) {
	inode_type file_inode;
	file_inode = read_inode(file_node_num, file_system);
	return (file_inode.addr[block_number_order]);
}

/*
 * Helper method to calculate inode's file size given inode number
 */
unsigned int get_inode_file_size(int to_file_inode_num, FILE* file_system) {
	inode_type to_file_inode;

	to_file_inode = read_inode(to_file_inode_num, file_system);

	return to_file_inode.size;
}

/*
 * list_directory funtions is used to list the files in the directory
 */
void list_directory(FILE* file_system, int dir_inode) {

	int inode_number;
	inode_type directory_inode;
	printf("\nDir node : %d\n", dir_inode);
	dir_type directory_entry;
	directory_inode = read_inode(dir_inode, file_system);
	fseek(file_system, (BLOCK_SIZE * directory_inode.addr[0]), SEEK_SET);
	int records = (BLOCK_SIZE - 2) / sizeof(directory_entry);
	int i;
	for (i = 0; i < records; i++) {
		fread(&directory_entry, sizeof(directory_entry), 1, file_system);
		if (strlen(directory_entry.file_name) != 0
				&& directory_entry.inode_offset != 0) {
			printf("\nFile name :%s ", directory_entry.file_name);
		}
	}
	printf("\n");

}

/*
 * Helper method to get the inode number given filename and its directory inode
 */
int get_inode_by_file_name_and_inode(const char* filename, FILE* file_system,
		int inode_num) {
	int inode_number;
	inode_type directory_inode;
	dir_type directory_entry;

	directory_inode = read_inode(inode_num, file_system);

	fseek(file_system, (BLOCK_SIZE * directory_inode.addr[0]), SEEK_SET);
	int records = (BLOCK_SIZE - 2) / sizeof(directory_entry);
	int i;
	for (i = 0; i < records; i++) {
		fread(&directory_entry, sizeof(directory_entry), 1, file_system);
		if (strcmp(filename, directory_entry.file_name) == 0)
			return directory_entry.inode_offset;
	}
	printf("\nFile %s not found", filename);
	return -1;
}
/*
 * Helper method to get the inode number given filename
 */
int get_inode_by_file_name(const char* filename, FILE* file_system) {
	int inode_number;
	inode_type directory_inode;
	dir_type directory_entry;

	directory_inode = read_inode(1, file_system);

	fseek(file_system, (BLOCK_SIZE * directory_inode.addr[0]), SEEK_SET);
	int records = (BLOCK_SIZE - 2) / sizeof(directory_entry);
	int i;
	for (i = 0; i < records; i++) {
		fread(&directory_entry, sizeof(directory_entry), 1, file_system);
		if (strcmp(filename, directory_entry.file_name) == 0)
			return directory_entry.inode_offset;
	}
	printf("\nFile %s not found", filename);
	return -1;
}

/*
 * Funtion that removes a file entry from its directory
 */
void remove_file_from_directory(int file_node_num, FILE* file_system,
		int dir_inode) {
	inode_type directory_inode;
	dir_type directory_entry;

	directory_inode = read_inode(dir_inode, file_system);
	printf("\nDir i node:%d\n", dir_inode);

	fseek(file_system, (BLOCK_SIZE * directory_inode.addr[0]), SEEK_SET); //Move to third record in directory
	int records = (BLOCK_SIZE - 2) / sizeof(directory_entry);
	int i;
	for (i = 0; i < records; i++) {
		fread(&directory_entry, sizeof(directory_entry), 1, file_system);
		printf("\nNode number in directory %i", directory_entry.inode_offset);
		printf("\nFileName%s\n", directory_entry.file_name);
		if (directory_entry.inode_offset == file_node_num) {
			printf("\n Inside\n");
			fseek(file_system, (-1) * sizeof(directory_entry), SEEK_CUR); //Go one record back
			directory_entry.inode_offset = 0;
			memset(directory_entry.file_name, 0,
					sizeof(directory_entry.file_name));
			fwrite(&directory_entry, sizeof(directory_entry), 1, file_system);
			return;
		}
	}
	return;
}
