#include "mfs.h"


mfs_DIR* inodes;

size_t NumberOfElementsInInodesArray = sizeof(inodes)/sizeof(inodes[0]); // calculate the number of elemnts in inodes array

void mfs_init() {
  printf("----------------------------mfs_init----------------------------\n");

  uint64_t totalBytes = getVCB()->totalInodeBlocks * getVCB()->blockSize;
  printf("totalInodeBlocks %ld, blockSize %ld\n", getVCB()->totalInodeBlocks, getVCB()->blockSize);
  printf("Allocating %ld bytes for inodes.\n", totalBytes);

  inodes = calloc(getVCB()->totalInodeBlocks, getVCB()->blockSize);
  printf("Inodes allocated at %p.\n", inodes);

  uint64_t blocksRead = LBAread(inodes, getVCB()->totalInodeBlocks, getVCB()->inodeStartBlock);
  printf("Loaded %ld blocks of inodes into cache.\n", blocksRead);
  
  if(blocksRead != getVCB()->totalInodeBlocks) {
    printf("Error: Not all inodes loaded into cache.\n");
    mfs_close();
    exit(0);
  }
  printf("----------------------------------------------------------------\n");
}

void writeInodes() {
  printf("----------------------------writeInodes---------------------------\n");
  LBAwrite(inodes, getVCB()->totalInodeBlocks, getVCB()->inodeStartBlock);
}

//8-1-20 Taylor: Initial implementation of parseFilePath.

// All path data structures should be linkedLists

char currentDirectoryPath[MAX_FILENAME_SIZE];
char currentDirectoryPathArray[MAX_DIRECTORY_DEPTH][MAX_FILENAME_SIZE] = { "/" };
int currentDirectoryPathArraySize = 1;

char requestedFilePathArray[MAX_DIRECTORY_DEPTH][MAX_FILENAME_SIZE];
int requestedFilePathArraySize;

void parseFilePath(const char *pathname) { 
  printf("------------------------Parsing File Path-----------------------\n");
  
  printf("Input: %s\n", pathname);

  /* Clear previous count. */
  requestedFilePathArraySize = 0;

  /* Make mutable copy of pathname. */
  char _pathname[MAX_FILENAME_SIZE];
  strcpy(_pathname, pathname);

  /* Setup tokenizer and check for '.' or '..' at beginning. */
  char* savePointer;
  char* token = strtok_r(_pathname, "/", &savePointer);
  int isRelative = !strcmp(token, "..") || !strcmp(token, ".");
  if(token) {
    if(isRelative) {
      for(int i=0; i<currentDirectoryPathArraySize-1; i++) {
        strcpy(requestedFilePathArray[i], currentDirectoryPathArray[i]);
        requestedFilePathArraySize++;
      }
    }
  }

  /* Discard '.' or '..'. */
  if(isRelative) {
    token = strtok_r(0, "/", &savePointer);
  }

  while(token && requestedFilePathArraySize < MAX_DIRECTORY_DEPTH) {

    strcpy(requestedFilePathArray[requestedFilePathArraySize], token);
    
    /* Printf for debug. */
    printf("\t%s\n", token);
    
    requestedFilePathArraySize++;
    token = strtok_r(0, "/", &savePointer);

  }

printf("----------------------------------------------------------------\n");

}

//8-1-20 Taylor: Added to test parseFileName
void printFilePath() {

  for(int i=0; i<requestedFilePathArraySize; i++) {

    if(i<requestedFilePathArraySize-1) {
      printf("Directory %d: %s\n", i, requestedFilePathArray[i]);
    } else {
      printf("Filename: %s\n", requestedFilePathArray[i]);
    }

  }
}

mfs_DIR* getInode(const char *pathname){
  printf("----------------------------getInode----------------------------\n");
  // 1- Loop over inodes
  // 2- find requested node
  // 3- return that node
  // if does not exist return NULL

  printf("pathname: %s\n", pathname);

  for (size_t i = 0; i < getVCB()->totalInodes; i++) {
    if (strcmp(inodes[i].path, pathname) == 0) {
      printf("Found inode.\n");
      return &inodes[i];
    }
  }

  printf("----------------------------------------------------------------\n");

  return NULL;

}

mfs_DIR* getFreeInode(){
  printf("--------------------------getFreeInode--------------------------\n");
  // Search through inodes
  // Return the first avalable inode
  // Update inUSe int of the returned node to 1 (Which means it's in use right now)
  // if there is no free inode return NULL
  mfs_DIR* returnediNode;

  for (size_t i = 0; i < getVCB()->totalInodes; i++) {
    if (inodes[i].inUse == 0) { // if the inode inUse equales 0 that means it is free so we return it
      inodes[i].inUse = 1; // upsdate the node to be in use before returning it
       returnediNode = &inodes[i];
      
      printf("----------------------------------------------------------------\n");
      
      return returnediNode;
      
    }
  }

  printf("----------------------------------------------------------------\n");

  return NULL;

}



/***************** 8-3-2020 ***********************************/

mfs_DIR* createInode(InodeType type,const char* path){
  printf("--------------------------createInode---------------------------\n");
  // returns an inode if succed and NULL if fales
  mfs_DIR* returnediNode;
  char parentPath[MAX_FILENAME_SIZE];
  mfs_DIR* parentNode;

  time_t currentTime;
  /* Obtain current time. */
  currentTime = time(NULL);

  // call checkValidityOfPath() if fales return NULL
  if (checkValidityOfPath() == 0){
    printf("----------------------------------------------------------------\n");
    return NULL;
  };
  // call getFreeInode(), this function will initialize inUse to 1
  returnediNode = getFreeInode();
  getParentPath(parentPath, path); // getParentPath()
  parentNode = getInode(parentPath);
 
  // initialize:
                // call setParent() 
                // 1- InodeType
                // 2- name
                // 3- lastAccessTime
                // 4- lastModificationTime
    
    if (setParent(parentNode, returnediNode) == 1){
                returnediNode->type = type;
                strcpy(returnediNode->name , requestedFilePathArray[requestedFilePathArraySize - 1]);
                returnediNode->lastAccessTime = currentTime;
                returnediNode->lastModificationTime = currentTime;
    } else {
      printf("Error setting parent.\n");
      printf("----------------------------------------------------------------\n");
      return NULL;
    }
    
  printf("Sucessfully created inode for path '%s'.\n", path);       
  printf("----------------------------------------------------------------\n");
  return returnediNode;

}

int setParent(mfs_DIR* parent, mfs_DIR* child){
  printf("----------------------------setParent---------------------------\n");
  // return 0 for fales and 1 for true
      
      // get parent indoe
      // check numChildren -> if 64 retrun 0
      // update parent indoe
        // update:
            // 1- children array
            // 2- numChildren
            // 3- lastAccessTime
            // 4- lastModificationTime
            // 5- sizeInBlocks
            // 6- sizeInBytes
      // update child inode
            // 1- parent
            // 2- path
  
  if(parent->numChildren == 64) {
    printf("Folder '%s' has maximum children.\n", parent->path);
    printf("----------------------------------------------------------------\n");
    return 0;
  }

  strcpy(parent->children[parent->numChildren], child->name);
  parent->numChildren++;
  parent->lastAccessTime = time(0);
  parent->lastModificationTime = time(0);
  parent->sizeInBlocks += child->sizeInBlocks;
  parent->sizeInBytes += child->sizeInBytes;

  strcpy(child->parent, parent->path);
  sprintf(child->path, "%s/%s", parent->path, child->name);

  printf("Set parent of '%s' to '%s'.\n", child->path, child->parent);
  
  printf("----------------------------------------------------------------\n");
  return 1;
}

char* getParentPath(char* buf ,const char* path){
  printf("-------------------------getParentPath--------------------------\n");
  // return NULL for fales and a "path" if succeed
    // parse the requestedFilePathArray into a string the return parent string "path"
    //Copy parent path to buf, return buf
  
  /* Parse the path. */
  parseFilePath(path);

  char parentPath[MAX_FILENAME_SIZE] = "";

  /* Loop until the second to last element. */
  for(int i=0; i<requestedFilePathArraySize - 1; i++) {

    /* Append a separator and the next level of the path. */
    strcat(parentPath, "/");
    strcat(parentPath, requestedFilePathArray[i]);
  }

  strcpy(buf, parentPath);

  printf("Input: %s, Parent Path: %s\n", path, buf);
  
  printf("----------------------------------------------------------------\n");
  
  return buf;
}

int checkValidityOfPath(){
  printf("----------------------checkValidityOfPath-----------------------\n");
   // return 0 for fales and 1 if succeed
  // loop over requestedFilePathArray
  // assemble each partial path by adding next level
  // search for partial path with getInode
  // fail on first not found
  printf("----------------------------------------------------------------\n");
}

int writeBufferToInode(mfs_DIR* inode, char* buffer, size_t bufSizeBytes, uint64_t blockNumber) {
  printf("-----------------------writeBufferToInode-----------------------\n");


  /* Check if dataBlockPointers is full. */
  int freeIndex = -1;
  for(int i=0; i<MAX_DATABLOCK_POINTERS; i++) {
    if(inode->directBlockPointers[i] == INVALID_DATABLOCK_POINTER) {
      
      /* Record free dataBlockPointer index. */
      freeIndex = i;
      break;
    }
  }

  /* If there is no place to put the new dataBlock pointer. Return 0 blocks/bytes written. */
  if(freeIndex == -1) {
    return 0;
  }

  /* Write buffered data to disk, update inode, write inodes to disk. */
  LBAwrite(buffer, 1, blockNumber);

  inode->directBlockPointers[freeIndex] = blockNumber;
  inode->numDirectBlockPointers++;
  inode->sizeInBlocks++;
  inode->sizeInBytes += bufSizeBytes;
  inode->lastAccessTime = time(0);
  inode->lastModificationTime = time(0);

  writeInodes();

  printf("----------------------------------------------------------------\n");

  return 1;

}

/***************************************** End 8-3-2020 **************************************************/

/***************************************** Start 8-4-2020 **************************************************/


void freeInode(mfs_DIR* node){
  printf("----------------------------freeInode---------------------------\n");
  node->inUse = 0;
  node->name[0] = NULL;
  node->path[0] = NULL;
  node->parent[0] = NULL;
  node->sizeInBlocks = 0;
  node->sizeInBytes = 0;

  /* free the data blockes asociated with the file, if we are deleting a file */
  if(node->type == I_FILE){
    for (size_t i = 0; i < node->numDirectBlockPointers; i++) {
      int blockPointer = node->directBlockPointers[i];
      clearBit(getVCB()->freeMap, blockPointer);
    }
  }
  printf("----------------------------------------------------------------\n");
}

/***************************************** END 8-4-2020 **************************************************/

void mfs_close() {
  printf("----------------------------mfs_close---------------------------\n");

  // free any linedLists

  free(inodes);
  printf("----------------------------------------------------------------\n");
}
 

int mfs_mkdir(const char *pathname, mode_t mode) { // return 0 for sucsess and -1 if not
  printf("----------------------------mfs_mkdir---------------------------\n");
   // Parse file name 
  // Add info an inode mfs_DIR
  // Add info to parent if necessary
  // check if the fiolder already exists
  char parentPath[256] = "";
  parseFilePath(pathname);

  for (size_t i = 0; i < requestedFilePathArraySize - 1; i++) {
     strcat(parentPath, "/");
     strcat(parentPath, requestedFilePathArray[i]);
  }
  
  mfs_DIR* parent = getInode(parentPath);
  if (parent) {
    for (size_t i = 0; i < parent->numChildren; i++){
      if(strcmp(parent->children[i], requestedFilePathArray[requestedFilePathArraySize - 1])){
          printf("Folder already exists!");
          printf("----------------------------------------------------------------\n");
          return -1;
      }
    }
  } else {
    printf("Parent '%s' does not exist!\n", parentPath);
    printf("----------------------------------------------------------------\n");
    return -1;
  }
  

  if( createInode(I_DIR, pathname)){
    writeInodes();
    printf("----------------------------------------------------------------\n");
    return 0;
  }
  printf("Failed to make directory '%s'.\n", pathname);
  printf("----------------------------------------------------------------\n");
  return -1;
}

int mfs_rmdir(const char *pathname) { // return 0 for sucsess and -1 if not
  printf("----------------------------mfs_rmdir---------------------------\n");
  mfs_DIR* node = getInode(pathname);
  if (node->type == I_DIR && node->numChildren == 0){
    freeInode(node);
    printf("----------------------------------------------------------------\n");
    return 0;
  }
  printf("----------------------------------------------------------------\n");
  return -1;
}

mfs_DIR* mfs_opendir(const char *fileName) {
  printf("--------------------------mfs_opendir-------------------------\n");
  int ret = b_open(fileName, 0);
  if(ret < 0) {
    printf("----------------------------------------------------------------\n");
    return NULL;
  }
  printf("----------------------------------------------------------------\n");
  return getInode(fileName);
}

struct mfs_dirent* mfs_readdir(mfs_DIR *dirp) {
  printf("-------------------------mfs_readdir----------------------------\n");
  printf("----------------------------------------------------------------\n");
  return 0;
}

int mfs_closedir(mfs_DIR *dirp) {
  printf("---------------------------mfs_closedir--------------------------\n");
  printf("----------------------------------------------------------------\n");
  // 
  return 0;
}



//8-1-20 Taylor: Initial implementation.
//8-2-20 Taylor: Modified to copy currentDirectoryPath to buf or set errno per description of getcwd in unistd.h
char * mfs_getcwd(char *buf, size_t size) {
  printf("----------------------------mfs_getcwd---------------------------\n");
  if(strlen(currentDirectoryPath) > size) {
    errno = ERANGE;
    return NULL;
  }
  strcpy(buf, currentDirectoryPath);
  printf("----------------------------------------------------------------\n");
  return buf;
}

//8-3-20 Taylor: Function now check for validity of the path. Returns 1 if inode does not exist,
//      0 otherwise.

//8-1-20 Taylor: Initial implementation.
//Note: This does not currently check validity of 
//      the path.
//Note: This may need to first check whether the provided path is within limit of MAX_FILENAME_SIZE
//      and set errno similar to mfs_getcwd.

int mfs_setcwd(char *buf) {
  printf("---------------------------mfs_setcwd---------------------------\n");
  
  /* Check if inode exists. */
  mfs_DIR* inode = getInode(buf);
  if(!inode) {
    printf("----------------------------------------------------------------\n");
    return 1;
  }
  
  /* Clear previous cwd and parse new path. */
  currentDirectoryPath[0] = '\0';
  currentDirectoryPathArraySize = 0;
  parseFilePath(buf);

  /* Copy parsed pathname to currentDirectoryPathArray. */
  for(int i=0; i<requestedFilePathArraySize; i++) {
    strcpy(currentDirectoryPathArray[i], requestedFilePathArray[i]);
    sprintf(currentDirectoryPath, "%s/%s", currentDirectoryPath, requestedFilePathArray[i]);
    currentDirectoryPathArraySize++;
  }
  printf("----------------------------------------------------------------\n");
  return 0;

}

//8-1-20 Taylor: Added to test mfs_setcwd
void printCurrentDirectoryPath() {

  for(int i=0; i<currentDirectoryPathArraySize; i++) {

    if(i<currentDirectoryPathArraySize-1) {
      printf("Directory %d: %s\n", i, currentDirectoryPathArray[i]);
    } else {
      printf("Filename: %s\n", currentDirectoryPathArray[i]);
    }

  }
}

//8-1-20 Taylor: Initial implementation of mfs_isFile and mfs_isDir
int mfs_isFile(char * path) {
  printf("------------------------------isFile----------------------------\n");
  mfs_DIR* inode = getInode(path);
  printf("----------------------------------------------------------------\n");
  return inode ? inode->type == I_FILE : 0;
}

int mfs_isDir(char * path) {
  printf("------------------------------isDir-----------------------------\n");
  mfs_DIR* inode = getInode(path);
  printf("----------------------------------------------------------------\n");
  return inode ? inode->type == I_DIR : 0;
}

int mfs_delete(char* filename) {
  printf("---------------------------mfs_delete---------------------------\n");
  //Get inode
  //Get parent
  //Remove child from parent
  //Clear properties on child inode so it is not found in search.
  //Set inuse to false
  printf("----------------------------------------------------------------\n");
  return 0;
}

//8-2-20 Taylor: Initial implementation of mfs_stat.
//    mfs_DIR needs rework to contain these fields.
int mfs_stat(const char *path, struct mfs_stat *buf) {
  printf("----------------------------mfs_stat----------------------------\n");
  mfs_DIR* inode = getInode(path);
  if(inode) {
    buf->st_size = 999;
    buf->st_blksize = getVCB()->blockSize;
    buf->st_blocks = 2;
    buf->st_accesstime = 1;
    buf->st_modtime = 1;
    buf->st_createtime = 1;
    printf("----------------------------------------------------------------\n");
    return 1;
  }
  printf("----------------------------------------------------------------\n");
  return 0;
}