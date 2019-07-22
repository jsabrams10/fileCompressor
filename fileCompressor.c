/*
 John Abrams - jsa109 | Chris Zachariah - cvz2
 4/3/2019
 CS 214 - Systems Programming
 Asst2 - fileCompressor
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

struct node {
    int type;              // will use 0 (check only frequency) and 1 (check only name) => once inside the Huffman tree Heap
    int frequency;         // number of times the word has appeared in the file
    char* name;            // word located in a file
    struct node* next;     // used only when nodes are in linked list
    struct node* left;     // children of the node
    struct node* right;
};

struct decompNode {
    char* bitString;                                    // copy of the bitStrig from the HuffmanCodebook
    char* tokenName;                                    // copy of the token from the HuffmanCodebook
    struct decompNode* next;                            // next node
};

void runBFlag(char* fileName);                                                          // makes code look clean by having this function run the bFlag()
struct node* bFlag(char* fileName);                                                     // function for the -b call
struct node* updateRoot(char* word , struct node* root);                                // will update the Linked List of nodes
struct node* orderNodes(struct node* root);                                             // will order the nodes in the Linked List based on frequency
struct node* makeTree(struct node* root);                                               // will make a tree given a linked list
void printTree(struct node* treePtr, char* arr , int cur, int beginning);               // print out the tree in-Order traversal into HuffmanCodebook
void deleteTree(struct node* treePtr);                                                  // delete all the nodes of the tree
void cFlag(char* fileName);                                                             // funtion for the -c call
char* addExt(char* a, char* b);                                                         // compbine a and b and get ab ( example test , .hcz => test.hcz)
void compressFile(char* fileName,char* compare);                                        // compresses given file name
void dFlag(char* compressedFileName);                                                   // decompress a given compressed file
void decompressFile(char* newFile , char* allBits , int curr , int starting);           // main function that decompresses given compressed file
struct decompNode* makeDecompNodes(char* codeBookName);                                 // comb through the HuffmanCodebook and make a Linked List of tokens and bitStrings
void recurFlag(char* dirPath, int fdOut , int mode);                                    // recursively run through a specific process given a certain mode

int main (int argc , char** argv) {
struct node* root = NULL;
  char mode = '\0';                                                            // mode scenario
	int recFlag = 0;                                                           // check for the -R flag
	int numFlags = 0;                                                          // number of flags found in the command line input

	if (argc < 2 || argc > 5) {
		printf("ERROR: TOO FEW/MANY FLAGS/ARGUMENTS WERE PROVIDED\n");
		return 0;
	}
	for(; numFlags < 2; numFlags++) {                                           // figure out the flag situation and check if the recursive flaf was thrown
		if(strstr(argv[1 + numFlags], "-") != NULL) {
			if(strcmp("-b", argv[1 + numFlags]) == 0) {
                mode = 'b';
            } else if (strcmp("-c", argv[1 + numFlags]) == 0) {
                mode = 'c';
            } else if (strcmp("-d", argv[1 + numFlags]) == 0) {
                mode = 'd';
            } else if (strcmp("-R", argv[1 + numFlags]) == 0) {
                recFlag = 1;
            } else {
                break;
            }
		} else if (numFlags == 0) {
			printf("ERROR: MUST PROVIDE AT LEAST 1 FLAG\n");
			return 0;
		} else {
            ;
        }
	} // ends the for loop

	if(recFlag == 0) {                                              // -R flag was found in the input
		int argFD = -1;
		argFD = open(argv[numFlags], O_RDWR, S_IRWXU);

		if(argFD < 0){
			printf("ERROR: open() FAILED.\n");
			return 0;
		}
		close(argFD);
	} else {	                                                    // -R flag was found in the input
		DIR* argDirPtr = NULL;
		argDirPtr = opendir(argv[numFlags + 1]);
		if(argDirPtr == NULL){
			printf("ERROR: opendir() FAILED.\n");
			return 0;
		}
		closedir(argDirPtr);
	}
	switch(mode) {
		case 'b':
			if ((recFlag == 0 && argc != 3) || (recFlag == 1 && argc != 4)) {
				printf("ERROR: TOO FEW/MANY FLAGS/ARGUMENTS WERE PROVIDED\n");
				return 0;
			}
            if (recFlag == 0) {
                int newCodeBook = open("HuffmanCodebook",O_RDWR | O_CREAT, 0644);         // this will be the 'HuffmanCodebook' file in the same directory
                if (newCodeBook >=  0) { remove("HuffmanCodebook"); }                     // get rid of the old HuffmanCodebook so that a new one can be made
                runBFlag(argv[numFlags]);
      
            } else {
                int newCodeBook = open("HuffmanCodebook",O_RDWR | O_CREAT | O_APPEND, 0644);
                if (newCodeBook >=  0) { remove("HuffmanCodebook"); }                     // get rid of the old HuffmanCodebook so that a new one can be made
          
                newCodeBook = open("HuffmanCodebook",O_RDWR | O_CREAT | O_APPEND, 0644);
          
                int newTempFile = open("tempfile.txt",O_RDWR | O_CREAT | O_APPEND, 0644); // look for a temp file within the directory
                if (newTempFile >=  0) { remove("tempfile.txt"); }                        // remove old temp file
                newTempFile = open("tempfile.txt",O_RDWR | O_CREAT | O_APPEND, 0644);     // create new temp file so that a HuffmanCodebook can be created later
                
                recurFlag(argv[numFlags+1], newTempFile , 1);                             // recursively go through the directories and sub-directories and write up file contents
                runBFlag("tempfile.txt");                                                 // will make a HuffmanCodebook within the same directory
                
                if (newTempFile >=  0) { remove("tempfile.txt"); }                        // delete the temp file
            }
            break;

		case 'c':
			if((recFlag == 0 && argc != 4) || (recFlag == 1 && argc != 5)){
				printf("ERROR: TOO FEW/MANY FLAGS/ARGUMENTS WERE PROVIDED\n");
				return 0;
			}
            if (recFlag == 0) {
                int newCodeBook = open("HuffmanCodebook",O_RDWR | O_CREAT, 0644); // this will be the 'HuffmanCodebook' file in the same directory
                if (newCodeBook < 0) {
                    printf("ERROR: THERE NEEDS TO BE A CODEBOOK FOR THE FILE IN THE DIRECTORY FOR THE COMPRESSION TO WORK");
                    return 0;
                }
                cFlag(argv[numFlags]);
            } else {
                int newCodeBook = open("HuffmanCodebook",O_RDWR | O_CREAT, 0644); // this will be the 'HuffmanCodebook' file in the same directory
                if (newCodeBook < 0) {
                    printf("ERROR: THERE NEEDS TO BE A CODEBOOK FOR THE FILES IN THE DIRECTORY AND SUBDIRECTORIES IN ORDER FOR THE RECURSIVE COMPRESSION TO WORK");
                    return 0;
                }
                recurFlag(argv[numFlags+1], newCodeBook , 2);                     // recursively compress files found in directory and sub-directories
            }
			break;

		case 'd':
            if((recFlag == 0 && argc != 4) || (recFlag == 1 && argc != 5)){
                printf("ERROR: too few/many flags/arguments ");
                printf("provided.\n");
                return 0;
            }
            if (recFlag == 0) {
                int newCodeBook = open("HuffmanCodebook",O_RDWR | O_CREAT, 0644); // this will be the 'HuffmanCodebook' file in the same directory
                if (newCodeBook < 0) {
                    printf("ERROR: THERE NEEDS TO BE A CODEBOOK FOR THE FILE IN THE DIRECTORY FOR THE DECOMPRESSION TO WORK");
                    return 0;
                }
                dFlag(argv[numFlags]);
        
            } else {
                int newCodeBook = open("HuffmanCodebook",O_RDWR | O_CREAT, 0644); // this will be the 'HuffmanCodebook' file in the same directory
                if (newCodeBook < 0) {
                    printf("ERROR: THERE NEEDS TO BE A CODEBOOK FOR THE FILES IN THE DIRECTORY AND SUBDIRECTORIES IN ORDER FOR THE RECURSIVE COMPRESSION TO WORK");
                    return 0;
                }
                recurFlag(argv[numFlags+1], newCodeBook , 3);
            }
			break;
    } // ends the switch statements
	return 0;
} // end of the main function

/*
  Params:
  - Char array with the file name
  Returns:
  - HuffmanCodebook file in the current directory
*/
void runBFlag(char* fileName) {
  struct node* root = NULL;
  root = bFlag(fileName);                                          // get LL of words in the correct order
  struct node* treeNode = NULL;
  treeNode = makeTree(root);                                       // make the Huffman Tree
  char* bits = (char*)malloc(100*sizeof(char));
  char* q = bits; int i = 0; while(i < 100){q[i] = 'a'; i++;}
  int cur = 0;
  int beginning = 0;
  printTree(treeNode,bits,cur,beginning);                           // make the HuffmanCodebook in the same directory
  free(bits);
  deleteTree(treeNode);
  treeNode = NULL;
} // ends the runBFlag() function

/*
  Params:
  - Char array with the file name
  Returns:
  - Node pointer to an updated Linked List in order from lowest to highest frequency
*/
struct node* bFlag(char* fileName) {
  int fd = open(fileName ,O_RDONLY);                             // opens the file selected
  if (fd < 0) { printf("ERROR: CANNOT OPEN FILE\n");}            // print in-case of error with the file

  char* wordFound = (char*)malloc(100*sizeof(char));             // once a word is found, the chars are inserted into this char array
  int i = 0;
  for (i = 0 ; i < 100 ; i++){wordFound[i] = '$';}

  struct node* root = NULL;
  char c;
  int reading = read(fd,&c,1);
  int currPlace = 0;

  while(reading > 0) {                                            // go through all the letters and put the words into an ordered LL
    if (c == ' ') {
      wordFound[currPlace] = '\0';
      root = updateRoot(wordFound,root);
      root = updateRoot(" ",root);
      currPlace = 0;
    } else if (c == '\t') {
      wordFound[currPlace] = '\0';
      root = updateRoot(wordFound,root);
      root = updateRoot("\t",root);
      currPlace = 0;
    } else if (c == '\n') {
      wordFound[currPlace] = '\0';
      root = updateRoot(wordFound,root);
      root = updateRoot("\n",root);
      currPlace = 0;
    } else if (c == '\0') {
      wordFound[currPlace] = '\0';
      root = updateRoot(wordFound,root);
    } else {
      wordFound[currPlace] = c;
      currPlace++;
    }
    reading = read(fd,&c,1);
  } // ends the while loop
  if (currPlace != 0) {                                         // just in case the last word did not get pushed into the LL
    wordFound[currPlace] = '\0';
    root = updateRoot(wordFound,root);
  }
  free(wordFound);
  close(fd);
  return root;
} // ends the bFlag() function

/*
  Params:
  - Char array with a single word
  - Node pointer to the root of the Linked List
  Returns:
  - Node pointer to an updated Linked List
*/
struct node* updateRoot(char* word , struct node* root) {
  char* ptr = word;                                           // pointer to the word
  struct node* nodePtr = NULL;

  if (root != NULL) {                                         // add this word to the LL if it is the first appearance or increase the frequency
    if (word[0] == ' ') {                                     // word is a space
      nodePtr = root;
      while (nodePtr != NULL) {                               // see if this word already exists in the LL
        if (strcmp(nodePtr->name,"$Space$") == 0) {
          nodePtr->frequency =  nodePtr->frequency + 1;       // word exists in LL, so just increase the frequency and then return the root
          root = orderNodes(root);
          return root;
        }
        nodePtr = nodePtr->next;
      } // ends the while loop
      if (nodePtr == NULL) {                                  // word does not exist in the LL and a new node needs to be added to the LL
        struct node* newNode = (struct node*)malloc(sizeof(struct node));
        nodePtr = newNode;
        nodePtr->type = 1;
        nodePtr->frequency = 1;
        nodePtr->name = (char*)malloc(8*sizeof(char));
        nodePtr->name[0] = '$';
        nodePtr->name[1] = 'S';
        nodePtr->name[2] = 'p';
        nodePtr->name[3] = 'a';
        nodePtr->name[4] = 'c';
        nodePtr->name[5] = 'e';
        nodePtr->name[6] = '$';
        nodePtr->name[7] = '\0';
        nodePtr->next = root;
        nodePtr->left = NULL;
        nodePtr->right = NULL;
        root = newNode;
        root = orderNodes(root);
        return root;
      } else {
        return root;
      }
    } else if (word[0] == '\t') {                             // word is a tab
      nodePtr = root;
      while (nodePtr != NULL) {                               // see if this word already exists in the LL
        if (strcmp(nodePtr->name,"$Tab$") == 0) {
          nodePtr->frequency =  nodePtr->frequency + 1;       // word exists in LL, so just increase the frequency and then return the root
          root = orderNodes(root);
          return root;
        }
        nodePtr = nodePtr->next;
      } // ends the while loop
      if (nodePtr == NULL) {                                  // word does not exist in the LL and a new node needs to be added to the LL
        struct node* newNode = (struct node*)malloc(sizeof(struct node));
        nodePtr = newNode;
        nodePtr->type = 1;
        nodePtr->frequency = 1;
        nodePtr->name = (char*)malloc(6*sizeof(char));
        nodePtr->name[0] = '$';
        nodePtr->name[1] = 'T';
        nodePtr->name[2] = 'a';
        nodePtr->name[3] = 'b';
        nodePtr->name[4] = '$';
        nodePtr->name[5] = '\0';
        nodePtr->next = root;
        nodePtr->left = NULL;
        nodePtr->right = NULL;
        root = newNode;
        root = orderNodes(root);
        return root;
      } else {
        return root;
      }
    } else if (word[0] == '\n') {                             // word is a new line
      nodePtr = root;
      while (nodePtr != NULL) {                               // see if this word already exists in the LL
        if (strcmp(nodePtr->name,"$NewLine$") == 0) {
          nodePtr->frequency =  nodePtr->frequency + 1;       // word exists in LL, so just increase the frequency and then return the root
          root = orderNodes(root);
          return root;
        }
        nodePtr = nodePtr->next;
      } // ends the while loops
      if (nodePtr == NULL) {                                  // word does not exist in the LL and a new node needs to be added to the LL
        struct node* newNode = (struct node*)malloc(sizeof(struct node));
        nodePtr = newNode;
        nodePtr->type = 1;
        nodePtr->frequency = 1;
        nodePtr->name = (char*)malloc(10*sizeof(char));
        nodePtr->name[0] = '$';
        nodePtr->name[1] = 'N';
        nodePtr->name[2] = 'e';
        nodePtr->name[3] = 'w';
        nodePtr->name[4] = 'L';
        nodePtr->name[5] = 'i';
        nodePtr->name[6] = 'n';
        nodePtr->name[7] = 'e';
        nodePtr->name[8] = '$';
        nodePtr->name[9] = '\0';
        nodePtr->next = root;
        nodePtr->left = NULL;
        nodePtr->right = NULL;
        root = newNode;
        root = orderNodes(root);
        return root;
      } else {
        return root;
      }
    } else if (word[0] == '\0') {                             // case where there is a null terminator placed in the front of word
      return root;
    } else {
      nodePtr = root;
      while (nodePtr != NULL) {                               // see if this word already exists in the LL
        if (strcmp(nodePtr->name,word) == 0) {
          nodePtr->frequency =  nodePtr->frequency + 1;       // word exists in LL, so just increase the frequency and then return the root
          root = orderNodes(root);
          return root;
        }
        nodePtr = nodePtr->next;
      } // ends the while loops
      if (nodePtr == NULL) {
        struct node* newNode = (struct node*)malloc(sizeof(struct node));
        nodePtr = newNode;
        nodePtr->type = 1;
        nodePtr->frequency = 1;
        nodePtr->name = (char*)malloc((strlen(word)+1)*sizeof(char));
        strcpy(nodePtr->name,word);
        nodePtr->name[strlen(word)] = '\0';
        nodePtr->next = root;
        nodePtr->left = NULL;
        nodePtr->right = NULL;
        root = newNode;
        root = orderNodes(root);
        return root;
      } else {
        return root;
      }
    }
  } else {                                                    // this is the first word to be added to the LL
    if (word[0] == ' ') {                                     // word is a space
      struct node* newNode = (struct node*)malloc(sizeof(struct node));
      nodePtr = newNode;
      nodePtr->type = 1;
      nodePtr->frequency = 1;
      nodePtr->name = (char*)malloc(8*sizeof(char));
      nodePtr->name[0] = '$';
      nodePtr->name[1] = 'S';
      nodePtr->name[2] = 'p';
      nodePtr->name[3] = 'a';
      nodePtr->name[4] = 'c';
      nodePtr->name[5] = 'e';
      nodePtr->name[6] = '$';
      nodePtr->name[7] = '\0';
      nodePtr->next = NULL;
      nodePtr->left = NULL;
      nodePtr->right = NULL;
      root = newNode;
      return root;
    } else if (word[0] == '\t') {                             // word is a tab
      struct node* newNode = (struct node*)malloc(sizeof(struct node));
      nodePtr = newNode;
      nodePtr->type = 1;
      nodePtr->frequency = 1;
      nodePtr->name = (char*)malloc(6*sizeof(char));
      nodePtr->name[0] = '$';
      nodePtr->name[1] = 'T';
      nodePtr->name[2] = 'a';
      nodePtr->name[3] = 'b';
      nodePtr->name[4] = '$';
      nodePtr->name[5] = '\0';
      nodePtr->next = NULL;
      nodePtr->left = NULL;
      nodePtr->right = NULL;
      root = newNode;
      return root;
    } else if (word[0] == '\n') {                             // word is a new line
      struct node* newNode = (struct node*)malloc(sizeof(struct node));
      nodePtr = newNode;
      nodePtr->type = 1;
      nodePtr->frequency = 1;
      nodePtr->name = (char*)malloc(10*sizeof(char));
      nodePtr->name[0] = '$';
      nodePtr->name[1] = 'N';
      nodePtr->name[2] = 'e';
      nodePtr->name[3] = 'w';
      nodePtr->name[4] = 'L';
      nodePtr->name[5] = 'i';
      nodePtr->name[6] = 'n';
      nodePtr->name[7] = 'e';
      nodePtr->name[8] = '$';
      nodePtr->name[9] = '\0';
      nodePtr->next = NULL;
      nodePtr->left = NULL;
      nodePtr->right = NULL;
      root = newNode;
      return root;
    } else if (word[0] == '\0') {                            // the case where the first char in a file is a space/tab/newline
      return root;
    } else {
      struct node* newNode = (struct node*)malloc(sizeof(struct node));
      nodePtr = newNode;
      nodePtr->type = 1;
      nodePtr->frequency = 1;
      nodePtr->name = (char*)malloc((strlen(word)+1)*sizeof(char));
      strcpy(nodePtr->name,word);
      nodePtr->name[strlen(word)] = '\0';
      nodePtr->next = NULL;
      nodePtr->left = NULL;
      nodePtr->right = NULL;
      root = newNode;
      return root;
    } // ends all the if-else cases
  } // ends the main if/else of the function (if root is NULL or not)
} // ends the updateRoot() function

/*
 Params:
 - Node pointer to the root of the Linked List
 Returns:
 - Node pointer to an ordered Linked List from lowest to highest frequency
 */
struct node* orderNodes(struct node* root) {
 struct node* rootPtr = root;                                // pointer to the root
 struct node* newRoot = NULL;                                // pointer to the new LL that will be returned
 struct node* next = NULL;                                   // pointer to the next node in the original root so that the LL won't be lost
 struct node* compare = NULL;                                // pointer used to compare frequencies
 struct node* prev = NULL;                                   // used for the comparison process and helps insert new nodes inside new LL

 while(rootPtr != NULL) {
   next = rootPtr->next;
   rootPtr->next = NULL;
   if (newRoot == NULL) {                                    // since the newRoot is empty, make the current node the first one
     newRoot = rootPtr;
     rootPtr->next = NULL;
   } else {
     compare = newRoot;                                      // iterate until we find a place to insert or pointer reaches end
     while( (compare != NULL) && ((rootPtr->frequency) > (compare->frequency)) ) {
       prev = compare;
       compare = compare->next;
     } // ends inner while loop
     if (compare != NULL) {                                  // put the new node inside the LL
       if (prev == NULL) {                                   // add the new node to the front
         rootPtr->next = newRoot;
         newRoot = rootPtr;
       } else {                                              // add the new node inside the LL
         rootPtr->next = prev->next;
         prev->next = rootPtr;
       }
     } else {                                                // put the new node at the end of the LL
       prev->next = rootPtr;
       rootPtr->next = NULL;
     }
   }
   rootPtr = next;
 } // ends the while loop
 return newRoot;
} // ends the orderNodes() function

/*
  Params:
  - pointer to ordered LL
  Returns:
  - pointer to a tree structure
*/
struct node* makeTree(struct node* root) {
  struct node* firstNode = NULL;
  struct node* secondNode = NULL;
  struct node* generalPtr = NULL;
  int keepGoing = 0;

  while(keepGoing == 0) {
    if (root == NULL) {                                           // root is NULL, do nothing
      keepGoing = 1;
    } else if ((root != NULL) && (root->next == NULL)) {          // there is only one node left , either its the tree itself
      firstNode = root;                                           // or there is only one token to start with
      if (firstNode->type == 0) {
        keepGoing = 1;
      } else {
        root->next = NULL;
        root->left = NULL;
        root->right = NULL;
        struct node* newNode = (struct node*)malloc(sizeof(struct node));
        generalPtr = newNode;
        generalPtr->type = 0;
        generalPtr->frequency = root->frequency;
        generalPtr->name = NULL;
        generalPtr->next = NULL;
        generalPtr->left = root;
        generalPtr->right = NULL;
        root = generalPtr;
        keepGoing = 1;
      }
    } else {
      firstNode = root;                                           // take first two nodes and combine to make subtree
      secondNode = root->next;
      root = secondNode->next;

      if (firstNode->type == 0) {
        firstNode->next = NULL;
      } else {
        firstNode->next = NULL;
        firstNode->left = NULL;
        firstNode->right= NULL;
      }
      if (secondNode->type == 0) {
        secondNode->next = NULL;
      } else {
        secondNode->next = NULL;
        secondNode->left = NULL;
        secondNode->right= NULL;
      }

      struct node* newNode = (struct node*)malloc(sizeof(struct node));
      generalPtr = newNode;
      generalPtr->type = 0;
      generalPtr->frequency = (firstNode->frequency) + (secondNode->frequency);
      generalPtr->name = NULL;
      generalPtr->next = root;
      generalPtr->left = firstNode;
      generalPtr->right = secondNode;
      root = generalPtr;
        root = orderNodes(root);
      keepGoing = 0;
    } // ends the main if-else if ...
  } // end of the while loop
  return root;
} // ends the makeTree() function

/*
  Params:
  - pointer to tree structure
  - char array to store binary bits
  - int to keep counter for going through array
  - int boolean to know when to create the HuffmanCodebook
  Returns:
  - makes a "HuffmanCodebook" file with the binary bits and token names
*/
void printTree(struct node* treePtr, char* arr , int cur , int beginning) {
  if (beginning == 0) {                                                          // first time the function is called, make the HuffmanCodebook
    int newCodeBook = open("HuffmanCodebook",O_RDWR | O_CREAT , 0644);           // this will be the 'HuffmanCodebook' file in the same directory
    if (newCodeBook < 0) { printf("ERROR: CANNOT OPEN FILE\n"); return;}         // print in-case of error with the file
    char c = '$';                                                                // escape character that goes on the first line of the codeBook
    int writing = write(newCodeBook,&c,1);
    c = '\n';                                                                    // get to the new line so the next function can write up the rest
    writing = write(newCodeBook,&c,1);
    int closefd = close(newCodeBook);
    if (closefd < 0) { printf("ERROR IN CLOSING THE HUFFMANCODEBOOK FILE\n"); return;}
    printTree(treePtr,arr,0,1);                                                  // let the program go fill out the rest of the codebook
    return;
  }
    if (treePtr == NULL) {
        return;
    }
  if (treePtr->left) {
    arr[cur] = '0';
    printTree(treePtr->left,arr,cur+1,1);
  }
  if (treePtr->right) {
    arr[cur] = '1';
    printTree(treePtr->right,arr,cur+1,1);
  }
  if (treePtr->type == 1) {                                                     // finaly got to a leaf, now put into the HuffmanCodebook
    arr[cur] = '\0';

    int addToCodeBook = open("HuffmanCodebook", O_WRONLY | O_APPEND);// append the rest to the codebook
    if (addToCodeBook < 0) { printf("ERROR: CANNOT OPEN FILE\n"); return; }

    char* ptr = arr;
    int a = 0;
    int appending;
    while(ptr[a] != '\0') {
      appending = write(addToCodeBook,&ptr[a],1);
      a++;
    }
    char tab = '\t';
    appending = write(addToCodeBook,&tab,1);
    a = 0;
    ptr = treePtr->name;
    while(ptr[a] != '\0') {
      appending = write(addToCodeBook,&ptr[a],1);
      a++;
    }
    char newLine = '\n';
    appending = write(addToCodeBook,&newLine,1);
  }
} // ends the printTree() function

/*
 Params:
 - pointer to tree structrue
 Returns:
 - deletes all the nodes of the tree in post order
 */
void deleteTree(struct node* treePtr) {
    if (treePtr == NULL) {
        return;
    }
    deleteTree(treePtr->left);
    deleteTree(treePtr->right);
    
    if (treePtr->type == 1) {
        free(treePtr->name);
    }
    free(treePtr);
} // ends the deleteTree() functionF

/*
  Params:
  - char array with the file name
  Returns:
  - makes the .hcz file with the compressed with binary bitString compression of tokens
*/
void cFlag(char* fileName){
  int fd = open(fileName ,O_RDONLY);                             // opens the file selected
  if (fd < 0) { printf("ERROR: CANNOT OPEN FILE\n");}            // print in-case of error with the file

  char* wordFound = (char*)malloc(100*sizeof(char));             // once a word is found, the chars are inserted into this char array
  int i = 0;
  for (i = 0 ; i < 100 ; i++){wordFound[i] = '$';}

  char c;
  int reading = read(fd,&c,1);
  int currPlace = 0;

  while(reading > 0) {                                            // go through all the letters and put it into the new compressed file
    if (c == ' ') {
      if (currPlace != 0) {
        wordFound[currPlace] = '\0';
        compressFile(fileName,wordFound);
      }
      wordFound[0] = '$';
      wordFound[1] = 'S';
      wordFound[2] = 'p';
      wordFound[3] = 'a';
      wordFound[4] = 'c';
      wordFound[5] = 'e';
      wordFound[6] = '$';
      wordFound[7] = '\0';
      compressFile(fileName,wordFound);
      currPlace = 0;
    } else if (c == '\t') {
      if (currPlace != 0) {
        wordFound[currPlace] = '\0';
        compressFile(fileName,wordFound);
      }
      wordFound[0] = '$';
      wordFound[1] = 'T';
      wordFound[2] = 'a';
      wordFound[3] = 'b';
      wordFound[4] = '$';
      wordFound[5] = '\0';
      compressFile(fileName,wordFound);
      currPlace = 0;
    } else if (c == '\n') {
      if (currPlace != 0) {
        wordFound[currPlace] = '\0';
        compressFile(fileName,wordFound);
      }
      wordFound[0] = '$';
      wordFound[1] = 'N';
      wordFound[2] = 'e';
      wordFound[3] = 'w';
      wordFound[4] = 'L';
      wordFound[5] = 'i';
      wordFound[6] = 'n';
      wordFound[7] = 'e';
      wordFound[8] = '$';
      wordFound[9] = '\0';
      compressFile(fileName,wordFound);
      currPlace = 0;
    } else if (c == '\0') {
      ;
    } else {
      wordFound[currPlace] = c;
      currPlace++;
    }
    reading = read(fd,&c,1);
  } // ends the while loop
  if (currPlace != 0) {                                         // just in case the last word did not get pushed into the LL
    wordFound[currPlace] = '\0';
    compressFile(fileName,wordFound);
  }
  close(fd);
  free(wordFound);
} // ends the cFlag() function

/*
  Param:
  - takes file name
  - takes the extension to add at the end
  Returns
  - char array with the new filename + extension
*/
char* addExt(char* a, char* b) {
  int count1 = 0;
  char* c = a;
  int i = 0;
  while(c[i] != '\0') {                                         //find the size of the filename
    count1++;
    i++;
  }
    
  int count2 = 0;
  i = 0;
  c = b;
  while(c[i] != '\0') {
    count2++;
    i++;
  }
                                                                // put the names togehter
  char* returnChar = (char*)malloc((count1+count2+1)*sizeof(char));
  i = 0;
  int curr = 0;
  c = a;

  while(i < count1) {
    returnChar[curr] = a[i];
    curr++;
    i++;
  }

  i = 0;
  while(i < count2) {
    returnChar[curr] = b[i];
    curr++;
    i++;
  }

  returnChar[count1+count2] = '\0';                             // put a null terminator at the end
  return returnChar;
} // ends the addExt() function

/*
  Param:
  - takes file name
  - take char array to compare against
  Returns
  - char array with the new filename + extension
*/
void compressFile(char* fileName,char* compare) {
  char p1 , p2;
  int p1Read , p2Read;

  char* compressedFileName = addExt(fileName,".hcz");                             // get the new file name
  int compressFile = open(compressedFileName,O_RDWR | O_CREAT | O_APPEND, 0644);  // this will be the <filename>.hcz within the same directory
  if (compressFile < 0) { printf("ERROR: CANNOT OPEN FILE\n"); return;}

  int pointerOneCB = open("HuffmanCodebook",O_RDWR,0644);                         // first pointer to the code codeBook
  if (pointerOneCB < 0) {printf("ERROR: CANNOT OPEN THE COOKBOOK\n"); return;}
  int pointerTwoCB = open("HuffmanCodebook",O_RDWR,0644);                         // second pointer to the code codeBook
  if (pointerTwoCB < 0) {printf("ERROR: CANNOT OPEN THE COOKBOOK\n"); return;}

  //both will read the first word ($)
  p1Read = read(pointerOneCB,&p1,1);
  p2Read = read(pointerTwoCB,&p2,1);

  // read the first \n
  p1Read = read(pointerOneCB,&p1,1);
  p2Read = read(pointerTwoCB,&p2,1);

  // get to the first bitSting
  p1Read = read(pointerOneCB,&p1,1);
  p2Read = read(pointerTwoCB,&p2,1);

  int target = 0;
  while (target == 0) {
    while( p1 != '\t') {
      read(pointerOneCB,&p1,1);
    }
    read(pointerOneCB,&p1,1);                      // now the first pointer is at the token side of the line
    int l = 0;
    if (p1 == compare[l]) {
      while((p1 == compare[l]) && (p1 != '\n') && (compare[l] != '\0')) {
        read(pointerOneCB,&p1,1);
        l++;
      }
      if (p1 == '\n' && (compare[l] == '\0')) {
        target = 1;                                 // we found the right word/token
      } else {
        target = 0;
        while(p1 != '\n') {
          read(pointerOneCB,&p1,1);
        }
        read(pointerOneCB,&p1,1);                    // first pointer is one the next line

        while(p2 != '\t') {
          read(pointerTwoCB,&p2,1);
        }
        read(pointerTwoCB,&p2,1);                   // second pointer at beginning of the word before

        while(p2 != '\n') {
          read(pointerTwoCB,&p2,1);
        }
        read(pointerTwoCB,&p2,1);                    // second pointer is one the next line
      }
    } else {
      target = 0;
      while(p1 != '\n') {
        read(pointerOneCB,&p1,1);
      }
      read(pointerOneCB,&p1,1);                    // first pointer is one the next line

      while(p2 != '\t') {
        read(pointerTwoCB,&p2,1);
      }
      read(pointerTwoCB,&p2,1);                   // second pointer at beginning of the word before

      while(p2 != '\n') {
        read(pointerTwoCB,&p2,1);
      }
      read(pointerTwoCB,&p2,1);                    // second pointer is one the next line
    }
  } // end of the main while loop

  // here the program has found the target, so now just print the bitString onto the new .hcz file using the second ptr
  while(p2 != '\t') {
      write(compressFile,&p2,1);
      read(pointerTwoCB,&p2,1);
    }
    close(compressFile);
    close(pointerOneCB);
    close(pointerTwoCB);
} // ends the compressFile() function

/*
  Param:
  - takes compressed file name (ends in .hcz)
  Returns
  - decompressed file file (without the .hcz) in the directory
*/
void dFlag(char* compressedFileName) {
  int totalLengthOfCFName = (int)strlen(compressedFileName);                    // length of the compressed file name
  int a = totalLengthOfCFName;
  char* newFile = (char*)malloc((a - 4)*sizeof(char));
  int count = 0;
  while(count < (a-4)) {newFile[count] = compressedFileName[count]; count++;}
  newFile[count] = '\0';
  if ((compressedFileName[a-4] == '.') && (compressedFileName[a-3] == 'h') && (compressedFileName[a-2] == 'c') && (compressedFileName[a-1] == 'z')) {
    int fd = open(compressedFileName ,O_RDONLY);                                // opens the file selected
    if (fd < 0) { printf("ERROR: CANNOT OPEN FILE\n");}                         // print in-case of error with the file
    char c;
    int reading = read(fd,&c,1);
    count = 1;
    while(reading > 0) {
      count++;
      reading = read(fd,&c,1);
    }
    close(fd);
    char* allBinaryBits= (char*)malloc((count+1)*sizeof(char));           // insert all chars in compressed file in this array
    int i = 0;
    for (i = 0 ; i <= count ; i++){allBinaryBits[i] = '0';}

    int fd2 = open(compressedFileName ,O_RDONLY);                                // opens the file selected
    if (fd2 < 0) { printf("ERROR: CANNOT OPEN FILE\n");}                         // print in-case of error with the file
    count = 0;
    reading = read(fd2,&c,1);
    while(reading > 0) {
      allBinaryBits[count] = c;
      count++;
      reading = read(fd,&c,1);
    }
    allBinaryBits[count] = '\0';
    int current = 0;
    int starting = 0;
    decompressFile(newFile , allBinaryBits , current , starting);
    close(fd2);
    free(allBinaryBits);
  } else {
    printf("ERROR: THIS IS NOT A COMPRESSED FILE. COMPRESSED FILES NEED THE EXTENSION \".hcz\"\n");
    return;
  }
} // ends the dFlag() function

/*
  Param:
  - takes char array of all the bits found in the compressed file
  - an int that tells the program where in the array it needs to start looking next
  Returns
  - decompressed file (without the .hcz) in the directory
*/
void decompressFile(char* newFile , char* allBits , int curr , int starting) {
  if (starting == 0) {
    int newDecompFile = open(newFile,O_RDWR | O_CREAT, 0644); // make the new decompressed file
    if (newDecompFile < 0) { printf("ERROR: CANNOT OPEN FILE\n"); return;}   // print in-case of error with the file
    curr = 0;
    starting = 1;
  }
  if (allBits[curr] == '\0') {
    return;
  }
  struct decompNode* root = NULL;
  root = makeDecompNodes("HuffmanCodebook");

  struct decompNode* ptr = root;

  int newDecompFile = open(newFile,O_RDWR | O_CREAT, 0644); // make the new decompressed file
  if (newDecompFile < 0) { printf("ERROR: CANNOT OPEN FILE\n"); return;}   // print in-case of error with the file

  char b;
  int temp;
  while (curr < strlen(allBits)) {
    ptr = root;
    while(ptr != NULL) {
      if (ptr->bitString[0] == allBits[curr]) {
        temp = curr;
        int i = 0;
        while(ptr->bitString[i] == allBits[temp]) {
          i++;
          temp++;
        } // ends inner-inner while loop
        if (ptr->bitString[i] == '\0') {
          i = 0;
          if (strcmp(ptr->tokenName,"$Space$") == 0) {
            b = ' ';
            write(newDecompFile,&b,1);
            ptr = NULL;
          } else if (strcmp(ptr->tokenName,"$Tab$") == 0) {
            b = '\t';
            write(newDecompFile,&b,1);
            ptr = NULL;
          } else if (strcmp(ptr->tokenName,"$NewLine$") == 0) {
            b = '\n';
            write(newDecompFile,&b,1);
            ptr = NULL;
          } else {
            while (ptr->tokenName[i] != '\0') {
              b = ptr->tokenName[i];
              write(newDecompFile,&b,1);
              i++;
            } // ends the inner-inner while loop
            ptr = NULL;
          }
        } else {
          ptr = ptr->next;
        }
      } else {
        ptr = ptr->next;
      }
    } // ends the inner while loop
    curr = temp;
  } // ends the while loop

  //free all the nodes of the decompNode LL
  ptr = root;
  while(ptr != NULL) {
    root = root->next;
    free(ptr->bitString);
    free(ptr->tokenName);
    free(ptr);
    ptr = root;
  } // ends the while loop
} // ends the decompressFile() function

/*
  Params:
  - requires a Codebook to be in the current directory
  Returns:
  - LL of nodes with bitString and token names used for decompression
*/
struct decompNode* makeDecompNodes(char* codeBookName) {
  char c , d;
  struct decompNode* root = NULL;
  int CBPointer = open(codeBookName,O_RDWR,0644);                         // first pointer to the codeBook
  if (CBPointer < 0) {printf("ERROR: CANNOT OPEN THE COOKBOOK\n"); return NULL;}
  int CBPointer2 = open(codeBookName,O_RDWR,0644);                        // second pointer to the codeBook, used to count chars
  if (CBPointer2 < 0) {printf("ERROR: CANNOT OPEN THE COOKBOOK\n"); return NULL;}

  // reads the $
  int pointerRead = read(CBPointer,&c,1);
  int pointerRead2 = read(CBPointer2,&d,1);
  //reads the \n
  read(CBPointer,&c,1);
  read(CBPointer2,&d,1);

  int done = 0;
  while (done == 0) {             // need the loop to keep going until the file is done being read through
    read(CBPointer,&c,1);
    read(CBPointer2,&d,1);

    if (d == '\n') {return root;} // marks the last line of the codeBook which means get out of the loop

    int count = 0;
    while(d != '\t') {
      count++;
      read(CBPointer2,&d,1);
    }

    struct decompNode* newNode = (struct decompNode*)malloc(sizeof(struct decompNode));
    struct decompNode* ptr = newNode;

    ptr->bitString = (char*)malloc((count+1)*sizeof(char));
    count = 0;
    while(c != '\t') {                      // put the bitString into the Node
      ptr->bitString[count] = c;
      count++;
      read(CBPointer,&c,1);
    }
    ptr->bitString[count] = '\0';

    read(CBPointer,&c,1);
    read(CBPointer2,&d,1);

    count = 0;
    while(d != '\n') {
      count++;
      read(CBPointer2,&d,1);
    }

    ptr->tokenName = (char*)malloc((count+1)*sizeof(char));
    count = 0;
    while(c != '\n') {                     // put the token name into the Node
      ptr->tokenName[count] = c;
      count++;
      read(CBPointer,&c,1);
    }
    ptr->tokenName[count] = '\0';

    if (root == NULL) {
      ptr->next = root;
      root = ptr;
    } else {
      struct decompNode* cur = root;
      struct decompNode* prev = NULL;
      while (cur != NULL) {
        prev = cur;
        cur = cur->next;
      }
      prev->next = ptr;
      ptr->next = NULL;
    }
  } // ends the while loop
  return root;
} // ends the makeDecompNodes() function

/*
 Params:
 - a file path to a directory
 - the file descriptor
 - the mode (1 = -b , 2 = -c , 3 = -d)
 Returns:
 - uses the other funtions and rcursion to do specific operations on multiple files depending on the mode
 */
void recurFlag(char* dirPath, int fdOut , int mode){
    DIR* pDir;
    struct dirent* pEnt;
    
    pDir = opendir(dirPath);                                // pointer to the current directory (given)
    
    if(pDir != NULL){
        while((pEnt = readdir(pDir)) != NULL) {
            if(strcmp(pEnt -> d_name, "..") == 0 || strcmp(pEnt -> d_name, ".") == 0){
                continue;
            }
            char* entPath;
            if((entPath = malloc(strlen(dirPath) + 1 + strlen(pEnt -> d_name) + 1)) != NULL) {
                entPath[0] = '\0';
                strcat(entPath, dirPath);
                
                if (entPath[strlen(entPath+1)] != '/' ) {
                    strcat(entPath, "/");
                }
                strcat(entPath, pEnt -> d_name);
            } else {
                printf("ERROR: malloc failed.\n");
                return;
            }
            
            if( pEnt -> d_type == DT_DIR) {
                recurFlag(entPath, fdOut , mode);
            } else if ((pEnt -> d_type == DT_REG) && (pEnt->d_name[0] != '.')) {       // is a regular file that can be read and is not a hidden file
                int fd = open(entPath, O_RDONLY);
                if(fd < 0){
                    printf("ERROR: open failed.\n");
                    return;
                }
                if (mode == 1) {
                    char buf;
                    while(read(fd, &buf, 1) != 0) {                     // write the contents into the temp file
                        write(fdOut, &buf, 1);
                    }
                } else if (mode == 2) {                                 // compress the file given
                    cFlag(entPath);
                } else {                                                // decompress the file given
                    int a = (int)strlen(entPath);                       // make sure the file being looked at ends in ".hcz"
                    if ((entPath[a-4] == '.') && (entPath[a-3] == 'h') && (entPath[a-2] == 'c') && (entPath[a-1] == 'z')) {
                        dFlag(entPath);
                    }
                }
                close(fd);
            } // ends the main if - else if
            free(entPath);
        } // ends the while loop
        closedir(pDir);
    }
} // ends the recurFlag() function
