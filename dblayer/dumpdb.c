#include <stdio.h>
#include <stdlib.h>
#include "codec.h"
#include "tbl.h"
#include "util.h"
#include "../pflayer/pf.h"
#include "../amlayer/am.h"
#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}


void
printRow(void *callbackObj, RecId rid, byte *row, int len) {
    Schema *schema = (Schema *) callbackObj;
    byte *cursor = row;

    // UNIMPLEMENTED;

    for(int i=0;i<schema->numColumns;i++){

        if(i > 0){
            printf(",");
        }

        switch(schema->columns[i]->type){
            case VARCHAR: {

                char value[PF_PAGE_SIZE];
                int encodedlen = DecodeCString(cursor,value,sizeof(value));
                printf("%s",value);
                cursor += encodedlen;
                break;
            }
            case INT: 
            printf("%d",DecodeInt(cursor));
            cursor += sizeof(int);
            break;
            
            case LONG:
            printf("%lld", DecodeLong(cursor));
            cursor += sizeof(long long);
            break;

            default:
            break;
        }
    }
    putchar('\n');
}

#define DB_NAME "data.db"
#define INDEX_NAME "data.db.0"
	 
void
index_scan(Table *tbl, Schema *schema, int indexFD, int op, int value) {
    // UNIMPLEMENTED;
    /*
    Open index ...
    while (true) {
	find next entry in index
	fetch rid from table
        printRow(...)

    }
    close index ...
    */

    
   int scanDesc = AM_OpenIndexScan(indexFD, 'i', sizeof(int), op,(char *) &value);

   if(scanDesc < 0){
    checkerr(scanDesc);
   }

   byte row[PF_PAGE_SIZE];
   int rid;

   while(rid = AM_FindNextEntry(scanDesc)  >= 0){
    
    int len = Table_get(tbl,rid,row,sizeof(row));
      if(len < 0){
        checkerr(len);
      }

      printRow(schema,rid,row,len);
   }

   int err = AM_CloseIndexScan(scanDesc);
   if(err < 0){
    checkerr(err);
    exit(ExitFailure);
   }


}

int
main(int argc, char **argv) {
    char *schemaTxt = "Country:varchar,Capital:varchar,Population:int";
    Schema *schema = parseSchema(schemaTxt);
    Table *tbl;

    // UNIMPLEMENTED;
    int err = Table_Open(DB_NAME,schema,false,&tbl);
    checkerr(err);

    if (argc == 2 && *(argv[1]) == 's') {
	// UNIMPLEMENTED;
	// invoke Table_Scan with printRow, which will be invoked for each row in the table.
    Table_Scan(tbl,schema,printRow,schema);
    
    } else {
	// index scan by default
	int indexFD = PF_OpenFile(INDEX_NAME);
	checkerr(indexFD);

	// Ask for populations less than 100000, then more than 100000. Together they should
	// yield the complete database.
	index_scan(tbl, schema, indexFD, LESS_THAN_EQUAL, 100000);
	index_scan(tbl, schema, indexFD, GREATER_THAN, 100000);
    }
    Table_Close(tbl);
}
