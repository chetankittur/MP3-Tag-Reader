# MP3 Tag Reader & Editor (ID3v2) – C Project
A C-based command-line application that reads and edits ID3v2 metadata tags in MP3 files. The project parses the raw binary structure of MP3 files to extract information such as Title, Artist, Album, Year, Genre, and Comments. It allows users to view existing metadata or modify specific tags by directly updating or creating ID3v2 frames. This project demonstrates low-level file handling, bitwise operations, buffer management, and manual parsing of audio metadata without using any external libraries.

# Features
1.Reads ID3v2 tags from MP3 files
2.Edits specific tags (Title, Artist, Album, Year, Comment, Genre)
3.Adds frames if they do not exist
4.Validates MP3 files and checks ID3 header
5.Pure C implementation (no external dependencies)

# How It Works
The program reads the MP3 file’s binary structure, parses ID3v2 frames like TIT2, TPE1, TALB, TYER, COMM, and TCON, and displays or updates metadata. Frame sizes are handled using synchsafe integers as defined in the ID3v2 standard.
