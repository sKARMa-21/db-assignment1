
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tbl.h"
#include "codec.h"
#include "../pflayer/pf.h"

#define SLOT_COUNT_OFFSET 2
#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(EXIT_FAILURE);}}

int  getLen(int slot, byte *pageBuf);
int  getNumSlots(byte *pageBuf);
void setNumSlots(byte *pageBuf, int nslots);
int  getNthSlotOffset(int slot, char* pageBuf);

int getNumSlots(byte *pageBuf)
{
    unsigned short nslots;
    memcpy(&nslots, pageBuf + SLOT_COUNT_OFFSET, sizeof(unsigned short));
    return nslots;
}

void setNumSlots(byte *pageBuf, int nslots)
{
    unsigned short value = (unsigned short)nslots;
    memcpy(pageBuf + SLOT_COUNT_OFFSET, &value, sizeof(unsigned short));
}

int getNthSlotOffset(int slot, char *pageBuf)
{
    unsigned short offset;
    memcpy(&offset,
           pageBuf + 4 + slot * sizeof(unsigned short),
           sizeof(unsigned short));

    return offset;
}

int getLen(int slot, byte *pageBuf)
{
    int offset = getNthSlotOffset(slot, (char *)pageBuf);
    int prevBoundary = (slot == 0) ? PF_PAGE_SIZE
                                    : getNthSlotOffset(slot - 1, (char *)pageBuf);
    return prevBoundary - offset;
}


/**
   Opens a paged file, creating one if it doesn't exist, and optionally
   overwriting it.
   Returns 0 on success and a negative error code otherwise.
   If successful, it returns an initialized Table*.
 */
int
Table_Open(char *dbname, Schema *schema, bool overwrite, Table **ptable)
{
    // UNIMPLEMENTED;
    // Initialize PF, create PF file,
    // allocate Table structure  and initialize and return via ptable
    // The Table structure only stores the schema. The current functionality
    // does not really need the schema, because we are only concentrating
    // on record storage. 

    PF_Init();
     
   // if overwirte is true them destroy the file if it existst
    if (overwrite) {
        PF_DestroyFile(dbname);
    }

    int fd = PF_OpenFile(dbname);
    
    if(fd < 0){
        // file does not exist ,create it
        int err = PF_CreateFile(dbname);
        if(err < 0){
            PF_PrintError();
            return err;
        }
        fd = PF_OpenFile(dbname);
        checkerr(fd);
    }
    // Table *tbl = malloc(sizeof(Table));
    

    Table *tbl = malloc(sizeof(Table));

    if(!tbl){
        PF_PrintError();
        return -1;
    }

    tbl->schema = schema;
    tbl->fileDesc = fd;
    tbl->currPN = -1;
    tbl->currPB = NULL;


    * ptable = tbl;
    return 0;
     

}


void
Table_Close(Table *tbl) {
    // UNIMPLEMENTED;
    // Unfix any dirty pages, close file.

    int fd = tbl->fileDesc;

    int err = PF_CloseFile(fd);
    checkerr(err);
    free(tbl);
}


int
Table_Insert(Table *tbl, byte *record, int len, RecId *rid) {
    // Allocate a fresh page if len is not enough for remaining space
    // Get the next free slot on page, and copy record in the free
    // space
    // Update slot and free space index information on top of page.
    // int pageNum;
    // char *pageBuf;
    int err;

    if(tbl->currPB == NULL ){
        // if(tbl->currPB != NULL){
        //     err = PF_UnfixPage(tbl->fileDesc, tbl->currPN, TRUE);
        //     checkerr(err);
        // }
        err = PF_AllocPage(tbl->fileDesc, &tbl->currPN, &tbl->currPB);
        checkerr(err);
        setNumSlots((byte *) tbl->currPB, 0);
    } else{
        int numSlots = getNumSlots((byte *)tbl -> currPB);
        int boundary;
        if (numSlots == 0){
            boundary = PF_PAGE_SIZE;
        } else{
            getNthSlotOffset(numSlots-1, tbl->currPB);
        }
        int headerEnd = 4 + (numSlots +1) * (int) sizeof(unsigned short);
        
        if(boundary - headerEnd < len){
            err = PF_UnfixPage(tbl->fileDesc, tbl->currPN, TRUE);
            checkerr(err);
            err  = PF_AllocPage(tbl->fileDesc, &tbl->currPN, &tbl->currPB);
            checkerr(err);
            setNumSlots((byte *) tbl->currPB, 0);
        }
    }

    int slot = getNumSlots((byte *)tbl->currPB);
    int boundary;
    if (slot == 0){
            boundary = PF_PAGE_SIZE;
        } else{
            getNthSlotOffset(slot-1, tbl->currPB);
        }
    int offset = boundary -len;


    // err = PF_AllocPage(tbl-> fileDesc, &pageNum, &pageBuf);
    // checkerr(err);

    // int slot = getNumSlots((byte *) pageBuf);
    // int offset = getNthSlotOffset(slot, pageBuf);
    memcpy(tbl->currPB + offset, record, len);

    memcpy(tbl->currPB + 4+slot * (int) sizeof(unsigned short), &offset, sizeof(unsigned short));

    setNumSlots((byte *) tbl->currPB, slot+1);


    *rid = (tbl->currPN << 8) | slot & 0xFF;
    return 0;

}

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(EXIT_FAILURE);}}

/*
  Given an rid, fill in the record (but at most maxlen bytes).
  Returns the number of bytes copied.
 */
int
Table_Get(Table *tbl, RecId rid, byte *record, int maxlen) {
    int slot = rid & 0xFF;
    int pageNum = rid >> 8;

    // UNIMPLEMENTED;
    // PF_GetThisPage(pageNum)
    // In the page get the slot offset of the record, and
    // memcpy bytes into the record supplied.
    // Unfix the page

    char *pageBuf;
    int err;

    err = PF_GetThisPage(tbl->fileDesc, pageNum, &pageBuf);
    checkerr(err);

    int offset = getNthSlotOffset(slot, pageBuf);
    int len = getLen(slot,(byte *) pageBuf);

    if (len > maxlen){
        len = maxlen;
    }
     
    memcpy(record, pageBuf + offset, len);

    err = PF_UnfixPage(tbl->fileDesc, pageNum, FALSE);
    checkerr(err);

    return len; // return size of record
}

void
Table_Scan(Table *tbl, void *callbackObj, ReadFunc callbackfn) {

    // UNIMPLEMENTED;

    // For each page obtained using PF_GetFirstPage and PF_GetNextPage
    //    for each record in that page,
    //          callbackfn(callbackObj, rid, record, recordLen)

    int err;
    int pagenum;
    char *pageBuf;

    err = PF_GetFirstPage(tbl->fileDesc, &pagenum, &pageBuf);

    while (err == PFE_OK){
        int nslots = getNumSlots((byte*)pageBuf);

        for (int slot = 0; slot < nslots; slot++){

            int offset = getNthSlotOffset(slot, pageBuf);
            int len = getLen(slot, (byte * )pageBuf);
            RecId rid = (pagenum << 8) | slot;

            callbackfn(callbackObj, rid, (byte *)(pageBuf + offset), len);

        }
        PF_UnfixPage(tbl->fileDesc, pagenum, FALSE);
        err = PF_GetNextPage(tbl->fileDesc, &pagenum, &pageBuf);
    }
    if(err != PFE_EOF){
        checkerr(err);
    }

}


