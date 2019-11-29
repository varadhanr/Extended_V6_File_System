Extended Version of V6 File system
----------------------------------

Objective : To create an extended version of V6 file sytem to support file size upto 4GB.

How to Compile and Execute:
--------------------------
1) Compile : gcc fsaccess.c -lm -o executable.out

2) Execute : ./executable.out

Commands that are supported:
---------------------------
1) initfs fname num_blocks num_inode </br>
   Initializes a v6 file system with num_blocks of blocks and num_inode of i-nodes
2) open fname </br>
   Opens the external file fname which has a v6 file system installed
3) cpin from_file_name to_file_name </br>
   Creates a new file called to_file_name in the v6 file system and fills in the contents of the newly created file with the contents of the external file from_file_name
4) cpout from_file_name to_file_name </br>
   If to_file_name exists in v6 file system,It create a external file to_file_name with from_file_name's content
5) rm v6_file </br>
   if v6_file exist, deletes the file
6) rmdir v6_directory </br>
   if v6_directory exists, deletes the directory
7) pwd </br>
   Lists the file path name of the current directory
8) ls </br>
   Lists the content of current directory
9) mkdir v6_directory </br>
   Creates a directory v6_directory in v6 file system
10) cd v6_directory </br>
   Change current working directory to the dir name
11) q </br>
   Exits the v6 file system
