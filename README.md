# Sensitive information text cleaner

Implement the following program in standard C++:

## file read

Read in the file input.txt, which is the text to be cleaned;

Read the file reject.txt, this file contains the sensitive words to be filtered, one sensitive word is stored in each line.

Read in the file ask.txt, this file output.txt eventually needs to append something to the file header besides the cleaned input.txt.

Read data from replace.txt. Each line has two words separated by a space. The word in front is the word to be replaced, and the word in the back is the word to be replaced. Use the word in the back to replace the word in front.

## Cleanup actions

If a line read into input.txt contains a sensitive word defined in reject.txt, the entire line is deleted.

If the line read in contains a mobile phone number in mainland China, that is, an 11-digit number starting with 1, delete the mobile phone number.

Read data from replace.txt to perform replacement operation.

All of the above processes the full text of input.txt.

## output result

Output the cleaned text to output.txt, leaving the original input.txt unchanged.

The header of output.txt inserts the content of ask.txt.

Save the output.txt file.
