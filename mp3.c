#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include "mp3.h"        

// Convert 4-byte synchsafe size into normal integer
int read_size(unsigned char size_bytes[4])
{
    return (size_bytes[0] << 21) | (size_bytes[1] << 14) | (size_bytes[2] << 7) | size_bytes[3];
}

// Write size back into 4-byte synchsafe format
void write_size(FILE *fp, int size)
{
    unsigned char b[4];
    b[0] = (size >> 21) & 0x7F;    // Highest 7 bits
    b[1] = (size >> 14) & 0x7F;    // Next 7 bits
    b[2] = (size >> 7)  & 0x7F;    // Next 7 bits
    b[3] = size & 0x7F;           // Last 7 bits
    fwrite(b, 1, 4, fp);          // Write 4 bytes to file
}

// Validate MP3 file extension and check for ID3 header
int validate_mp3(const char *filename)
{
    const char *ext = strrchr(filename, '.');          // Find last dot in filename
    if(!ext || strcmp(ext, ".mp3") != 0) {             // Check extension
        printf("Error: %s is not an MP3 file!\n", filename);
        return 0;
    }

    FILE *fp = fopen(filename, "rb");                  // Open file in read-binary mode
    if(!fp) {
        printf("Error: Cannot open %s\n", filename);
        return 0;
    }

    unsigned char header[10];
    if(fread(header, 1, 10, fp) != 10) {               // Read ID3 header
        fclose(fp);
        return 0;
    }
    fclose(fp);

    if(strncmp((char*)header, "ID3", 3) != 0) {        // Verify ID3 signature
        printf("Error: No ID3v2 tag found!\n");
        return 0;
    }
    return 1;
}

// Display all ID3v2 tag values from MP3 file
void view_tags(const char *filename)
{
    FILE *fp = fopen(filename, "rb");                  // Open file
    if(!fp)
        return;

    TagInfo tag = {"","","","","",""};                 // Initialize tag struct with empty strings

    unsigned char header[10];
    fread(header, 1, 10, fp);                          // Read ID3 header
    int tag_size = read_size(header + 6);              // Extract tag size from header

    long start = ftell(fp);                            // Starting position of frames

    // Loop through all frames
    while(ftell(fp) - start < tag_size)
    {
        char frame[5] = {0};                           // Frame ID buffer
        unsigned char size_bytes[4];
        char flags[2];

        if(fread(frame, 1, 4, fp) != 4) break;         // Read frame ID
        if(frame[0] == 0) break;                       // End of frames
        if(fread(size_bytes, 1, 4, fp) != 4) break;    // Read frame size
        if(fread(flags, 1, 2, fp) != 2) break;         // Read frame flags

        int size = (size_bytes[0]<<24)|(size_bytes[1]<<16)|(size_bytes[2]<<8)|size_bytes[3];  // Convert size
        if(size <= 0) break;

        unsigned char *data = (unsigned char*)malloc(size);   // Allocate memory for frame data
        fread(data, 1, size, fp);                             // Read frame content

        int offset = 1;                                       // Skip text encoding byte

        // Match each tag and copy data
        if(strcmp(frame, "TIT2") == 0) strncpy(tag.title,   (char*)data + offset, sizeof(tag.title)-1);
        else if(strcmp(frame, "TPE1") == 0) strncpy(tag.artist, (char*)data + offset, sizeof(tag.artist)-1);
        else if(strcmp(frame, "TALB") == 0) strncpy(tag.album,  (char*)data + offset, sizeof(tag.album)-1);
        else if(strcmp(frame, "TYER") == 0 || strcmp(frame,"TDRC")==0)
            strncpy(tag.year, (char*)data + offset, sizeof(tag.year)-1);
        else if(strcmp(frame, "COMM") == 0) {
            int i = 4; while(i < size && data[i] != 0) i++;   // Skip language code & content descriptor
            if(i + 1 < size) strncpy(tag.comment, (char*)data + i + 1, sizeof(tag.comment)-1);
        }
        else if(strcmp(frame, "TCON") == 0) strncpy(tag.genre, (char*)data + offset, sizeof(tag.genre)-1);

        free(data);                                           // Free allocated memory
    }
    fclose(fp);

    // Print extracted tag information
    printf("------------------------------------------------------------\n");
    printf("           MP3 Tag Reader and Editor for ID3v2\n");
    printf("------------------------------------------------------------\n");
    printf("Title     : %s\n", tag.title);
    printf("Artist    : %s\n", tag.artist);
    printf("Album     : %s\n", tag.album);
    printf("Year      : %s\n", tag.year);
    printf("Music     : %s\n", tag.genre);
    printf("Comment   : %s\n", tag.comment);
    printf("------------------------------------------------------------\n");
}

// Edit or add a tag frame in the MP3 file
int edit_tags(const char *filename, const char *tag, const char *new_value)
{
    FILE *fp = fopen(filename, "r+b");        // Open file in read/update binary mode
    if(!fp)
        return 0;

    unsigned char header[10];
    fread(header, 1, 10, fp);                 // Read ID3 header
    int tag_size = read_size(header + 6);     // Get size of all frames

    long start = ftell(fp);
    int updated = 0;                           // Flag to track if tag is modified

    // Search through frames
    while(ftell(fp) - start < tag_size)
    {
        char frame[5] = {0};
        unsigned char size_bytes[4];
        char flags[2];

        long pos = ftell(fp);                  // Save starting position of frame

        if(fread(frame, 1, 4, fp) != 4) break; // Read frame ID
        if(frame[0] == 0) break;               // End of frames
        if(fread(size_bytes, 1, 4, fp) != 4) break;  // Read size
        if(fread(flags, 1, 2, fp) != 2) break;       // Read flags

        int size = (size_bytes[0]<<24)|(size_bytes[1]<<16)|(size_bytes[2]<<8)|size_bytes[3];

        if(size <= 0) break;

        // If frame matches requested tag
        if(strcmp(frame, tag) == 0)
        {
            if(strlen(new_value) + 1 > size) {        // Check if new value fits in existing frame
                printf("Error: New value too long for frame!\n");
                fclose(fp);
                return 0;
            }

            fseek(fp, pos + 10, SEEK_SET);            // Move to frame content
            fputc(0x00, fp);                          // Write encoding byte
            fwrite(new_value, 1, strlen(new_value), fp);  // Write new value

            for(int i = strlen(new_value)+1; i < size; i++) 
                fputc(0, fp);                         // Pad remaining bytes

            printf("%s updated successfully: %s\n", tag, new_value);
            updated = 1;
            break;
        }

        fseek(fp, size, SEEK_CUR);                    // Skip to next frame
    }

    // If tag does not exist, create a new frame
    if(!updated)
    {
        fseek(fp, start + tag_size, SEEK_SET);        // Go to end of existing frames

        char frame_id[4] = {0};
        strncpy(frame_id, tag, 4);                    // Copy tag name
        fwrite(frame_id, 1, 4, fp);                   // Write frame ID

        int size = strlen(new_value) + 1;             // Frame size
        unsigned char b[4];
        b[0] = (size >> 24) & 0xFF;
        b[1] = (size >> 16) & 0xFF;
        b[2] = (size >> 8) & 0xFF;
        b[3] = size & 0xFF;

        fwrite(b, 1, 4, fp);                           // Write size
        fwrite("\0\0", 1, 2, fp);                      // Write frame flags
        fputc(0x00, fp);                               // Encoding byte
        fwrite(new_value, 1, strlen(new_value), fp);   // Write new value

        tag_size += 10 + strlen(new_value) + 1;        // Update total tag size
        fseek(fp, 6, SEEK_SET);                        // Jump to tag size field in header
        write_size(fp, tag_size);                      // Rewrite updated tag size

        printf("%s added successfully: %s\n", tag, new_value);
    }

    fclose(fp);
    return 1;
}
