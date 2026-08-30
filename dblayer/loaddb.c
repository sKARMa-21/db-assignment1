#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "codec.h"
#include "../pflayer/pf.h"
#include "../amlayer/am.h"
#include "tbl.h"
#include "util.h"

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}

#define MAX_PAGE_SIZE 4000


#define DB_NAME "data.db"
#define INDEX_NAME "data.db.0"
#define CSV_NAME "data.csv"


/*
Takes a schema, and an array of strings (fields), and uses the functionality
in codec.c to convert strings into compact binary representations
 */
int
encode(Schema *sch, char **fields, byte *record, int spaceLeft) {
    // UNIMPLEMENTED;
    // for each field
    //    switch corresponding schema type is
    //        VARCHAR : EncodeCString
    //        INT : EncodeInt
    //        LONG: EncodeLong
    // return the total number of bytes encoded into record
    

    int offset = 0;

    for(int i=0;i<sch->numColumns;i++){
        int encoded = 0;

        switch (sch->columns[i]->type)
        {
        case VARCHAR:
            encoded = EncodeCString(fields[i],record+offset, spaceLeft - offset);
            break;
        case INT:
            encoded = EncodeInt(atoi(fields[i]),record+offset);
            break;
        case LONG: 
             encoded = EncodeLong(atol(fields[i]),record+offset);    
             break;
        default:
            break;
        }
        offset += encoded;
    }

    return offset;



}

Schema *
loadCSV() {
    // Open csv file, parse schema
    FILE *fp = fopen(CSV_NAME, "r");

    if (!fp) {
	    perror("data.csv could not be opened");
        exit(EXIT_FAILURE);
    }

    char buf[MAX_LINE_LEN];
    char *line = fgets(buf, MAX_LINE_LEN, fp);
    if (line == NULL) {
        fprintf(stderr, "Unable to read data.csv\n");
        exit(EXIT_FAILURE);
    }

    // Open main db file
    Schema *sch = parseSchema(line);
    Table *tbl;
    
    // UNIMPLEMENTED;
    int err = Table_Open(DB_NAME, sch, true, &tbl);
    checkerr(err);

    // int filedec = PF_OpenFile(DB_NAME);

    // int err2 = AM_CreateIndex(DB_NAME, 0, 'i', sizeof(int));
    // checkerr(err2);
    int err2 = AM_CreateIndex(DB_NAME, 0, 'i', sizeof(int));
printf("AM_CreateIndex returned: %d\n", err2);
checkerr(err2);
    int indexFD = PF_OpenFile(INDEX_NAME);
    checkerr(indexFD);
    // if (indexFD < 0) {
    //     AM_PrintError("AM_Create Index Failed");
    //     Table_Close(tbl);
    //     fclose(fp);
    //     exit(EXIT_FAILURE);
    // }


    char *tokens[MAX_TOKENS];
    char record[MAX_PAGE_SIZE];

    while ((line = fgets(buf, MAX_LINE_LEN, fp)) != NULL) {

        int n = split(line, ",", tokens);
        assert (n == sch->numColumns);

        int len = encode(sch, tokens, record, sizeof(record));
        RecId rid;

        // UNIMPLEMENTED;

        err = Table_Insert(tbl, (byte *)record, len, &rid);
        checkerr(err);

        printf("%d %s\n", rid, tokens[0]);

        // Indexing 
        int population = atoi(tokens[2]);

        // UNIMPLEMENTED;
        // Use the population field as the field to index on

        // int indexFD = AM_CreateIndex(DB_NAME, 0, 'i', sizeof(int));

        err = AM_InsertEntry(indexFD, 'i', sizeof(int), (char *)&population, rid);

            
        checkerr(err);
    }
    fclose(fp);
    Table_Close(tbl);
    // err = AM_CloseIndex(indexFD);
    // checkerr(err);
    
    return sch;
}

int
main() {
    loadCSV();
}
