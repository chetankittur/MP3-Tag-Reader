#ifndef MP3_H
#define MP3_H

// Structure to store extracted MP3 tag information
typedef struct
{
    char title[100];      // Song title
    char artist[100];     // Artist name
    char album[100];      // Album name
    char year[10];        // Release year
    char comment[200];    // Comment or description
    char genre[50];       // Genre of the song
} TagInfo;

// Reads a 4-byte synchsafe integer and converts to normal size
int read_size(unsigned char size_bytes[4]);

// Writes an integer size into synchsafe 4-byte format
void write_size(FILE *fp, int size);

// Validates MP3 file and checks for ID3v2 header
int validate_mp3(const char *filename);

// Reads and prints all ID3v2 tags from an MP3 file
void view_tags(const char *filename);

// Edits an existing tag or adds a new tag to the MP3 file
int edit_tags(const char *filename, const char *tag, const char *new_value);

#endif
