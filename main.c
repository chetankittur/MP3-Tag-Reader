/*Name: Chetan Kittur
  Date: 21/11/2025
  Project name: MP3 Tag Reader
  Description:The MP3 Tag Reader and Editor is a C-based application designed to read, display, and modify metadata stored
              in ID3v2 tags of MP3 audio files. ID3 tags contain important information such as the title, artist, album, year,
              genre, and comments embedded within an MP3 file. This project demonstrates low-level file handling, binary data 
              parsing, and manipulation of structured metadata embedded inside MP3 files.
              The application first validates whether the selected file contains a proper ID3v2 header. It then extracts tag 
              frame information by reading raw bytes directly from the MP3 file and interpreting frame structures according to 
              the ID3v2 specification. Users can view all available tags or selectively edit tags such as title (TIT2), 
              artist (TPE1), album (TALB), year (TYER), comment (COMM), and genre (TCON). The editor updates existing frames or 
              creates new frames when required, ensuring tag sizes are correctly recalculated using synchsafe integers.
              This project emphasizes binary-level understanding of MP3 format, robust parsing logic, pointer handling,
              dynamic memory allocation, and safe string manipulation. It serves as a practical demonstration of how audio
              metadata is stored and how low-level manipulation enables custom tag editing without relying on external libraries.
 */
#include <stdio.h>      // Standard I/O functions
#include <stdlib.h>     // Memory and process control functions
#include <string.h>     // String handling functions
#include "mp3.h"        // Custom header for MP3 operations

int main(int argc,char *argv[])
{
    // Check if minimum arguments are provided
    if(argc<3)
    {
        printf("Usage:\n");
        printf("View : %s -v <file.mp3>\n",argv[0]);    // Show correct usage for viewing tags
        printf("Edit : %s -e -t|-a|-y|-l|-c|-g <new_value> <file.mp3>\n",argv[0]); // Usage for editing tags
        return 1;   // Exit due to insufficient arguments
    }

    // Check if user selected view mode
    if(strcasecmp(argv[1],"-v")==0)
    {
        if(!validate_mp3(argv[2]))     // Validate MP3 file before viewing
            return 1;

        view_tags(argv[2]);            // Display all MP3 tags
    }
    // Check if user selected edit mode
    else if(strcasecmp(argv[1],"-e")==0)
    {
        // Ensure user passed minimum required arguments for editing
        if(argc<5)
        {
           printf("Usage:\n");
        printf("View : %s -v <file.mp3>\n",argv[0]);    // Show correct usage for viewing tags
        printf("Edit : %s -e -t|-a|-y|-l|-c|-g <new_value> <file.mp3>\n",argv[0]); 
            return 1;
        }

        char *tag=NULL;   // Pointer to store selected ID3 tag name

        // Map user option to ID3v2 tag
        if(strcasecmp(argv[2],"-t")==0) 
            tag="TIT2";   // Title
        else if(strcasecmp(argv[2],"-a")==0)
            tag="TPE1";   // Artist
        else if(strcasecmp(argv[2],"-l")==0) 
            tag="TALB";   // Album
        else if(strcasecmp(argv[2],"-y")==0) 
            tag="TYER";   // Year
        else if(strcasecmp(argv[2],"-c")==0) 
            tag="COMM";   // Comment
        else if(strcasecmp(argv[2],"-g")==0)
            tag="TCON";   // Genre
        else {
            printf("Invalid tag option!\n");  // Unknown tag option
            return 1;
        }

        if(!validate_mp3(argv[4]))      // Validate file before editing
            return 1;

        // Attempt to edit the tag value
        if(edit_tags(argv[4],tag,argv[3]))
        {
            printf("\nUpdated Details:\n");
            view_tags(argv[4]);         // Display updated tags
        }
        else 
            printf("Failed to edit tag.\n");   // Editing failed
    }
    else 
        printf("Invalid option.\n");    // If user enters unknown command

    return 0;  
}
