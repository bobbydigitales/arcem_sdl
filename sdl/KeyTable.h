/* KeyTable.h */

/* Only required because the Makefile insists $(SYSTEM)/KeyTable.h
 * exists. */

struct ArcKeyTrans {
  int sym;
  int row,col;
};

// Used in the inverted key table
struct keyloc {
    int row, col;
};