#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utils/utils.h"
#include "system.h"

#define LARGE_SPACE_SIZE 2048

// Show help if user enters invalid arguments
void help(void) {
	fprintf(stderr, "Please use the following syntax:\n");
	fprintf(stderr, "- extract: ./AE2pak.out filename.pak -e <directory/of/extracted/files>\n");
	fprintf(stderr, "- pack: ./AE2pak.out filename.pak -p _filelist.log\n\n");
}

void getFilename(char* sdata) {
	int i, k;
	char sdata2[LARGE_SPACE_SIZE];
	strcpy(sdata2,sdata);
	strrev(sdata2);
	k = strlen(sdata2);
	for (i=0;i<k;i++){
		if (sdata2[i]=='/'){
			sdata2[i]=0;
			break;
		}
	}
	strrev(sdata2);
	strcpy(sdata,sdata2);
	return;
}

char extract_file(const char* fo2s, char* fn2s, unsigned long int fileDataPos, unsigned long int fileSize) {
	FILE *fo2, *fn2;
	unsigned int i;
	unsigned char c1;

	fo2 = fopen(fo2s, "rb");
	if (!fo2) {
		fprintf(stderr, "ERROR: Could not open \"%s\" for extraction!\n", fo2s);
		return 0;
	}
	rewind(fo2);
	fseek(fo2, fileDataPos, 0);

	fn2 = fopen(fn2s, "wb");
	if (!fn2) {
		fprintf(stderr, "ERROR: Could not open \"%s\" for writing !\n", fn2s);
		fclose(fo2);
		return 0;
	}

	for (i = 0; i < fileSize; ++i) {
		c1 = getc(fo2);
		if (feof(fo2)) {
			fprintf(stderr, "ERROR: EOF reached when extracting!\n Check your file!");
			fclose(fo2);
			fclose(fn2);
			return 0;
		}
		putc(c1, fn2);
	}

	fclose(fo2);
	fclose(fn2);
	return 1;
}

void extract(const char* pakFile, const char* extractDir) {

	printf("Extracting...\n");

	// open .pak file for reading
	mkdir(extractDir, MKDIR_DEFAULT_MODE);
	FILE* pakFileDesc = fopen(pakFile, "rb");
	if (!pakFileDesc) {
		fprintf(stderr, "ERROR: PAK file \"%s\" not found.\n", pakFile);
		fclose(pakFileDesc);
		exit(ERROR_RW);
	}

	// get .log file in which to write extracted file list
	// place it in current working directory
	char fileListLOG[LARGE_SPACE_SIZE];
	strcpy(fileListLOG, "_filelist.log");
	printf("\nStoring file list in log file: \"%s\"\n\n", fileListLOG);

	// open the file list .log file for writing
	FILE* fileListDesc = fopen(fileListLOG, "wb");
	if (!fileListDesc) {
		fprintf(stderr, "ERROR: \"%s\" cannot be created for writing.\n", fileListLOG);
		fclose(pakFileDesc);
		fclose(fileListDesc);
		exit(ERROR_RW);
	}

	unsigned char c1, c2, c3, c4;

	// get starting position of file data (first 2 bytes)
	rewind(pakFileDesc);
	c1 = getc(pakFileDesc);
	c2 = getc(pakFileDesc);
	unsigned int fileDataStartPos = fourBytesToUnsignedInt(0, 0, c1, c2);
	printf("File data starts at byte: %d\n", fileDataStartPos);

	// get number of total files (next 2 bytes)
	c1 = getc(pakFileDesc);
	c2 = getc(pakFileDesc);
	unsigned int totalFiles = fourBytesToUnsignedInt(0, 0, c1, c2);
	printf("Total Files announced: %d\n", totalFiles);

	// check unexpected ending of .pak file
	if (feof(pakFileDesc)) {
		fclose(pakFileDesc);
		fprintf(stderr, "ERROR: Unexpected end of .pak file:\n\"%s\"\n", pakFile);
		fclose(pakFileDesc);
		fclose(fileListDesc);
		exit(ERROR_RW);
	}

	// process all files

	unsigned int totalExtracted = 0, totalErrors = 0;

	for (unsigned int i = 0; i < totalFiles; ++i) {
		// get filename length (2 bytes)
		c1 = getc(pakFileDesc);
		c2 = getc(pakFileDesc);
		unsigned int filenameLen = fourBytesToUnsignedInt(0, 0, c1, c2);

		// get filename
		unsigned int j = 0;
		char filename[LARGE_SPACE_SIZE];
		for (; j < filenameLen; ++j) {
			filename[j] = getc(pakFileDesc);
		}
		filename[j] = 0;

		// get file data position (4 bytes)
		c1 = getc(pakFileDesc);
		c2 = getc(pakFileDesc);
		c3 = getc(pakFileDesc);
		c4 = getc(pakFileDesc);
		unsigned int fileDataPos = fourBytesToUnsignedInt(c1, c2, c3, c4) + fileDataStartPos;

		// get file size (2 bytes)
		c1 = getc(pakFileDesc);
		c2 = getc(pakFileDesc);
		unsigned int fileSize = fourBytesToUnsignedInt(0, 0, c1, c2);

		// write to file list
		char line[LARGE_SPACE_SIZE];
		size_t extractDirLen = strlen(extractDir);
		strcpy(line, extractDir);
		if (line[extractDirLen-1] == DBQUOTE) {
			line[extractDirLen-1]=0;
		}
		else if (line[extractDirLen-1] == BACKSLASH) {
			line[extractDirLen-1]=0;
		}
		strcat(line, "/");
		strcat(line, filename);
		fprintf(fileListDesc, "%s\n", line);

		// Extract file
		if (!extract_file(pakFile, line, fileDataPos, fileSize)) {
			++totalErrors;
		}
		++totalExtracted;
	}

	// finish
	fclose(pakFileDesc);
	fclose(fileListDesc);

	printf("\nUh yeah, it's done!\n");
	printf("Total files: %d\n", totalFiles);
	printf("Total extracted: %d\n", totalExtracted);
	printf("Total errors: %d\n", totalErrors);
}

// void pack(void) {
// 	FILE *fo, *fo3, *fn, *fl;
// 	unsigned long int i, j, k, fileDataStartPos, totalFiles=0, totalExtracted=0, fileDataPos=0, fileSize, totalErrors=0;
// 	unsigned char c1, c2, c3, c4;
// 	char sdata[LARGE_SPACE_SIZE], sdata3[LARGE_SPACE_SIZE];
// 	char sdata2[LARGE_SPACE_SIZE][LARGE_SPACE_SIZE];
// 	unsigned int sdata2s[LARGE_SPACE_SIZE];
// }

int main(int argc, char *argv[]) {
	FILE *fo, *fo3, *fn;
	unsigned long int i, j, k, totalFiles=0, fileDataPos=0, totalErrors=0;
	unsigned char c1, c2, c3, c4;
	char sdata[LARGE_SPACE_SIZE];
	char sdata2[LARGE_SPACE_SIZE][LARGE_SPACE_SIZE];
	unsigned int sdata2s[LARGE_SPACE_SIZE];


	// Program title
	printf("\n=== Ancient Empires II packer / unpacker v0.11b ===\n\n");

	if (argc < 3) {
		help();
		exit(ERROR_ARGS);
	}

	if (!strcmp(argv[2], "-e")) {
		extract(argv[1], argv[3]);
		exit(0);
	}

	if (!strcmp(argv[2], "-p")) {
		goto pack;
		exit(0);
	}

	help();


pack:
	if (!argv[3]) {
		help();
		exit(ERROR_ARGS);
	}

	printf("Packing...\n");
	fo = fopen(argv[3], "r");
	if (!fo) {
		fprintf(stderr, "ERROR: File %s not found.\n", argv[3]);
		help();
		}
	rewind(fo);
	printf("Checking filelist...\n");
	while(!feof(fo)) {
		sdata[0]=0;
		fgets(sdata, LARGE_SPACE_SIZE, fo);
		k = strlen(sdata);
		if ((k>0) && ((sdata[k-1]==0x0A) || (sdata[k-1]==0x0D))) {
			sdata[k-1]=0;
		}
		if ((strcmp(sdata2[totalFiles], sdata)) && (k>1)) {
			strcpy(sdata2[totalFiles], sdata);
			fo3 = fopen(sdata2[totalFiles], "rb");
			if (!fo3) {
				fprintf(stderr, "ERROR: Could not find \"%s\"\n", sdata2[totalFiles]);
				totalErrors++;
			}
			else {
				rewind(fo3);
				fseek (fo3, 0, SEEK_END);
				sdata2s[totalFiles] = ftell(fo3);
				fclose(fo3);
			}
			totalFiles++;
			if (totalFiles > LARGE_SPACE_SIZE) break;
		}
	}
	fclose(fo);

	if (totalErrors > 0) {
		printf("Sorry, could not found %ld files, fix the problem before retrying.\n", totalErrors);
		exit(0);
	}
	else if (totalFiles == 0) {
		printf("Nothing to pack. Check your files!\n");
		exit(1);
	}
	else if (totalFiles > LARGE_SPACE_SIZE) {
		printf("Sorry, this crappy exe cannot pack more than %d files!\n", LARGE_SPACE_SIZE);
		exit(1);
	}

	fn = fopen(argv[1], "wb");
	if (!fn) {
		fprintf(stderr, "ERROR: Could not open file \"%s\" for writing\n", argv[1]);
		exit(1);
	}


	rewind(fn);
	//r�serv�
	putc(0xFF, fn);
	putc(0xFF, fn);

	c3 = totalFiles / BYTE_CAP;
	c4 = totalFiles % BYTE_CAP;
	putc(c3, fn);
	putc(c4, fn);


	for (i = 0; i < totalFiles; ++i) {
		strcpy(sdata, sdata2[i]);
		getFilename(sdata);
		k = strlen(sdata);
		c3 = k / 256;
		j = c3 * 256;
		k = k-j;
		c4 = k;

		putc(c3, fn);
		putc(c4, fn);

		fputs(sdata, fn);


		// position relative
		k = fileDataPos;
		c1 = k / 16777216;
		j = c1 * 16777216;

		k = k - j;
		c2 = k / 65536;
		j = c2 * 65536;

		k = k - j;
		c3 = k / 256;
		j = c3 * 256;

		k = k-j;
		c4 = k;
		putc(c1, fn);
		putc(c2, fn);
		putc(c3, fn);
		putc(c4, fn);


		// file size
		k = sdata2s[i];
		fileDataPos = (fileDataPos+k);

		c3 = (k/256);
		j = (c3*256);

		k = (k-j);
		c4 = k;

		putc(c3, fn);
		putc(c4, fn);

	}

	// Writing the header of the end position of the header
	fseek(fn, 0, SEEK_END);
	fileDataPos = ftell(fn);
	k = fileDataPos;

	c3 = (k/256);
	j = (c3*256);

	k = (k-j);
	c4 = k;

	rewind(fn);
	putc(c3, fn);
	putc(c4, fn);

	// pack files
	rewind(fn);
	fseek (fn, fileDataPos, 0);

	for (i=0;i<totalFiles;i++) {
		fo = fopen(sdata2[i], "rb");
		if (!fo) {
			fprintf(stderr, "ERROR: Could not open file \"%s\" for reading\n", sdata2[i]);
			fclose(fn);
			exit(1);
		}
		j=0;
		while(!feof(fo)) {
			c1=getc(fo);
			if (!feof(fo)) putc(c1, fn);
			j++;
		}
		if (j - 1 != sdata2s[i]) {
			fprintf(stderr, "ERROR: Could not match size of file! j = %ld\n", j);
		}
		fclose(fo);
	}
	fclose(fn);


	printf("\n Uh yeah, its done! %ld errors for %ld (announced)\n", totalErrors, totalFiles);
	exit(0);
}
